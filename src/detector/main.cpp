/////////////////////////////////////////////////////////////////
//  ADFinspec: a lightweight non-determinism checking
//          tool for ADF applications
//
//    Copyright (c) 2015 - 2018 Hassan Salehe Matar & MSRC at Koc University
//      Copying or using this code by any means whatsoever
//      without consent of the owner is strictly prohibited.
//
//   Contact: hmatar-at-ku-dot-edu-dot-tr
//
/////////////////////////////////////////////////////////////////

// implements the main function.

#include "checker.h"  // header
#include "validator.h"

void processLogLines(Checker & aChecker, string & line) {

  stringstream ssin(line); // split string

  int taskID;
  string operation;

  ssin >> taskID; // get task id
  ssin >> operation; // get operation

  if(operation.find("W") != string::npos || // write action, or
     operation.find("R") != string::npos) { // read action
     aChecker.detectNondeterminismOnMem(taskID, operation, ssin);
  }
  // Check if this is just function name
  else if(operation.find("F") != string::npos) {

    // task id position is func ID in this case
    int funcID = taskID;
    string funcName;
    getline(ssin, funcName); // get function name
    aChecker.registerFuncSignature(funcName, funcID);
  }
  // if new task creation, parents terminated
  else if(operation.find("B") != string::npos) {
    aChecker.onTaskCreate(taskID);
  }
}

int main(int argc, char * argv[])
{

  if(argc != 4) {
    cout << endl;
    cout << "ERROR!" << endl;
    cout << "Usage: ./ADDFinspec TraceLog.txt HBlog.txt IRlog.txt" << endl;
    cout << endl;
    exit(-1);
  }

  // take time at begin of analysis
  std::chrono::high_resolution_clock::time_point t1 =
      std::chrono::high_resolution_clock::now();

  Checker aChecker; // checker instance
  string logLine;

  ifstream HBlog(argv[2]); //  hb file
  // check if HBlog file successfully opened
  if(!HBlog.is_open()) {
    cout << "ERROR!" << endl;
    cout << "HBlog file: " << argv[2] << " could not open." << endl;
    exit(-1);
  }

  while( getline(HBlog, logLine) )
    aChecker.addTaskNode(logLine);
  HBlog.close();

  ifstream log(argv[1]); //  log file
  // check if trace file successfully opened
  if(!log.is_open()) {
    cout << "ERROR!" << endl;
    cout << "Trace file: " << argv[1] << " could not open." << endl;
    exit(-1);
  }

  while( getline(log, logLine) )
    // processes log file and detects nondeterminism
    processLogLines(aChecker, logLine);
  log.close();

  // validate the detected nondeterminism bugs
  BugValidator validator;
  validator.parseTasksIR( argv[3] ); // read IR file

  // do the validation to eliminate commutative operations
  aChecker.checkCommutativeOperations( validator );

  // take time at end of analyis
  std::chrono::high_resolution_clock::time_point t2 =
      std::chrono::high_resolution_clock::now();
  auto duration = std::chrono::duration_cast
      <std::chrono::microseconds>( t2 - t1 ).count();

  // testing writes
  //aChecker.testing();
  aChecker.reportConflicts();
  aChecker.printHBGraph();
  aChecker.printHBGraphJS(); // print in JS format

  cout << "Checker execution time: "<< duration/1000.0 << " milliseconds" << endl;
  return 0;
}
