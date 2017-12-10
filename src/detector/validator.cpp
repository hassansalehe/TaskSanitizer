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

#ifndef _VALIDATOR_CPP_
#define _VALIDATOR_CPP_


// includes and definitions
#include "validator.h"
#include "conflictReport.h"

VOID BugValidator::parseTasksIR(char * IRlogName)
{
  vector<Instruction> * currentTask = NULL;
  string sttmt; // program statement
  ifstream IRcode(IRlogName); //  open IRlog file

  while( getline( IRcode, sttmt ) )
  {
    if( isEmpty(sttmt) ) continue; // skip empty line
    if( isDebugCall(sttmt) ) continue; // skip debug call

    sttmt = Instruction::trim( sttmt ); // trim spaces
    if ( isValid(sttmt) ) { // check if normal statement

      INTEGER lineNo = getLineNumber( sttmt );

      // skip instruction with line # 0: args to task body
      if (lineNo <= 0) continue;

      sttmt = sttmt.substr(sttmt.find_first_not_of(' ', sttmt.find_first_of(' ')));
      Instruction instr( sttmt );
      instr.lineNo = lineNo;
      currentTask->push_back(instr);
      continue;
    }

    if( isTaskName(sttmt) ) { // else a new task name
#ifdef DEBUG
      cout << "Task name: " << sttmt << endl;
#endif
      Tasks[sttmt] = vector<Instruction>();
      currentTask = &Tasks[sttmt];
      //map<string, map<INTEGER, vector<string>>> Tasks;
      continue;
    }

#ifdef DEBUG
    // got here :-(, wierd string!
    cout << "Unexpected program statement: " << sttmt << endl;
#endif
  }
  IRcode.close();
  cout << "Tasks no: " << Tasks.size() << endl;
}


/**
 * Checks for commutative task operations which have been
 * flagged as conflicts.
 */
VOID BugValidator::validate(Report & conflictSet) {

  // get task names
  string task1 = conflictSet.task1Name;
  string task2 = conflictSet.task2Name;

  // for each action pair
  auto  conflict  = conflictSet.buggyAccesses.begin();
  while(conflict != conflictSet.buggyAccesses.end()) {

     // skip commutativity check if read-write conflict
     if(conflict->action1.isWrite != conflict->action2.isWrite) {
       conflict++;
       continue;
     }
#ifdef DEBUG
     cout << task1 << " <--> "<< task2 << endl;
#endif
     INTEGER line1 = conflict->action1.lineNo;
     INTEGER line2 = conflict->action2.lineNo;
#ifdef DEBUG
     cout << "Lines " << line1 << " <--> " << line2 << endl;
#endif
     operationSet.clear(); // clear set of commuting operations

     // check if line1 operations commute
     if( involveSimpleOperations( task1, line1 ) &&
       // check if line2 operations commute
       involveSimpleOperations( task2, line2 )) {
       //it->second.erase(temPair);
       conflict = conflictSet.buggyAccesses.erase( conflict );
#ifdef DEBUG
       cout << "THERE IS SAFETY line1: " << line1 << " line2: " << line2 << endl;
#endif
     }
     else
        conflict++;
  }
}

BOOL BugValidator::involveSimpleOperations(string taskName, INTEGER lineNumber) {

  // get the instructions of a task
  vector<Instruction> &taskBody = Tasks[taskName];
  Instruction instr;
  INTEGER index = -1;

  for(auto i = taskBody.begin(); i != taskBody.end(); i++) {
     if(i->lineNo > lineNumber) break;
     if(i->lineNo == lineNumber) instr  = *i;
     index++;
  }
#ifdef DEBUG
  cout << "SAFET " << taskName << " " << instr.destination << ", idx: "<< index << endl;
#endif
  // expected to be a store
  if(instr.oper == STORE) {
    return isSafe(taskBody, index, instr.operand1);
    //bool r1 = isOnsimpleOperations(lineNumber - 1, istr.destination);
    //bool r2 = isOnsimpleOperations(lineNumber - 1, istr.operand1);
  }
  return false;
}

bool BugValidator::isSafe(const vector<Instruction> & taskBody, INTEGER loc, string operand) {

  if(loc < 0)
      return true;

  Instruction instr = taskBody.at(loc);

  if(instr.oper == ALLOCA && instr.destination == operand)
      return true;

  if( instr.oper == BITCAST ) {
    // destination has been casted from a different address
    if( instr.destination == operand)
      return isSafe(taskBody, loc-1, instr.operand1);
    else
      return isSafe(taskBody, loc-1, operand);
  }

  // used as parameter somewhere and might be a pointer
  if(instr.oper == CALL) {
    if(instr.raw.find(operand) != string::npos)
      return false;
    else
     return isSafe(taskBody, loc-1, operand);
  }

  // ADD or SUB or MUL or DIV
  if(instr.oper == ADD || instr.oper == SUB ||
    instr.oper == MUL || instr.oper == DIV) {
    if(instr.destination == operand) {

      // return immediately is operation can not
      // commute with previous operations
      if(! operationSet.isCommutative(instr.oper ) )
        return false;

      // append the commutative operation
      operationSet.appendOperation( instr.oper );

      bool t1 = isSafe(taskBody, loc-1, instr.operand1);
      bool t2 = isSafe(taskBody, loc-1, instr.operand2);
      return t1 && t2;
    }
    else
      return isSafe(taskBody, loc-1, operand);
  }

  // STORE
  if(instr.oper == STORE) {
    if(instr.destination == operand)
      return isSafe(taskBody, loc-1, instr.operand1);
    return isSafe(taskBody, loc-1, operand);
  }

  // load
  if(instr.oper == LOAD) {
    if(instr.destination == operand)
      return isSafe(taskBody, loc-1, instr.operand1);
  }

  return isSafe(taskBody, loc-1, operand);
  // GETEMEMENTSPTR
}

#endif // end validator.cpp
