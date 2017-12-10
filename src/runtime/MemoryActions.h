/////////////////////////////////////////////////////////////////
//  Finspec: a lightweight non-determinism checking
//          tool for shared memory Dataflow applications
//
//    Copyright (c) 2015 - 2017 Hassan Salehe Matar & MSRC at Koc University
//      Copying or using this code by any means whatsoever
//      without consent of the owner is strictly prohibited.
//
//   Contact: hmatar-at-ku-dot-edu-dot-tr
//
/////////////////////////////////////////////////////////////////
//
// This class holds the first action to a memory memory
// location and the last write action.
// Every new write action replaces the previous write.

#ifndef MEMORY_ACTIONS_H
#define MEMORY_ACTIONS_H

#include "action.h"

class MemoryActions {
  public:
    bool isEmpty;

    Action action;

    int taskId;
    ADDRESS addr;     // destination address

    // Default constructor
    MemoryActions() {
      isEmpty = true;
    }

    // Constructor which accepts an action
    MemoryActions(Action & act) {

      isEmpty = true;
      storeAction( act );
    }

    // Stores action if
    // (a) is first action of task, or
    // (b) is last write action
    inline void storeAction(Action & act) {
       if ( isEmpty || act.isWrite ) {
         action = act;
         isEmpty = false;
         taskId = action.taskId;
         addr = action.addr;
       }
    }

    inline void storeAction(uint & taskID, ADDRESS & adr,
                  INTEGER & val, INTEGER & linNo,
                  INTEGER & funcID, bool isWrite_) {

         if(isEmpty || isWrite_) {
           action.taskId = taskID;
           action.addr = adr;
           action.funcId = funcID;
           action.value = val;
           action.lineNo = linNo;
           action.isWrite = isWrite_;
           isEmpty = false;
         }
    }

    /**
     * Returns true if current action is a write
     */
    bool hasWrite() {
      if( isEmpty || ( !action.isWrite ))
       return false;

      return true;
    }

    void printActions(ostringstream & os) {
      if( !isEmpty )
        action.printAction( os );
    }
};

#endif // MemoryActions.h
