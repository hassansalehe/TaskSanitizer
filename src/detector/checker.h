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

// this files redifines data types for redability and WORE (Write-Once-Reuse-Everywhere)
// Defines the ADF Checker class

#ifndef _CHECKER_HPP_
#define _CHECKER_HPP_

// includes and definitions
#include "defs.h"
#include "action.h" // defines Action class
#include "conflictReport.h" // defines Conflict and Report structs
#include "sigManager.h" // for managing function names
#include "MemoryActions.h"
#include "validator.h"
#include <list>

using namespace std;

// a bag to hold the tasks that happened-before
typedef struct SerialBag {
  int outBufferCount;
  UNORD_INTSET HB;  // unordered int set

  SerialBag(): outBufferCount(0){}
} SerialBag;

// for constructing happans-before between tasks
typedef struct Task {
  string name;     // name of the task
  UNORD_INTSET inEdges;  // incoming data streams
  UNORD_INTSET outEdges; // outgoing data streams
} Task;

typedef SerialBag * SerialBagPtr;

class Checker {
  public:
  VOID addTaskNode(string & logLine);
  VOID saveTaskActions( const MemoryActions & taskActions );
  VOID processLogLines(string & line);

  // a pair of conflicting task body with a set of line numbers
  VOID checkCommutativeOperations( BugValidator & validator );

  VOID reportConflicts();
  VOID printHBGraph();
  VOID printHBGraphJS();  // for printing dependency graph in JS format
  VOID testing();
  ~Checker();

  private:
    /** Constructs action object from the log file */
    VOID constructMemoryAction(stringstream & ssin, string & opType, Action & action);

    VOID saveNondeterminismReport(const Action& curWrite, const Action& write);

    unordered_map <INTEGER, SerialBagPtr> serial_bags; // hold bags of tasks
    unordered_map<INTEGER, Task> graph;  // in and out edges
    unordered_map<ADDRESS, list<MemoryActions>> writes; // for writes
    map<pair<STRING, STRING>, Report> conflictTable;
    CONFLICT_PAIRS conflictTasksAndLines;

    // For holding function signatures.
    SigManager signatureManager;
};

#endif // end checker.h
