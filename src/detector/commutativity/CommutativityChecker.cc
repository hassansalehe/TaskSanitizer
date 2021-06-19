/////////////////////////////////////////////////////////////////
//  TaskSanitizer: a lightweight determinacy race checking
//          tool for OpenMP task applications
//
//    Copyright (c) 2015 - 2018 Hassan Salehe Matar
//      Copying or using this code by any means whatsoever
//      without consent of the owner is strictly prohibited.
//
//   Contact: hassansalehe-at-gmail-dot-com
//
/////////////////////////////////////////////////////////////////

// Defines a class which verifies that bugs detected are not commutative

// includes and definitions
#include "detector/commutativity/CommutativityChecker.h"
#include "detector/determinacy/conflict.h"
#include "detector/determinacy/report.h"

// Parses IIR representation file for critical sections
VOID CommutativityChecker::parseTasksIR(char * IRlogName) {
  std::vector<Instruction>  currentTask;
  std::string               sttmt;             // program statement
  std::ifstream             IRcode(IRlogName); // open IRlog file

  while ( getline(IRcode, sttmt) ) {
    if ( isEmpty(sttmt) ) continue;            // skip empty line
    if ( isDebugCall(sttmt) ) continue;        // skip debug call

    sttmt = Instruction::trim( sttmt );        // trim spaces
    if ( isValidStatement(sttmt) ) {           // check if normal statement
      INTEGER lineNo = getLineNumber( sttmt );
      // skip instruction with line # 0: args to task body
      if (lineNo <= 0) continue;

      sttmt = sttmt.substr(sttmt.find_first_not_of(' ',
                           sttmt.find_first_of(' ')));
      Instruction instr( sttmt );
      instr.lineNo = lineNo;
      currentTask.push_back(instr);
      continue;
    } // if

    if ( isCriticalSectionStart(sttmt) ) {      // new critical sec.
      currentTask = std::vector<Instruction>();
    }

    if ( isCriticalSectionEnd(sttmt) ) {        // end critical sec.
      Tasks.insert( currentTask );
    }
  } // end while
  IRcode.close();
  std::cout << "No. of critical sections in IIR: " 
            << Tasks.getSize() << std::endl;
}

// Checks for commutative critical sections operations which have been
// flagged as conflicts.
bool CommutativityChecker::isCommutative(const Conflict & conflict) {

  // skip commutativity check if read-write conflict
  if (conflict.action1.isWrite != conflict.action2.isWrite) {
    return false;
  }
  INTEGER line1 = conflict.action1.lineNo;
  INTEGER line2 = conflict.action2.lineNo;
  operationSet.clear(); // clear set of commuting operations

  // check if line1 operations commute & line2 operations commute
  if ( involveSimpleOperations( /*function ID*/ line1 ) &&
       involveSimpleOperations( /*function ID*/ line2 ) ) {
    return true;
  } else {
    return false;
  }
}

BOOL CommutativityChecker::involveSimpleOperations(
    // std::string taskName,
    INTEGER lineNumber) {

  // get the instructions of a task
  tasksan::commute::CriticalSectionBody *taskBody = Tasks.find(lineNumber);
  if (nullptr == taskBody) return false;

  Instruction instr;
  INTEGER index = -1;

  for (auto i = taskBody->begin(); i != taskBody->end(); i++) {
     if (i->lineNo > lineNumber) break;
     if (i->lineNo == lineNumber) instr  = *i;
     index++;
  }
  
  // expected to be a store
  if (instr.oper == STORE) {
    return isSafe(taskBody->getCriticalSectionBody(),
        index, instr.operand1);
    //bool r1 = isOnsimpleOperations(lineNumber - 1, istr.destination);
    //bool r2 = isOnsimpleOperations(lineNumber - 1, istr.operand1);
  }
  return false;
}

bool CommutativityChecker::isSafe(
    const std::vector<Instruction> & taskBody,
    INTEGER loc,
    std::string operand) {

  if (loc < 0) {
    if ( !operationSet.size() )              return false;
    else if ( operationSet.isCommutative() ) return true;
    else                                     return false;
  }
  
  Instruction instr = taskBody.at(loc);
  if (instr.oper == ALLOCA && instr.destination == operand) {
      return true;
  }

  if (instr.oper == BITCAST) {
    // destination has been casted from a different address
    if (instr.destination == operand) {
      return isSafe(taskBody, loc-1, instr.operand1);
    } else {
      return isSafe(taskBody, loc-1, operand);
    }
  }

  // used as parameter somewhere and might be a pointer
  if (instr.oper == CALL) {
    if (instr.raw.find(operand) != std::string::npos) {
      return false;
    } else {
      return isSafe(taskBody, loc-1, operand);
    }
  }

  // ADD or SUB or MUL or DIV
  if (instr.oper == ADD || instr.oper == SUB ||
      instr.oper == MUL || instr.oper == DIV) {
    if (instr.destination == operand) {

      // return immediately is operation can not
      // commute with previous operations
      if ( !operationSet.isCommutative(instr.oper ) ) {
        return false;
      }
      // append the commutative operation
      operationSet.appendOperation( instr.oper );

      bool t1 = isSafe(taskBody, loc-1, instr.operand1);
      bool t2 = isSafe(taskBody, loc-1, instr.operand2);
      return t1 && t2;
    } else {
      return isSafe(taskBody, loc-1, operand);
    }
  }

  // STORE
  if (instr.oper == STORE) {
    if (instr.destination == operand) {
      return isSafe(taskBody, loc-1, instr.operand1);
    }
    return isSafe(taskBody, loc-1, operand);
  }

  // load
  if (instr.oper == LOAD) {
    if (instr.destination == operand) {
      return isSafe(taskBody, loc-1, instr.operand1);
    }
  }

  return isSafe(taskBody, loc-1, operand);
  // GETEMEMENTSPTR
}
