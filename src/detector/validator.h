/////////////////////////////////////////////////////////////////
//  ADFinspec: a lightweight non-determinism checking
//          tool for ADF applications
//
//    Copyright (c) 2015 - 2017 Hassan Salehe Matar & MSRC at Koc University
//      Copying or using this code by any means whatsoever
//      without consent of the owner is strictly prohibited.
//
//   Contact: hmatar-at-ku-dot-edu-dot-tr
//
/////////////////////////////////////////////////////////////////

// Defines the Validator class. It determines whether the bugs detected are real.

#ifndef _VALIDATOR_HPP_
#define _VALIDATOR_HPP_

// includes and definitions
#include "defs.h"
#include "instruction.h"
#include "operationSet.h"
#include "conflictReport.h"

class BugValidator {

  public:
    VOID parseTasksIR(char * IRlogName);
    void validate(Report & report);

  private:
    unordered_map<string, vector<Instruction>> Tasks;
    bool involveSimpleOperations(string task1, INTEGER line1);
    bool isSafe(const vector<Instruction> & trace, INTEGER loc, string operand);
    //INTEGER getLineNumber(const string & statement);

    OperationSet operationSet;

    // Helper functions
    inline bool isEmpty(const string& statement) {
      return statement.find_first_not_of(' ') == string::npos;
    }

    inline bool isDebugCall(const string& statement) {
      return statement.find("llvm.dbg.declare") != string::npos;
    }

    inline bool isValid(const string& sttmt) {
      // valid starts with line number, e.g. "42: "
      return regex_search (sttmt, regex("^[0-9]+: "));
    }

    inline bool isTaskName(const string& sttmt) {
      return sttmt.find_first_of(' ') == string::npos;
    }

    /**
     * Returns the line number from the IR statement string
     */
    INTEGER getLineNumber(const string & sttmt) {
      smatch result; // get line number
      regex_search(sttmt, result, regex("^[0-9]+") );

      if(result.size() <= 0 || result.size()> 1 ) {
        // a line should have only one line number
        cerr << "Incorrect number of lines: " << sttmt << endl;
        exit(EXIT_FAILURE);
      }
      return stoul(result[0]);
    }

    Instruction makeStoreInstruction(const vector<string> & contents) {
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
