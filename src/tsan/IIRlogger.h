/////////////////////////////////////////////////////////////////
//  FlowSanitizer: a lightweight non-determinism checking
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

using namespace llvm;
using namespace std;

namespace IIRlog {

  // the log out stream
  ofstream logFile;

  void Init( StringRef cppName ) {
    errs() <<  "File name will be " << cppName << "\n";
    logFile.open("" + cppName.str() + ".iir",  ofstream::out | ofstream::trunc);
    if( !logFile.is_open() )
      errs() << "FILE NO OPEN \n";
  }

  void LogNewTask( StringRef taskName ) {

     errs() << taskName.str() << "\n";
     logFile << taskName.str() << endl << flush;
  }

  void LogNewIIRcode(int lineNo, Instruction& IIRcode ) {
    //errs() << lineNo << ": " << IIRcode.str() << "\n";
    string tempBuf;
    raw_string_ostream rso(tempBuf);
    IIRcode.print(rso);
    logFile << lineNo << ": " << tempBuf << endl;
  }

  /**
   * Logs all task body statements for verification of
   * nondererminism bugs
   */
  void logTaskBody(Function & F, StringRef name) {

    IIRlog::LogNewTask( name );
    for (auto &BB : F) {
      for (auto &Inst : BB) {
        unsigned lineNo = 0;
         if(auto Loc = Inst.getDebugLoc())
           lineNo = Loc->getLine();
         IIRlog::LogNewIIRcode( lineNo, Inst);
      }
    }
  }
}

#endif
