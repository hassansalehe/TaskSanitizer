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

// this file implements the checking tool functionalities.
#include <cassert>
#include "checker.h"  // header
#include "MemoryActions.h"

#define VERBOSE
#define CONC_THREASHOLD 5

// Saves the function name/signature for reporting nondeterminism
void Checker::registerFuncSignature(std::string funcName, int funcID) {
  assert(functions.find(funcID) == functions.end());
  functions[funcID] = funcName;
}

// Executed when a new task is created
void Checker::onTaskCreate(int taskID) {

  // we already know its parents
  // use this information to inherit or greate new serial bag
  auto parentTasks = graph[taskID].inEdges.begin();
  if (parentTasks == graph[taskID].inEdges.end()) { // if no HB tasks in graph

    // check if has no serial bag
    if (serial_bags.find(taskID) == serial_bags.end()) {
      auto newTaskbag = new SerialBag();
      if (graph.find(taskID) != graph.end()) {
        // specify number of tasks dependent of this task
        newTaskbag->outBufferCount = graph[taskID].outEdges.size();
      } else {//put it in the simple HB graph
          graph[taskID] = Task();
      }
      graph[taskID].taskID = taskID; // save the ID of the task
      serial_bags[taskID] = newTaskbag;
    }
  } else { // has parent tasks
    // look for the parents serial bags and inherit them
    // or construct your own by cloning the parent's

    // 1.find the parent bag which can be inherited
    SerialBagPtr taskBag = NULL;
    auto inEdge = graph[taskID].inEdges.begin();
/* Hassan 02.01.2018 modify this code to accommodate chunked tasks.
    for (; inEdge != graph[taskID].inEdges.end(); inEdge++) {

      // take with outstr 1 and longest
      auto curBag = serial_bags[*inEdge];
      if (curBag->outBufferCount == 1) {
        serial_bags.erase(*inEdge);
        graph[taskID].inEdges.erase(*inEdge);
        taskBag = curBag;
        curBag->HB.insert(*inEdge);
        break;  // could optimize by looking all bags
      }
    }
*/

    if (!taskBag) {
      taskBag = new SerialBag(); // no bag inherited
    }
    // the number of inheriting bags
    taskBag->outBufferCount = graph[taskID].outEdges.size();

    // 2. merge the HBs of the parent nodes
    inEdge = graph[taskID].inEdges.begin();
    for (; inEdge != graph[taskID].inEdges.end(); inEdge++) {
      auto aBag = serial_bags[*inEdge];
      taskBag->HB.insert(aBag->HB.begin(), aBag->HB.end()); // merging...
      taskBag->HB.insert(*inEdge); // parents happen-before me
/* Hassan 02.01.2018 modify this code to accommodate chunked tasks.
      aBag->outBufferCount--; // for inheriting bags
      if (!aBag->outBufferCount)
        serial_bags.erase(*inEdge);
*/
    }

    graph[taskID].taskID = taskID; // set the ID of the task
    serial_bags[taskID] = taskBag; // 3. add the bag to serial_bags
  }
}

// Saves a happens edge between predecessor and successor task in
// dependence edge
void Checker::saveHappensBeforeEdge(int parentId, int siblingId) {
  if ( graph.find(parentId) == graph.end() ) {
    graph[parentId] = Task();
  }
  if ( graph.find(siblingId) == graph.end() ) {
    graph[siblingId] = Task();
    graph[siblingId].taskID = siblingId;
  }

  graph[parentId].outEdges.insert(siblingId);
  graph[siblingId].inEdges.insert(parentId);
  Checker::onTaskCreate(siblingId);
}

// Detects output nondeterminism on a memory read or write
void Checker::detectNondeterminismOnMem(
    int taskID,
    std::string operation,
    std::stringstream & ssin) {

  Action action;
  action.taskId = taskID;
  constructMemoryAction(ssin, operation, action);

  if (action.funcId == 0) {
    std::cout << "Warning function Id 0: " << std::endl;
    exit(0);
  }
  MemoryActions memActions( action ); // save first action

  if ( !ssin.eof() ) { // there are still tokens
    std::string separator;
    ssin >> separator;
    ssin >> taskID;
    Action lastWAction;
    lastWAction.taskId = taskID;
    ssin >> operation;
    constructMemoryAction(ssin, operation, lastWAction);
    memActions.storeAction( lastWAction ); // save second action
  }
  saveTaskActions( memActions ); // save the actions
}

void Checker::saveTaskActions( const MemoryActions & taskActions ) {

  // CASES
  // 1. first action -> just save
  // 2. nth write of the same task -> just save
  // 3. a previous write in a happens-before -> just save
  // 4. previous write is parallel:
  //    4.1 but same value -> append new write
  //    4.2 otherwise report a race and look for a happens-before
  //        write in the parallel writes,update and take it forward
  //        4.2.1 check conflicts with other parallel tasks

  auto perAddrActions = writes.find( taskActions.addr );
  if (perAddrActions == writes.end()) {// 1. first action
     writes[taskActions.addr] = std::list<MemoryActions>();
  }
  std::list<MemoryActions> & AddrActions = writes[taskActions.addr];
  for (auto lastWrt = AddrActions.begin();
       lastWrt != AddrActions.end(); lastWrt++) {
    // actions of same task
    if (taskActions.taskId == lastWrt->taskId) continue;

    auto HBfound = serial_bags[taskActions.taskId]->HB.find(lastWrt->taskId);
    auto end     = serial_bags[taskActions.taskId]->HB.end();
    if (HBfound != end) continue; // 3. there's happens-before

    // 4. parallel, possible race! ((check race))

    // check write-write case (different values written)
    // 4.1 both write to shared memory
    if ( (taskActions.action.isWrite && lastWrt->action.isWrite) &&
         (taskActions.action.value != lastWrt->action.value) ) {
      // write different values, code for recording errors
      saveNondeterminismReport( taskActions.action, lastWrt->action );
    } else if ((!taskActions.action.isWrite) && lastWrt->action.isWrite) {
    // 4.2 read-after-write or write-after-read conflicts
    // (a) taskActions is read-only and lastWrt is a writer:
      // code for recording errors
      saveNondeterminismReport(taskActions.action, lastWrt->action);
    } else if ((!lastWrt->action.isWrite) && taskActions.action.isWrite ) {
    // (b) lastWrt is read-only and taskActions is a writer:
      // code for recording errors
      saveNondeterminismReport(taskActions.action, lastWrt->action);
    }
  } // end for

  if (AddrActions.size() >= CONC_THREASHOLD) {
    AddrActions.pop_front(); // remove oldest element
  }

  writes[taskActions.addr].push_back( taskActions ); // save
}


/**
 * Records the nondeterminism warning to the conflicts table.
 * This is per pair of concurrent tasks.
 */
VOID Checker::saveNondeterminismReport(const Action& curMemAction,
                                       const Action& prevMemAction) {
  Conflict aConflict(curMemAction, prevMemAction);

  // store only if conflict is not commutative
  if ( !commutativeChecker.isCommutative(aConflict) ) {

    // code for recording errors
    std::pair<int, int> linePair =
        {
          std::min(curMemAction.lineNo, prevMemAction.lineNo),
          std::max(curMemAction.lineNo, prevMemAction.lineNo)
        };
    conflictTable[linePair].insert( aConflict );
  }
}


// Adds a new task node in the simple happens-before graph
// @params: logLine, a log entry the contains the task ids
void Checker::addTaskNode(std::string & logLine) {
    std::stringstream ssin(logLine);
    int sibId;
    int parId;

    ssin >> sibId;
    ssin >> parId;
    //std::cout << line << "(" << sibId << " " << parId << ")" << std::endl;
    Checker::saveHappensBeforeEdge(parId, sibId);
}

/** Constructs action object from the log file */
void Checker::constructMemoryAction(std::stringstream & ssin,
                                    std::string & operation,
                                    Action & action) {
    std::string tempBuff;
    ssin >> tempBuff; // address
    action.addr = (ADDRESS)stoul(tempBuff, 0, 16);

    ssin >> tempBuff; // value
    action.value = stol(tempBuff);

    ssin >> tempBuff; // line number
    action.lineNo = stol(tempBuff);

    ssin >> action.funcId; // get function id

    if (operation == "W") {
      action.isWrite = true;
    } else {
      action.isWrite = false;
    }
#ifdef DEBUG // check if data correctly set.
    std::cout << "Action constructed: ";
    std::ostringstream buff;
    action.printAction(buff);
    std::cout << buff.str();
    std::cout << std::endl;
#endif
}

void Checker::checkCommutativeOperations(CommutativityChecker & validator) {
  // a pair of conflicting task body with a set of line numbers
  for (auto it = conflictTable.begin(); it != conflictTable.end(); ) {
    for ( auto aConflict = it->second.begin();
        aConflict != it->second.end(); ) {
      if ( validator.isCommutative( *aConflict ) ) {
        aConflict = it->second.erase(aConflict);
        if ( 0 == it->second.size() ) {
           it = conflictTable.erase(it);
        } else {
          ++it;
        }
      } else {
        ++aConflict;
      }
    }
  }
}


VOID Checker::reportConflicts() {
  const std::string emptyLine(
       "                                                            ");
  const std::string borderLine(
      "============================================================");
  std::cout << borderLine                              << std::endl;
  std::cout << emptyLine                               << std::endl;
  std::cout << "                    Summary  "         << std::endl;
  std::cout << emptyLine                               << std::endl;
  std::cout << " Total number of tasks: " <<  graph.size() << std::endl;
  std::cout << emptyLine                               << std::endl;
  std::cout << emptyLine                               << std::endl;
  std::cout << emptyLine                               << std::endl;
  std::cout << "              Non-determinism checking report  " << std::endl;
  std::cout << emptyLine                               << std::endl;

  // print appropriate message in case no errors found
  if (! conflictTable.size() ) {
    std::cout << "                 No nondeterminism found! " << std::endl;
  } else {
    std::cout << " The following " << conflictTable.size()
              << " task pairs have conflicts: " << std::endl;
  }

  for (auto it : conflictTable) {
    std::cout << "    " << it.first.first << " ("
              << it.first.first <<")  <--> "
              << it.first.second << " (" << it.first.second << ")"
              << " on " << it.second.size()
              << " memory addresses"                    << std::endl;

    if (it.second.size() > 10) {
      // we want to print at most 10 addresses if they are too many.
      std::cout << "    showing at most 10 addresses: " << std::endl;
    }
    int addressCount = 0;

    for (auto aConflict : it.second) {
      std::cout << "      " <<  aConflict.addr << " lines: " << " "
                << functions.at( aConflict.action1.funcId )
                << ": "     << aConflict.action1.lineNo
                << ", "     << functions.at( aConflict.action2.funcId )
                << ": "     << aConflict.action2.lineNo << std::endl;
      addressCount++;

      if (addressCount == 10) break;
    } // end for
  }

  std::cout << emptyLine     << std::endl;
  std::cout << borderLine    << std::endl;
}

VOID Checker::testing() {
  for (auto it = writes.begin(); it != writes.end(); it++) {
     std::cout << it->first << ": Bucket {" << it->second.size();
     std::cout <<"} "<< std::endl;
  }
  std::cout << "Total Addresses: " << writes.size() << std::endl;

  // testing
  std::cout << "====================" << std::endl;
  for (auto it = serial_bags.begin(); it != serial_bags.end(); it++) {
      std::cout << it->first << " ("<< it->second->outBufferCount<< "): {";
      for (auto x = it->second->HB.begin();
           x != it->second->HB.end(); x++) {
        std::cout << *x << " ";
      }
      std::cout << "}" << std::endl;
  }
}


/**
 * implementation of the checker destructor frees
 * the memory dynamically generated for S-bags */
Checker::~Checker() {
  for (auto it = serial_bags.begin(); it != serial_bags.end(); it++) {
    delete it->second;
  }
}
