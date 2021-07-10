/////////////////////////////////////////////////////////////////
//  TaskSanitizer: a lightweight determinacy race checking
//          tool for OpenMP task applications
//
//    Copyright (c) 2015 - 2021 Hassan Salehe Matar
//      Copying or using this code by any means whatsoever
//      without consent of the owner is strictly prohibited.
//
//   Contact: hassansalehe-at-gmail-dot-com
//
/////////////////////////////////////////////////////////////////

// this files redifines data types for redability and
// WORE (Write-Once-Reuse-Everywhere). Defines the Checker class

#ifndef _DETECTOR_DETERMINACY_CHECKER_H_
#define _DETECTOR_DETERMINACY_CHECKER_H_

// includes and definitions
#include "common/defs.h"
#include "common/MemoryActions.h"
#include "detector/determinacy/conflict.h"
#include "detector/determinacy/report.h"
#include "detector/commutativity/CommutativityChecker.h"
#include <list>

// a bag to hold the tasks that happened-before
typedef struct SerialBag {
  int outBufferCount;
  UNORD_INTSET HB;  // unordered int set

  SerialBag(): outBufferCount(0){}
} SerialBag;

// for constructing happans-before between tasks
typedef struct Task {
  int taskID;     // identity of the task
  UNORD_INTSET inEdges;  // incoming data streams
  UNORD_INTSET outEdges; // outgoing data streams
} Task;

typedef SerialBag * SerialBagPtr;

class Checker {
  public:
  VOID addTaskNode(std::string & logLine);
  VOID saveTaskActions(const MemoryActions & taskActions);

  // a pair of conflicting task body with a set of line numbers
  VOID checkCommutativeOperations(CommutativityChecker & validator);

  VOID registerFuncSignature(std::string funcName, int funcID);
  VOID onTaskCreate(int taskID);
  VOID saveHappensBeforeEdge(int parentId, int siblingId);
  VOID detectRaceOnMem(int taskID,
                                 std::string operation,
                                 std::stringstream & ssin);

  std::map<std::pair<int, int>, std::set<Conflict>> & getConflicts() {
    return conflictTable;
  }

  VOID initializeCommutativityChecker(char *fileName) {
    commutativeChecker.parseTasksIR(fileName);
  }
  VOID reportConflicts();
  VOID testing();
  ~Checker();

  private:
    // Constructs action object from the log file
    VOID constructMemoryAction(std::stringstream & ssin,
                               std::string & opType,
                               Action & action);
    VOID saveDeterminacyRaceReport(const Action& curWrite,
                                  const Action& write);

    // hold bags of tasks
    std::unordered_map <INTEGER, SerialBagPtr> serial_bags;
    std::unordered_map<INTEGER, Task> graph;  // in and out edges
    // for write actions
    std::unordered_map<ADDRESS, std::list<MemoryActions>> writes;
    std::map<std::pair<int, int>, std::set<Conflict>> conflictTable;
    CONFLICT_PAIRS conflictTasksAndLines;

    // For holding function signatures.
    std::unordered_map<INTEGER, std::string> functions;

    // the commutativity checker
   CommutativityChecker commutativeChecker;
};

#endif // end checker.h
