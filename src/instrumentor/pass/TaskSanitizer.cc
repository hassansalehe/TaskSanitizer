//////////////////////////////////////////////////////////////
//
// TaskSanitizer.cc
//
// Copyright (c) 2017 - 2018 Hassan Salehe Matar
// All rights reserved.
//
// This file is part of TaskSanitizer. For details, see
// https://github.com/hassansalehe/TaskSanitizer. Please also
// see the LICENSE file for additional BSD notice
//
// Redistribution and use in source and binary forms, with
// or without modification, are permitted provided that
// the following conditions are met:
//
// * Redistributions of source code must retain the above
//   copyright notice, this list of conditions and the
//   following disclaimer.
//
// * Redistributions in binary form must reproduce the
//   above copyright notice, this list of conditions and
//   the following disclaimer in the documentation and/or
//   other materials provided with the distribution.
//
// * Neither the name of the copyright holder nor the names
//   of its contributors may be used to endorse or promote
//   products derived from this software without specific
//   prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
// CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
// INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
// BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
// WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
// SUCH DAMAGE.
//
//////////////////////////////////////////////////////////////

#include "instrumentor/pass/LLVMLibs.h"
#include "instrumentor/pass/Util.h"
#include "instrumentor/pass/IIRlogger.h"
#include "instrumentor/pass/DebugInfoHelper.h"

#define DEBUG_TYPE "tasksan"

static llvm::cl::opt<bool>  ClInstrumentMemoryAccesses(
    "tasksan-instrument-memory-accesses", llvm::cl::init(true),
    llvm::cl::desc("Instrument memory accesses"), llvm::cl::Hidden);
static llvm::cl::opt<bool>  ClInstrumentFuncEntryExit(
    "tasksan-instrument-func-entry-exit", llvm::cl::init(true),
    llvm::cl::desc("Instrument function entry and exit"), llvm::cl::Hidden);
static llvm::cl::opt<bool>  ClHandleCxxExceptions(
    "tasksan-handle-cxx-exceptions", llvm::cl::init(true),
    llvm::cl::desc("Handle C++ exceptions (insert cleanup blocks for unwinding)"),
    llvm::cl::Hidden);
static llvm::cl::opt<bool>  ClInstrumentAtomics(
    "tasksan-instrument-atomics", llvm::cl::init(true),
    llvm::cl::desc("Instrument atomics"), llvm::cl::Hidden);
static llvm::cl::opt<bool>  ClInstrumentMemIntrinsics(
    "tasksan-instrument-memintrinsics", llvm::cl::init(true),
    llvm::cl::desc("Instrument memintrinsics (memset/memcpy/memmove)"), llvm::cl::Hidden);

STATISTIC(NumInstrumentedReads, "Number of instrumented reads");
STATISTIC(NumInstrumentedWrites, "Number of instrumented writes");
STATISTIC(NumOmittedReadsBeforeWrite,
          "Number of reads ignored due to following writes");
STATISTIC(NumAccessesWithBadSize, "Number of accesses with bad size");
STATISTIC(NumInstrumentedVtableWrites, "Number of vtable ptr writes");
STATISTIC(NumInstrumentedVtableReads, "Number of vtable ptr reads");

STATISTIC(NumOmittedReadsFromConstantGlobals,
          "Number of reads from constant globals");
STATISTIC(NumOmittedReadsFromVtable, "Number of vtable reads");
STATISTIC(NumOmittedNonCaptured, "Number of accesses ignored due to capturing");

static const char *const kTsanModuleCtorName = "tsan.module_ctor";
static const char *const kTsanInitName = "__tsan_init";

namespace {

// Function pass to instrument shared memory accesses in OpenMP programs.
struct TaskSanitizer : public llvm::FunctionPass {

  static char ID;
  TaskSanitizer() : FunctionPass(ID) {}

  llvm::StringRef getPassName() const override {
    return "TaskSanitizer";
  }

  void getAnalysisUsage(llvm::AnalysisUsage &AU) const override {
    AU.addRequired<llvm::TargetLibraryInfoWrapperPass>();
  }

  bool doInitialization(llvm::Module &M) override {

    // initialization of task IIR logger.
    tasan::IIRlog::InitializeLogger( M.getName() );

    if ( tasan::debug::hasMainFunction(M) ) {
      IIRfileURL = std::string(
          tasan::debug::getFullFilename(M) + ".iir");
    }

    const llvm::DataLayout &DL = M.getDataLayout();
    IntptrTy = DL.getIntPtrType(M.getContext());
// HASSAN:
//    std::tie(TsanCtorFunction, std::ignore)
//        = createSanitizerCtorAndInitFunctions(
//      M, kTsanModuleCtorName, kTsanInitName, /*InitArgTypes=*/{},
//      /*InitArgs=*/{});
//
//    appendToGlobalCtors(M, TsanCtorFunction, 0);

    return true;
  }

  bool runOnFunction(llvm::Function &F) override;

 private:
  void initializeCallbacks(llvm::Module &M);
  bool instrumentLoadOrStore(llvm::Instruction *I, const llvm::DataLayout &DL);
  bool instrumentAtomic(llvm::Instruction *I, const llvm::DataLayout &DL);
  bool instrumentMemIntrinsic(llvm::Instruction *I);
  void chooseInstructionsToInstrument(llvm::SmallVectorImpl<llvm::Instruction *> &Local,
                                      llvm::SmallVectorImpl<llvm::Instruction *> &All,
                                      const llvm::DataLayout &DL);
  bool addrPointsToConstantData(llvm::Value *Addr);
  int getMemoryAccessFuncIndex(llvm::Value *Addr, const llvm::DataLayout &DL);
  void InsertRuntimeIgnores(llvm::Function &F);

  llvm::Type *IntptrTy;
  llvm::IntegerType *OrdTy;

  // register every new instrumented function
  llvm::Value *funcNamePtr = NULL;

  // Data for registering IIR file name
  llvm::Value *IIRfile;
  std::string IIRfileURL;

  // Callbacks to run-time library are computed in doInitialization.
  llvm::Function *RegisterIIRfile;
  llvm::Function *TsanFuncEntry;
  llvm::Function *TsanFuncExit;
  llvm::Function *TsanIgnoreBegin;
  llvm::Function *TsanIgnoreEnd;

  llvm::Function *FlowSan_TaskBeginFunc;

  // callbacks for instrumenting doubles and floats
  llvm::Constant *FlowSan_MemWriteFloat;
  llvm::Constant *FlowSan_MemWriteDouble;

  // Accesses sizes are powers of two: 1, 2, 4, 8, 16.
  static const size_t kNumberOfAccessSizes = 5;
  llvm::Function *TsanRead[kNumberOfAccessSizes];
  llvm::Function *TsanWrite[kNumberOfAccessSizes];
  llvm::Function *TsanUnalignedRead[kNumberOfAccessSizes];
  llvm::Function *TsanUnalignedWrite[kNumberOfAccessSizes];
  llvm::Function *TsanAtomicLoad[kNumberOfAccessSizes];
  llvm::Function *TsanAtomicStore[kNumberOfAccessSizes];
  llvm::Function *TsanAtomicRMW[llvm::AtomicRMWInst::LAST_BINOP + 1][kNumberOfAccessSizes];
  llvm::Function *TsanAtomicCAS[kNumberOfAccessSizes];
  llvm::Function *TsanAtomicThreadFence;
  llvm::Function *TsanAtomicSignalFence;
  llvm::Function *TsanVptrUpdate;
  llvm::Function *TsanVptrLoad;
  llvm::Function *MemmoveFn, *MemcpyFn, *MemsetFn;
  llvm::Function *TsanCtorFunction;

}; // end of TaskSanitizer
} // end of namespace

char TaskSanitizer::ID = 1;

static void registerTaskSanitizer(
  const llvm::PassManagerBuilder &,
  llvm::legacy::PassManagerBase &PM) { PM.add(new TaskSanitizer()); }

static llvm::RegisterStandardPasses regPass(
   llvm::PassManagerBuilder::EP_EarlyAsPossible,
   registerTaskSanitizer);

void TaskSanitizer::initializeCallbacks(llvm::Module &M) {
  llvm::IRBuilder<> IRB(M.getContext());
  llvm::AttributeList Attr;
  Attr = Attr.addAttribute(M.getContext(),
      llvm::AttributeList::FunctionIndex, llvm::Attribute::NoUnwind);
  // Initialize the callbacks.
  RegisterIIRfile = checkSanitizerInterfaceFunction(M.getOrInsertFunction(
      "__tsan_register_iir_file", Attr, IRB.getVoidTy(), IRB.getInt8PtrTy()));

  TsanFuncEntry = checkSanitizerInterfaceFunction(M.getOrInsertFunction(
      "__tsan_func_entry", Attr, IRB.getVoidTy(), IRB.getInt8PtrTy()));
  TsanFuncExit = checkSanitizerInterfaceFunction(M.getOrInsertFunction(
      "__tsan_func_exit", Attr, IRB.getVoidTy(), IRB.getInt8PtrTy()));

  TsanIgnoreBegin = checkSanitizerInterfaceFunction(M.getOrInsertFunction(
      "__tsan_ignore_thread_begin", Attr, IRB.getVoidTy()));

  TsanIgnoreEnd = checkSanitizerInterfaceFunction(M.getOrInsertFunction(
      "__tsan_ignore_thread_end", Attr, IRB.getVoidTy()));

  FlowSan_TaskBeginFunc = checkSanitizerInterfaceFunction(M.getOrInsertFunction(
      "FlowSan_TaskBeginFunc", IRB.getVoidTy(), IRB.getInt8PtrTy()));

  // functions to instrument floats and doubles
  llvm::LLVMContext &Ctx = M.getContext();
  FlowSan_MemWriteFloat = M.getOrInsertFunction("__fsan_write_float",
      llvm::Type::getVoidTy(Ctx), llvm::Type::getFloatPtrTy(Ctx),
      llvm::Type::getFloatTy(Ctx), llvm::Type::getInt8Ty(Ctx));

  FlowSan_MemWriteDouble = M.getOrInsertFunction("__fsan_write_double",
      llvm::Type::getVoidTy(Ctx), llvm::Type::getDoublePtrTy(Ctx),
      llvm::Type::getDoubleTy(Ctx), llvm::Type::getInt8Ty(Ctx));

  OrdTy = IRB.getInt32Ty();
  for (size_t i = 0; i < kNumberOfAccessSizes; ++i) {
    const unsigned ByteSize = 1U << i;
    const unsigned BitSize = ByteSize * 8;
    std::string ByteSizeStr = llvm::utostr(ByteSize);
    std::string BitSizeStr = llvm::utostr(BitSize);
    llvm::SmallString<32> ReadName("__tsan_read" + ByteSizeStr);
    TsanRead[i] = checkSanitizerInterfaceFunction(M.getOrInsertFunction(
        ReadName, Attr, IRB.getVoidTy(), IRB.getInt8PtrTy(),
        IRB.getInt64Ty()));

    llvm::SmallString<32> WriteName("__tsan_write" + ByteSizeStr);
    TsanWrite[i] = checkSanitizerInterfaceFunction(M.getOrInsertFunction(
        WriteName, Attr, IRB.getVoidTy(), IRB.getInt8PtrTy(),
        IRB.getInt64Ty(), IRB.getInt8Ty()));

    llvm::SmallString<64> UnalignedReadName("__tsan_unaligned_read" + ByteSizeStr);
    TsanUnalignedRead[i] =
        checkSanitizerInterfaceFunction(M.getOrInsertFunction(
            UnalignedReadName, Attr, IRB.getVoidTy(), IRB.getInt8PtrTy()));

    llvm::SmallString<64> UnalignedWriteName("__tsan_unaligned_write" + ByteSizeStr);
    TsanUnalignedWrite[i] =
        checkSanitizerInterfaceFunction(M.getOrInsertFunction(
            UnalignedWriteName, Attr, IRB.getVoidTy(), IRB.getInt8PtrTy()));

    llvm::Type *Ty = llvm::Type::getIntNTy(M.getContext(), BitSize);
    llvm::Type *PtrTy = Ty->getPointerTo();
    llvm::SmallString<32> AtomicLoadName("__tsan_atomic" + BitSizeStr + "_load");
    TsanAtomicLoad[i] = checkSanitizerInterfaceFunction(
        M.getOrInsertFunction(AtomicLoadName, Attr, Ty, PtrTy, OrdTy));

    llvm::SmallString<32> AtomicStoreName("__tsan_atomic" + BitSizeStr + "_store");
    TsanAtomicStore[i] = checkSanitizerInterfaceFunction(M.getOrInsertFunction(
        AtomicStoreName, Attr, IRB.getVoidTy(), PtrTy, Ty, OrdTy));

    for (int op = llvm::AtomicRMWInst::FIRST_BINOP;
        op <= llvm::AtomicRMWInst::LAST_BINOP; ++op) {
      TsanAtomicRMW[op][i] = nullptr;
      const char *NamePart = nullptr;
      if (op == llvm::AtomicRMWInst::Xchg)
        NamePart = "_exchange";
      else if (op == llvm::AtomicRMWInst::Add)
        NamePart = "_fetch_add";
      else if (op == llvm::AtomicRMWInst::Sub)
        NamePart = "_fetch_sub";
      else if (op == llvm::AtomicRMWInst::And)
        NamePart = "_fetch_and";
      else if (op == llvm::AtomicRMWInst::Or)
        NamePart = "_fetch_or";
      else if (op == llvm::AtomicRMWInst::Xor)
        NamePart = "_fetch_xor";
      else if (op == llvm::AtomicRMWInst::Nand)
        NamePart = "_fetch_nand";
      else
        continue;
      llvm::SmallString<32> RMWName("__tsan_atomic" + llvm::itostr(BitSize) + NamePart);
      TsanAtomicRMW[op][i] = checkSanitizerInterfaceFunction(
          M.getOrInsertFunction(RMWName, Attr, Ty, PtrTy, Ty, OrdTy));
    }

    llvm::SmallString<32> AtomicCASName("__tsan_atomic" + BitSizeStr +
                                  "_compare_exchange_val");
    TsanAtomicCAS[i] = checkSanitizerInterfaceFunction(M.getOrInsertFunction(
        AtomicCASName, Attr, Ty, PtrTy, Ty, Ty, OrdTy, OrdTy));
  }
  TsanVptrUpdate = checkSanitizerInterfaceFunction(
      M.getOrInsertFunction("__tsan_vptr_update", Attr, IRB.getVoidTy(),
                            IRB.getInt8PtrTy(), IRB.getInt8PtrTy()));
  TsanVptrLoad = checkSanitizerInterfaceFunction(M.getOrInsertFunction(
      "__tsan_vptr_read", Attr, IRB.getVoidTy(), IRB.getInt8PtrTy()));
  TsanAtomicThreadFence = checkSanitizerInterfaceFunction(M.getOrInsertFunction(
      "__tsan_atomic_thread_fence", Attr, IRB.getVoidTy(), OrdTy));
  TsanAtomicSignalFence = checkSanitizerInterfaceFunction(M.getOrInsertFunction(
      "__tsan_atomic_signal_fence", Attr, IRB.getVoidTy(), OrdTy));

  MemmoveFn = checkSanitizerInterfaceFunction(
      M.getOrInsertFunction("memmove", Attr, IRB.getInt8PtrTy(), IRB.getInt8PtrTy(),
                            IRB.getInt8PtrTy(), IntptrTy));
  MemcpyFn = checkSanitizerInterfaceFunction(
      M.getOrInsertFunction("memcpy", Attr, IRB.getInt8PtrTy(), IRB.getInt8PtrTy(),
                            IRB.getInt8PtrTy(), IntptrTy));
  MemsetFn = checkSanitizerInterfaceFunction(
      M.getOrInsertFunction("memset", Attr, IRB.getInt8PtrTy(), IRB.getInt8PtrTy(),
                            IRB.getInt32Ty(), IntptrTy));
}

static bool isVtableAccess(llvm::Instruction *I) {
  if (llvm::MDNode *Tag = I->getMetadata(llvm::LLVMContext::MD_tbaa))
    return Tag->isTBAAVtableAccess();
  return false;
}

// Do not instrument known races/"benign races" that come from compiler
// instrumentatin. The user has no way of suppressing them.
static bool shouldInstrumentReadWriteFromAddress(const llvm::Module *M,
		                                 llvm::Value *Addr) {
  // Peel off GEPs and BitCasts.
  Addr = Addr->stripInBoundsOffsets();

  if (llvm::GlobalVariable *GV = llvm::dyn_cast<llvm::GlobalVariable>(Addr)) {
    if (GV->hasSection()) {
      llvm::StringRef SectionName = GV->getSection();
      // Check if the global is in the PGO counters section.
      auto OF = llvm::Triple(M->getTargetTriple()).getObjectFormat();
      if (SectionName.endswith(llvm::getInstrProfSectionName(
            llvm::IPSK_cnts, OF, /*AddSegment=*/false)))
        return false;
    }

    // Check if the global is private gcov data.
    if (GV->getName().startswith("__llvm_gcov") ||
        GV->getName().startswith("__llvm_gcda"))
      return false;
  }

  // Do not instrument acesses from different address spaces; we cannot deal
  // with them.
  if (Addr) {
    llvm::Type *PtrTy = llvm::cast<llvm::PointerType>(Addr->getType()->getScalarType());
    if (PtrTy->getPointerAddressSpace() != 0)
      return false;
  }

  return true;
}

bool TaskSanitizer::addrPointsToConstantData(llvm::Value *Addr) {
  // If this is a GEP, just analyze its pointer operand.
  if (llvm::GetElementPtrInst *GEP = llvm::dyn_cast<llvm::GetElementPtrInst>(Addr))
    Addr = GEP->getPointerOperand();

  if (llvm::GlobalVariable *GV = llvm::dyn_cast<llvm::GlobalVariable>(Addr)) {
    if (GV->isConstant()) {
      // Reads from constant globals can not race with any writes.
      NumOmittedReadsFromConstantGlobals++;
      return true;
    }
  } else if (llvm::LoadInst *L = llvm::dyn_cast<llvm::LoadInst>(Addr)) {
    if (isVtableAccess(L)) {
      // Reads from a vtable pointer can not race with any writes.
      NumOmittedReadsFromVtable++;
      return true;
    }
  }
  return false;
}

// Instrumenting some of the accesses may be proven redundant.
// Currently handled:
//  - read-before-write (within same BB, no calls between)
//  - not captured variables
//
// We do not handle some of the patterns that should not survive
// after the classic compiler optimizations.
// E.g. two reads from the same temp should be eliminated by CSE,
// two writes should be eliminated by DSE, etc.
//
// 'Local' is a std::vector of insns within the same BB (no calls between).
// 'All' is a std::vector of insns that will be instrumented.
void TaskSanitizer::chooseInstructionsToInstrument(
    llvm::SmallVectorImpl<llvm::Instruction *> &Local, llvm::SmallVectorImpl<llvm::Instruction *> &All,
    const llvm::DataLayout &DL) {
  llvm::SmallSet<llvm::Value*, 8> WriteTargets;
  // Iterate from the end.
  for (llvm::Instruction *I : reverse(Local)) {
    if (llvm::StoreInst *Store = llvm::dyn_cast<llvm::StoreInst>(I)) {
      llvm::Value *Addr = Store->getPointerOperand();
      if (!shouldInstrumentReadWriteFromAddress(I->getModule(), Addr))
        continue;
      WriteTargets.insert(Addr);
    } else {
      llvm::LoadInst *Load = llvm::cast<llvm::LoadInst>(I);
      llvm::Value *Addr = Load->getPointerOperand();
      if (!shouldInstrumentReadWriteFromAddress(I->getModule(), Addr))
        continue;
      if (WriteTargets.count(Addr)) {
        // We will write to this temp, so no reason to analyze the read.
        NumOmittedReadsBeforeWrite++;
        continue;
      }
      if (addrPointsToConstantData(Addr)) {
        // Addr points to some constant data -- it can not race with any writes.
        continue;
      }
    }
    llvm::Value *Addr = llvm::isa<llvm::StoreInst>(*I)
        ? llvm::cast<llvm::StoreInst>(I)->getPointerOperand()
        : llvm::cast<llvm::LoadInst>(I)->getPointerOperand();
    if (llvm::isa<llvm::AllocaInst>(GetUnderlyingObject(Addr, DL)) &&
        !PointerMayBeCaptured(Addr, true, true)) {
      // The variable is addressable but not captured, so it cannot be
      // referenced from a different thread and participate in a data race
      // (see llvm/Analysis/CaptureTracking.h for details).
      NumOmittedNonCaptured++;
      continue;
    }
    All.push_back(I);
  }
  Local.clear();
}

static bool isAtomic(llvm::Instruction *I) {
  // TODO: Ask TTI whether synchronization scope is between threads.
  if (llvm::LoadInst *LI = llvm::dyn_cast<llvm::LoadInst>(I))
    return LI->isAtomic() && LI->getSyncScopeID() != llvm::SyncScope::SingleThread;
  if (llvm::StoreInst *SI = llvm::dyn_cast<llvm::StoreInst>(I))
    return SI->isAtomic() && SI->getSyncScopeID() != llvm::SyncScope::SingleThread;
  if (llvm::isa<llvm::AtomicRMWInst>(I))
    return true;
  if (llvm::isa<llvm::AtomicCmpXchgInst>(I))
    return true;
  if (llvm::isa<llvm::FenceInst>(I))
    return true;
  return false;
}

void TaskSanitizer::InsertRuntimeIgnores(llvm::Function &F) {
  llvm::IRBuilder<> IRB(F.getEntryBlock().getFirstNonPHI());
  IRB.CreateCall(TsanIgnoreBegin);
  llvm::EscapeEnumerator EE(F, "tsan_ignore_cleanup", ClHandleCxxExceptions);
  while (llvm::IRBuilder<> *AtExit = EE.Next()) {
    AtExit->CreateCall(TsanIgnoreEnd);
  }
}

bool TaskSanitizer::runOnFunction(llvm::Function &F) {
  // This is required to prevent instrumenting call to
  // __tsan_init from within the module constructor.
  if (&F == TsanCtorFunction)
    return false;

  bool Res = false;

  tasan::IIRlog::logTaskBody(F, tasan::util::getPlainFuncName(F));

  // Register function name
  llvm::StringRef funcName = tasan::util::demangleName(F.getName());
  llvm::IRBuilder<> IRB(F.getEntryBlock().getFirstNonPHI());
  funcNamePtr = IRB.CreateGlobalStringPtr(funcName, "functionName");

  initializeCallbacks(*F.getParent());
  llvm::SmallVector<llvm::Instruction*, 8> AllLoadsAndStores;
  llvm::SmallVector<llvm::Instruction*, 8> LocalLoadsAndStores;
  llvm::SmallVector<llvm::Instruction*, 8> AtomicAccesses;
  llvm::SmallVector<llvm::Instruction*, 8> MemIntrinCalls;

  bool HasCalls = false;
  bool SanitizeFunction = true; //HASSAN F.hasFnAttribute(Attribute::SanitizeThread);
  const llvm::DataLayout &DL = F.getParent()->getDataLayout();
  const llvm::TargetLibraryInfo *TLI =
      &getAnalysis<llvm::TargetLibraryInfoWrapperPass>().getTLI();

  // Traverse all instructions, collect loads/stores/returns, check for calls.
  for (auto &BB : F) {
    for (auto &Inst : BB) {
      if (isAtomic(&Inst))
        AtomicAccesses.push_back(&Inst);
      else if (llvm::isa<llvm::LoadInst>(Inst) || llvm::isa<llvm::StoreInst>(Inst))
        LocalLoadsAndStores.push_back(&Inst);
      else if (llvm::isa<llvm::CallInst>(Inst) || llvm::isa<llvm::InvokeInst>(Inst)) {
        if (llvm::CallInst *CI = llvm::dyn_cast<llvm::CallInst>(&Inst))
          maybeMarkSanitizerLibraryCallNoBuiltin(CI, TLI);
        if (llvm::isa<llvm::MemIntrinsic>(Inst))
          MemIntrinCalls.push_back(&Inst);
        HasCalls = true;
        chooseInstructionsToInstrument(LocalLoadsAndStores, AllLoadsAndStores,
                                       DL);
      }
    }
    chooseInstructionsToInstrument(LocalLoadsAndStores, AllLoadsAndStores, DL);
  }

  // We have collected all loads and stores.
  // FIXME: many of these accesses do not need to be checked for races
  // (e.g. variables that do not escape, etc).

  // Instrument memory accesses only if we want to report bugs in the function.
  if (ClInstrumentMemoryAccesses && SanitizeFunction)
    for (auto Inst : AllLoadsAndStores) {
      Res |= instrumentLoadOrStore(Inst, DL);
    }

  // Instrument atomic memory accesses in any case (they can be used to
  // implement synchronization).
  if (ClInstrumentAtomics)
    for (auto Inst : AtomicAccesses) {
      Res |= instrumentAtomic(Inst, DL);
    }

  if (ClInstrumentMemIntrinsics && SanitizeFunction)
    for (auto Inst : MemIntrinCalls) {
      Res |= instrumentMemIntrinsic(Inst);
    }

  if (F.hasFnAttribute("sanitize_thread_no_checking_at_run_time")) {
    assert(!F.hasFnAttribute(llvm::Attribute::SanitizeThread));
    if (HasCalls)
      InsertRuntimeIgnores(F);
  }

  // save full path name of .iir log file
  if ( tasan::util::isMainFunction(F) ) {
    llvm::IRBuilder<> IRB(F.getEntryBlock().getFirstNonPHI());
    IIRfile = IRB.CreateGlobalStringPtr(IIRfileURL, "iirFileLoc");
    IRB.CreateCall(RegisterIIRfile,
                   IRB.CreatePointerCast(IIRfile, IRB.getInt8PtrTy()));
  }

  // Instrument function entry/exit points if there were instrumented accesses.
  if ((Res || HasCalls) && ClInstrumentFuncEntryExit) {
    llvm::IRBuilder<> IRB(F.getEntryBlock().getFirstNonPHI());
    llvm::Value *ReturnAddress = IRB.CreateCall(
        llvm::Intrinsic::getDeclaration(F.getParent(), llvm::Intrinsic::returnaddress),
        IRB.getInt32(0));
    IRB.CreateCall(TsanFuncEntry, ReturnAddress);

    llvm::EscapeEnumerator EE(F, "tsan_cleanup", ClHandleCxxExceptions);
    while (llvm::IRBuilder<> *AtExit = EE.Next()) {
      AtExit->CreateCall(TsanFuncExit, {funcNamePtr});
    }
    Res = true;
  }
  return Res;
}

bool TaskSanitizer::instrumentLoadOrStore(llvm::Instruction *I,
                                            const llvm::DataLayout &DL) {
  llvm::IRBuilder<> IRB(I);
  bool IsWrite = llvm::isa<llvm::StoreInst>(*I);
  llvm::Value *Addr = IsWrite
      ? llvm::cast<llvm::StoreInst>(I)->getPointerOperand()
      : llvm::cast<llvm::LoadInst>(I)->getPointerOperand();

  // swifterror memory addresses are mem2reg promoted by instruction selection.
  // As such they cannot have regular uses like an instrumentation function and
  // it makes no sense to track them as memory.
  if (Addr->isSwiftError())
    return false;

  int Idx = getMemoryAccessFuncIndex(Addr, DL);
  if (Idx < 0)
    return false;
  if (IsWrite && isVtableAccess(I)) {
    DEBUG(llvm::dbgs() << "  VPTR : " << *I << "\n");
    llvm::Value *StoredValue = llvm::cast<llvm::StoreInst>(I)->getValueOperand();
    // StoredValue may be a std::vector type if we are storing several vptrs at once.
    // In this case, just take the first element of the std::vector since this is
    // enough to find vptr races.
    if (llvm::isa<llvm::VectorType>(StoredValue->getType()))
      StoredValue = IRB.CreateExtractElement(
          StoredValue, llvm::ConstantInt::get(IRB.getInt32Ty(), 0));
    if (StoredValue->getType()->isIntegerTy())
      StoredValue = IRB.CreateIntToPtr(StoredValue, IRB.getInt8PtrTy());
    // Call TsanVptrUpdate.
    IRB.CreateCall(TsanVptrUpdate,
                   {IRB.CreatePointerCast(Addr, IRB.getInt8PtrTy()),
                    IRB.CreatePointerCast(StoredValue, IRB.getInt8PtrTy())});
    NumInstrumentedVtableWrites++;
    return true;
  }
  if (!IsWrite && isVtableAccess(I)) {
    IRB.CreateCall(TsanVptrLoad,
                   IRB.CreatePointerCast(Addr, IRB.getInt8PtrTy()));
    NumInstrumentedVtableReads++;
    return true;
  }
  const unsigned Alignment = IsWrite
      ? llvm::cast<llvm::StoreInst>(I)->getAlignment()
      : llvm::cast<llvm::LoadInst>(I)->getAlignment();
  llvm::Type *OrigTy = llvm::cast<llvm::PointerType>(Addr->getType())->getElementType();
  const uint32_t TypeSize = DL.getTypeStoreSizeInBits(OrigTy);
  llvm::Value *OnAccessFunc = nullptr;
  if (Alignment == 0 || Alignment >= 8 || (Alignment % (TypeSize / 8)) == 0)
    OnAccessFunc = IsWrite ? TsanWrite[Idx] : TsanRead[Idx];
  else
    OnAccessFunc = IsWrite ? TsanUnalignedWrite[Idx] : TsanUnalignedRead[Idx];

  if (IsWrite) {
      llvm::Value *Val = llvm::cast<llvm::StoreInst>(I)->getValueOperand();
      if ( Val->getType()->isFloatTy() )
          OnAccessFunc = FlowSan_MemWriteFloat;
      else if ( Val->getType()->isDoubleTy() )
          OnAccessFunc = FlowSan_MemWriteDouble;

      IRB.CreateCall(OnAccessFunc,
          {IRB.CreatePointerCast(Addr, IRB.getInt8PtrTy()), Val,
           IRB.CreateIntCast(tasan::debug::getLineNumber(I),
                             IRB.getInt8Ty(),
                             false),
           IRB.CreatePointerCast(funcNamePtr, IRB.getInt8PtrTy())});
  } else {
    IRB.CreateCall(OnAccessFunc,
        {IRB.CreatePointerCast(Addr, IRB.getInt8PtrTy()),
         IRB.CreateIntCast(tasan::debug::getLineNumber(I),
                           IRB.getInt8Ty(),
                           false),
         IRB.CreatePointerCast(funcNamePtr, IRB.getInt8PtrTy())});
  }

  if (IsWrite) NumInstrumentedWrites++;
  else         NumInstrumentedReads++;
  return true;
}

static llvm::ConstantInt *createOrdering(llvm::IRBuilder<> *IRB, llvm::AtomicOrdering ord) {
  uint32_t v = 0;
  switch (ord) {
    case llvm::AtomicOrdering::NotAtomic:
      llvm_unreachable("unexpected atomic ordering!");
    case llvm::AtomicOrdering::Unordered:              LLVM_FALLTHROUGH;
    case llvm::AtomicOrdering::Monotonic:              v = 0; break;
    // Not specified yet:
    // case llvm::AtomicOrdering::Consume:                v = 1; break;
    case llvm::AtomicOrdering::Acquire:                v = 2; break;
    case llvm::AtomicOrdering::Release:                v = 3; break;
    case llvm::AtomicOrdering::AcquireRelease:         v = 4; break;
    case llvm::AtomicOrdering::SequentiallyConsistent: v = 5; break;
  }
  return IRB->getInt32(v);
}

// If a memset intrinsic gets inlined by the code gen, we will miss races on it.
// So, we either need to ensure the intrinsic is not inlined, or instrument it.
// We do not instrument memset/memmove/memcpy intrinsics (too complicated),
// instead we simply replace them with regular function calls, which are then
// intercepted by the run-time.
// Since tsan is running after everyone else, the calls should not be
// replaced back with intrinsics. If that becomes wrong at some point,
// we will need to call e.g. __tsan_memset to avoid the intrinsics.
bool TaskSanitizer::instrumentMemIntrinsic(llvm::Instruction *I) {
  llvm::IRBuilder<> IRB(I);
  if (llvm::MemSetInst *M = llvm::dyn_cast<llvm::MemSetInst>(I)) {
    IRB.CreateCall(
        MemsetFn,
        {IRB.CreatePointerCast(M->getArgOperand(0), IRB.getInt8PtrTy()),
         IRB.CreateIntCast(M->getArgOperand(1), IRB.getInt32Ty(), false),
         IRB.CreateIntCast(M->getArgOperand(2), IntptrTy, false)});
    I->eraseFromParent();
  } else if (llvm::MemTransferInst *M = llvm::dyn_cast<llvm::MemTransferInst>(I)) {
    IRB.CreateCall(
        llvm::isa<llvm::MemCpyInst>(M) ? MemcpyFn : MemmoveFn,
        {IRB.CreatePointerCast(M->getArgOperand(0), IRB.getInt8PtrTy()),
         IRB.CreatePointerCast(M->getArgOperand(1), IRB.getInt8PtrTy()),
         IRB.CreateIntCast(M->getArgOperand(2), IntptrTy, false)});
    I->eraseFromParent();
  }
  return false;
}

// Both llvm and ThreadSanitizer atomic operations are based on C++11/C1x
// standards.  For background see C++11 standard.  A slightly older, publicly
// available draft of the standard (not entirely up-to-date, but close enough
// for casual browsing) is available here:
// http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2011/n3242.pdf
// The following page contains more background information:
// http://www.hpl.hp.com/personal/Hans_Boehm/c++mm/

bool TaskSanitizer::instrumentAtomic(llvm::Instruction *I, const llvm::DataLayout &DL) {
  llvm::IRBuilder<> IRB(I);
  if (llvm::LoadInst *LI = llvm::dyn_cast<llvm::LoadInst>(I)) {
    llvm::Value *Addr = LI->getPointerOperand();
    int Idx = getMemoryAccessFuncIndex(Addr, DL);
    if (Idx < 0)
      return false;
    const unsigned ByteSize = 1U << Idx;
    const unsigned BitSize = ByteSize * 8;
    llvm::Type *Ty = llvm::Type::getIntNTy(IRB.getContext(), BitSize);
    llvm::Type *PtrTy = Ty->getPointerTo();
    llvm::Value *Args[] = {IRB.CreatePointerCast(Addr, PtrTy),
                     createOrdering(&IRB, LI->getOrdering())};
    llvm::Type *OrigTy = llvm::cast<llvm::PointerType>(Addr->getType())->getElementType();
    llvm::Value *C = IRB.CreateCall(TsanAtomicLoad[Idx], Args);
    llvm::Value *Cast = IRB.CreateBitOrPointerCast(C, OrigTy);
    I->replaceAllUsesWith(Cast);
  } else if (llvm::StoreInst *SI = llvm::dyn_cast<llvm::StoreInst>(I)) {
    llvm::Value *Addr = SI->getPointerOperand();
    int Idx = getMemoryAccessFuncIndex(Addr, DL);
    if (Idx < 0)
      return false;
    const unsigned ByteSize = 1U << Idx;
    const unsigned BitSize = ByteSize * 8;
    llvm::Type *Ty = llvm::Type::getIntNTy(IRB.getContext(), BitSize);
    llvm::Type *PtrTy = Ty->getPointerTo();
    llvm::Value *Args[] = {IRB.CreatePointerCast(Addr, PtrTy),
                     IRB.CreateBitOrPointerCast(SI->getValueOperand(), Ty),
                     createOrdering(&IRB, SI->getOrdering())};
    llvm::CallInst *C = llvm::CallInst::Create(TsanAtomicStore[Idx], Args);
    ReplaceInstWithInst(I, C);
  } else if (llvm::AtomicRMWInst *RMWI = llvm::dyn_cast<llvm::AtomicRMWInst>(I)) {
    llvm::Value *Addr = RMWI->getPointerOperand();
    int Idx = getMemoryAccessFuncIndex(Addr, DL);
    if (Idx < 0)
      return false;
    llvm::Function *F = TsanAtomicRMW[RMWI->getOperation()][Idx];
    if (!F)
      return false;
    const unsigned ByteSize = 1U << Idx;
    const unsigned BitSize = ByteSize * 8;
    llvm::Type *Ty = llvm::Type::getIntNTy(IRB.getContext(), BitSize);
    llvm::Type *PtrTy = Ty->getPointerTo();
    llvm::Value *Args[] = {IRB.CreatePointerCast(Addr, PtrTy),
                     IRB.CreateIntCast(RMWI->getValOperand(), Ty, false),
                     createOrdering(&IRB, RMWI->getOrdering())};
    llvm::CallInst *C = llvm::CallInst::Create(F, Args);
    ReplaceInstWithInst(I, C);
  } else if (llvm::AtomicCmpXchgInst *CASI = llvm::dyn_cast<llvm::AtomicCmpXchgInst>(I)) {
    llvm::Value *Addr = CASI->getPointerOperand();
    int Idx = getMemoryAccessFuncIndex(Addr, DL);
    if (Idx < 0)
      return false;
    const unsigned ByteSize = 1U << Idx;
    const unsigned BitSize = ByteSize * 8;
    llvm::Type *Ty = llvm::Type::getIntNTy(IRB.getContext(), BitSize);
    llvm::Type *PtrTy = Ty->getPointerTo();
    llvm::Value *CmpOperand =
      IRB.CreateBitOrPointerCast(CASI->getCompareOperand(), Ty);
    llvm::Value *NewOperand =
      IRB.CreateBitOrPointerCast(CASI->getNewValOperand(), Ty);
    llvm::Value *Args[] = {IRB.CreatePointerCast(Addr, PtrTy),
                     CmpOperand,
                     NewOperand,
                     createOrdering(&IRB, CASI->getSuccessOrdering()),
                     createOrdering(&IRB, CASI->getFailureOrdering())};
    llvm::CallInst *C = IRB.CreateCall(TsanAtomicCAS[Idx], Args);
    llvm::Value *Success = IRB.CreateICmpEQ(C, CmpOperand);
    llvm::Value *OldVal = C;
    llvm::Type *OrigOldValTy = CASI->getNewValOperand()->getType();
    if (Ty != OrigOldValTy) {
      // The value is a pointer, so we need to cast the return value.
      OldVal = IRB.CreateIntToPtr(C, OrigOldValTy);
    }

    llvm::Value *Res =
      IRB.CreateInsertValue(llvm::UndefValue::get(CASI->getType()), OldVal, 0);
    Res = IRB.CreateInsertValue(Res, Success, 1);

    I->replaceAllUsesWith(Res);
    I->eraseFromParent();
  } else if (llvm::FenceInst *FI = llvm::dyn_cast<llvm::FenceInst>(I)) {
    llvm::Value *Args[] = {createOrdering(&IRB, FI->getOrdering())};
    llvm::Function *F = FI->getSyncScopeID() == llvm::SyncScope::SingleThread ?
        TsanAtomicSignalFence : TsanAtomicThreadFence;
    llvm::CallInst *C = llvm::CallInst::Create(F, Args);
    ReplaceInstWithInst(I, C);
  }
  return true;
}

int TaskSanitizer::getMemoryAccessFuncIndex(llvm::Value *Addr,
                                              const llvm::DataLayout &DL) {
  llvm::Type *OrigPtrTy = Addr->getType();
  llvm::Type *OrigTy = llvm::cast<llvm::PointerType>(OrigPtrTy)->getElementType();
  assert(OrigTy->isSized());
  uint32_t TypeSize = DL.getTypeStoreSizeInBits(OrigTy);
  if (TypeSize != 8  && TypeSize != 16 &&
      TypeSize != 32 && TypeSize != 64 && TypeSize != 128) {
    NumAccessesWithBadSize++;
    // Ignore all unusual sizes.
    return -1;
  }
  size_t Idx = llvm::countTrailingZeros(TypeSize / 8);
  assert(Idx < kNumberOfAccessSizes);
  return Idx;
}
