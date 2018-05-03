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

// Defines a class to hold task identification info

#ifndef _INSTRUMENTOR_EVENTLOGGER_H_
#define _INSTRUMENTOR_EVENTLOGGER_H_

#include "common/defs.h"
#include "common/MemoryActions.h"

typedef struct TaskInfo {
  uint threadID = 0;
  uint taskID   = 0;
  bool active   = false;

  // stores pointers of signatures of functions executed by task
  // for faster acces
  std::unordered_map<STRING, INTEGER> functions;

  // stores memory actions performed by task.
  std::unordered_map<address, MemoryActions> memoryLocations;

  // improve performance by buffering actions and write only once.
  std::ostringstream actionBuffer;

  // stores the IDs of child tasks created by this task
  std::vector<int> childrenIDs;

  /**
   * Appends child ID to a list of children IDs */
  inline void addChild(int childID) {
    childrenIDs.push_back(childID);
  }

  /**
   * Stores the action info as performed by task. The rules for
   * storing this information are explained in MemoryActions.h */
  inline void saveMemoryAction(Action & action) {
    MemoryActions &loc = memoryLocations[action.addr];
    loc.storeAction( action );
  }

  inline void saveReadAction(ADDRESS & addr,
      INTEGER & lineNo,
      const INTEGER funcID) {

    MemoryActions & loc = memoryLocations[addr];
    if ( !loc.hasWrite() ) {
      Action action( taskID, addr, 0, lineNo, funcID );
      action.isWrite = false;
      loc.storeAction( action );
    }
  }

  inline void saveWriteAction(
      ADDRESS addr,
      INTEGER value,
      INTEGER lineNo,
      INTEGER funcID) {

     MemoryActions & loc = memoryLocations[addr];
     bool isWrite = true;
     loc.storeAction(taskID, addr, value, lineNo, funcID, isWrite);
  }

  /**
   * Prints to ostringstream all memory access actions recorded. */
  void printMemoryActions() {
    for (auto it = memoryLocations.begin();
         it != memoryLocations.end(); ++it) {
      it->second.printActions( actionBuffer );
    }
  }

  /** HELPER FUNCTIONS */

  /**
   * returns ID if function registered before, otherwise 0. */
   inline INTEGER getFunctionId( const STRING funcName ) {
     auto fd = functions.find( funcName );
     if ( fd == functions.end() ) {
       return 0;
     } else {
       return fd->second;
     }
   }

   /**
    * Registers function for faster access. */
   void registerFunction(STRING funcName, INTEGER funcId ) {
     functions[funcName] = funcId;
   }

   /**
    * Clears all stored memory actions.
    * Can executed once the actions are written to log file. */
   void flushLogs() {
     memoryLocations.clear();
     actionBuffer.str(""); // clear buffer
   }

} TaskInfo;

// holder of task identification information
static std::unordered_map<uint, TaskInfo> taskInfos;

#endif // TaskInfo.h
