/////////////////////////////////////////////////////////////////
//  TaskSanitizer: a lightweight determinacy race checking
//          tool for OpenMP applications
//
//    Copyright (c) 2015 - 2018 Hassan Salehe Matar
//      Copying or using this code by any means whatsoever
//      without consent of the owner is strictly prohibited.
//
//   Contact: hassansalehe-at-gmail-dot-com
//
/////////////////////////////////////////////////////////////////

// Include for the instrumentation passes.

#ifndef _INSTRUMENTOR_PASS_IIRLOGGER_H_
#define _INSTRUMENTOR_PASS_IIRLOGGER_H_

#include "instrumentor/pass/LLVMLibs.h" // all LLVM includes stored there
#include "instrumentor/pass/DebugInfoHelper.h"
#include "common/CriticalSignatures.h"

/// general namespace for TaskSanitizer tool
namespace tasksan {

/// This namespace contains utilities for parsing program
/// function bodies for locating critical sections.
/// Then program statements, in the form of IIR representation,
/// in these critical sections are logged into .iir file.
/// The .iir file is later loaded into memory at runtime to check
/// commutativity among critical sections if determinacy race is
/// detected among them.
namespace IIRlog {

  // the log out stream
  std::ofstream logFile;

  // Opens an output log file which is later used for
  // logging all instructions in program's critical sections.
  void InitializeLogger( std::string cppName ) {
    std::string full_file_name = "" + cppName + ".iir";

    if ( logFile.is_open() ) logFile.close();

    logFile.open(full_file_name, std::ofstream::out | std::ofstream::app);
    if ( !logFile.is_open() ) {
      llvm::errs() << "FILE NO OPEN \n";
    }
  }


  // Saves a string to a log file
  void SaveToLogFile( llvm::StringRef taskName ) {
     logFile << taskName.str() << std::endl << std::flush;
  }

  // Saves IIR represenation of instruction and its
  // corresponding line number to a file.
  //
  // This function is used in logging instructions which
  // are in critical sections of a program.
  void LogNewIIRcode(int source_line_num, llvm::Instruction& IIRcode ) {
    //errs() << source_line_num << ": " << IIRcode.str() << "\n";
    std::string tempBuf;
    llvm::raw_string_ostream rso(tempBuf);
    IIRcode.print(rso);
    logFile << source_line_num << ": " << tempBuf << std::endl;
  }

  // Returns signature (function name) of call being made
  llvm::StringRef getCalledFunction(llvm::Instruction & Inst) {
    if ( llvm::isa<llvm::CallInst>(Inst) ) {
      llvm::CallInst *M = llvm::dyn_cast<llvm::CallInst>(&Inst);
      llvm::Function *calledF = M->getCalledFunction();
      if (calledF) {
        return tasksan::util::demangleName(calledF->getName());
      }
    } else if ( llvm::isa<llvm::InvokeInst>(Inst) ) {
      llvm::InvokeInst *M = llvm::dyn_cast<llvm::InvokeInst>(&Inst);
      llvm::Function *calledF = M->getCalledFunction();
      if (calledF) {
        return tasksan::util::demangleName(calledF->getName());
      }
    }

    // everything failed
    return llvm::StringRef("");
  }

  // Checks if the function call is for acquiring a lock
  bool isLockInvocation(llvm::Instruction & Instr) {
    llvm::StringRef funcName = getCalledFunction(Instr);
    if (funcName.find("omp_set_lock") != llvm::StringRef::npos ||
        funcName.find("__kmpc_critical") != llvm::StringRef::npos) {
      return true;
    } else {
      return false;
    }
  }

  // Checks if function call is related to lock release
  bool isUnlockInvocation(llvm::Instruction & Instr) {
    llvm::StringRef funcName = getCalledFunction(Instr);
    if (funcName.find("omp_unset_lock") != llvm::StringRef::npos ||
        funcName.find("__kmpc_end_critical") != llvm::StringRef::npos) {
      return true;
    } else {
      return false;
    }
  }

  // Logs all statements in critical sections for commutativity
  // checking in verification of determinacy races
  void logTaskBody(llvm::Function & F, llvm::StringRef name) {

    std::string fullFileName = tasksan::debug::getFilename(F);
    if (fullFileName == "Unknown") {
      return;
    }

    int in_critical_section = 0;

    // search for critical sections in the whole function body
    for (auto &BB : F) {
      for (auto &Inst : BB) {

        if ( isLockInvocation(Inst) ) { // set critical section
          InitializeLogger(fullFileName);
          if (in_critical_section == 0) {
            IIRlog::SaveToLogFile( tasksan::getStartCriticalSignature() );
          }
          in_critical_section++;
        } else if ( isUnlockInvocation(Inst) ) { // exit critical section
          if (in_critical_section > 0) in_critical_section--;
          if (in_critical_section == 0) {
            IIRlog::SaveToLogFile( tasksan::getEndCriticalSignature() );
          }
        } else if (in_critical_section > 0) { // in critical section
          unsigned source_line_num = 0;
          if (auto Loc = Inst.getDebugLoc()) {
            source_line_num = Loc->getLine();
          }
          IIRlog::LogNewIIRcode(source_line_num, Inst);
        }
      }
    }
  }
} // end IIRlog namespace

} // end tasksan namespace

#endif
