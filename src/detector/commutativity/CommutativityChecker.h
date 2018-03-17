/////////////////////////////////////////////////////////////////
//  TaskSanitizer: a lightweight non-determinism checking
//          tool for OpenMP task applications
//
//    Copyright (c) 2015 - 2018 Hassan Salehe Matar
//      Copying or using this code by any means whatsoever
//      without consent of the owner is strictly prohibited.
//
//   Contact: hmatar-at-ku-dot-edu-dot-tr
//
/////////////////////////////////////////////////////////////////

// Defines the Validator class.
// It determines whether the bugs detected are real.

#ifndef _DETECTOR_COMMUTATIVITY_COMMUTATIVITYCHECKER_H_
#define _DETECTOR_COMMUTATIVITY_COMMUTATIVITYCHECKER_H_

// includes and definitions
#include "defs.h"
#include "instruction.h"
#include "operationSet.h"
#include "conflict.h"
#include "report.h"
#include "common/CriticalSignatures.h"
#include "detector/commutativity/CriticalSections.h"

class CommutativityChecker {

  public:
    VOID parseTasksIR(char * IRlogName);
    bool isCommutative(const Conflict & conflict);

  private:
    tasan::commute::CriticalSections Tasks;
    bool involveSimpleOperations(/*std::string task1, */INTEGER line1);
    bool isSafe(const std::vector<Instruction> & trace, INTEGER loc,
                std::string operand);
    //INTEGER getLineNumber(const std::string & statement);

    OperationSet operationSet;

    // Helper functions
    inline bool isEmpty(const std::string& statement) {
      return statement.find_first_not_of(' ') == std::string::npos;
    }

    inline bool isDebugCall(const std::string& statement) {
      return statement.find("llvm.dbg.declare") != std::string::npos;
    }

    inline bool isValidStatement(const std::string& sttmt) {
      // valid starts with line number, e.g. "42: "
      return regex_search (sttmt, std::regex("^[0-9]+: "));
    }

    inline bool isCriticalSectionStart(const std::string& sttmt) {
      return tasan::getStartCriticalSignature() == sttmt;
    }

    inline bool isCriticalSectionEnd(const std::string& sttmt) {
      return tasan::getEndCriticalSignature() == sttmt;
    }
    /**
     * Returns the line number from the IR statement std::string
     */
    INTEGER getLineNumber(const std::string & sttmt) {
      std::smatch result; // get line number
      regex_search(sttmt, result, std::regex("^[0-9]+") );

      if (result.size() <= 0 || result.size()> 1 ) {
        // a line should have only one line number
        std::cerr << "Incorrect no. of lines: " << sttmt << std::endl;
        exit(EXIT_FAILURE);
      }
      return stoul(result[0]);
    }

    Instruction makeStoreInstruction(
        const std::vector<std::string> & contents) {
      Instruction instr;
      instr.oper = STORE;
      instr.destination = contents[4];
      instr.operand1 = contents[2];
      instr.operand2 = contents[2];
      instr.type = contents[1];
      return instr;
    }
};


#endif // end validator.h
