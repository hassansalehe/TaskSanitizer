/////////////////////////////////////////////////////////////////
//  TaskSanitizer: a lightweight non-determinism checking
//          tool for OpenMP applications
//
//    Copyright (c) 2015 - 2018 Hassan Salehe Matar
//      Copying or using this code by any means whatsoever
//      without consent of the owner is strictly prohibited.
//
//   Contact: hmatar-at-ku-dot-edu-dot-tr
//
/////////////////////////////////////////////////////////////////

// Include for the instrumentation passes.

#ifndef IIRLOGGER_H_P_P
#define IIRLOGGER_H_P_P

#include "Libs.h" // all LLVM includes stored there
#include "DebugInfoHelper.h"
#include "CriticalSignatures.h"

/// general namespace for TaskSanitizer tool
namespace tasan {

/// This namespace contains utilities for parsing program
/// function bodies for locating critical sections.
/// Then program statements, in the form of IIR representation,
/// in these critical sections are logged into .iir file.
/// The .iir file is later loaded into memory at runtime to check
/// commutativity among critical sections if nondeterminism is
/// detected among them.
namespace IIRlog {

  // the log out stream
  ofstream logFile;

  /**
   * Opens an output log file which is later used for
   * logging all instructions in program's critical sections.
   */
  void InitializeLogger( StringRef cppName ) {
    std::string full_file_name = "" + cppName.str() + ".iir";

    logFile.open(full_file_name,  ofstream::out | ofstream::trunc);
    if ( !logFile.is_open() ) {
      errs() << "FILE NO OPEN \n";
    }
  }


  /**
   * Saves a string to a log file
   */
  void SaveToLogFile( StringRef taskName ) {
     logFile << taskName.str() << std::endl << flush;
  }

  /**
   * Saves IIR represenation of instruction and its
   * corresponding line number to a file.
   *
   * This function is used in logging instructions which
   * are in critical sections of a program.
   */
  void LogNewIIRcode(int lineNo, Instruction& IIRcode ) {
    //errs() << lineNo << ": " << IIRcode.str() << "\n";
    std::string tempBuf;
    raw_string_ostream rso(tempBuf);
    IIRcode.print(rso);
    logFile << lineNo << ": " << tempBuf << std::endl;
  }

  /**
   * Returns signature (function name) of call being made
   */
  StringRef getCalledFunction(Instruction & Inst) {
    if ( llvm::isa<llvm::CallInst>(Inst) ) {
      llvm::CallInst *M = llvm::dyn_cast<llvm::CallInst>(&Inst);
      llvm::Function *calledF = M->getCalledFunction();
      if (calledF) {
        return tasan::util::demangleName(calledF->getName());
      }
    } else if ( llvm::isa<llvm::InvokeInst>(Inst) ) {
      llvm::InvokeInst *M = llvm::dyn_cast<llvm::InvokeInst>(&Inst);
      llvm::Function *calledF = M->getCalledFunction();
      if (calledF) {
        return tasan::util::demangleName(calledF->getName());
      }
    }

    // everything failed
    return llvm::StringRef("");
  }

  /**
   * Checks if the function call is for acquiring a lock
   */
  bool isLockInvocation(Instruction & Instr) {
    StringRef funcName = getCalledFunction(Instr);
    if (funcName.find("omp_set_lock") != StringRef::npos ||
        funcName.find("__kmpc_critical") != StringRef::npos) {
      return true;
    } else {
      return false;
    }
  }

  /**
   * Checks if function call is related to lock release
   */
  bool isUnlockInvocation(Instruction & Instr) {
    StringRef funcName = getCalledFunction(Instr);
    if (funcName.find("omp_unset_lock") != StringRef::npos ||
        funcName.find("__kmpc_end_critical") != StringRef::npos) {
      return true;
    } else {
      return false;
    }
  }

  /**
   * Logs all statements in critical sections for commutativity
   * checking in verification of nondererminism bugs
   */
  void logTaskBody(Function & F, StringRef name) {

    int in_critical_section = 0;

    // search for critical sections in the whole function body
    for (auto &BB : F) {
      for (auto &Inst : BB) {

        if ( isLockInvocation(Inst) ) { // set critical section
          if (in_critical_section == 0) {
            IIRlog::SaveToLogFile( tasan::getStartCriticalSignature() );
          }
          in_critical_section++;
        } else if ( isUnlockInvocation(Inst) ) { // exit critical section
          if (in_critical_section > 0) in_critical_section--;
          if (in_critical_section == 0) {
            IIRlog::SaveToLogFile( tasan::getEndCriticalSignature() );
          }
        } else if (in_critical_section > 0) { // in critical section
          unsigned lineNo = 0;
          if (auto Loc = Inst.getDebugLoc()) {
            lineNo = Loc->getLine();
          }
          IIRlog::LogNewIIRcode(lineNo, Inst);
        }
      }
    }
  }
} // end IIRlog namespace

} // end tasan namespace

#endif
