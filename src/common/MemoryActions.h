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
//
// This class holds the first action to a memory location and the last
// write action. Every new write action replaces the previous write.

#ifndef _COMMON_MEMORYACTIONS_H_
#define _COMMON_MEMORYACTIONS_H_

#include "common/action.h"

class MemoryActions {
  public:
    bool isEmpty;

    Action action;

    int accessing_task_id;
    ADDRESS destination_address;

    // Default constructor
    MemoryActions() {
      isEmpty = true;
    }

    // Constructor which accepts an action
    MemoryActions(Action & act) {
      isEmpty = true;
      storeAction( act );
    }

    // Stores action if (a) is first action of task, or
    //                  (b) is last write action
    inline void storeAction(Action & act) {
       if ( isEmpty || act.is_write_action ) {
         action              = act;
         isEmpty             = false;
         accessing_task_id   = action.accessing_task_id;
         destination_address = action.destination_address;
       }
    }

    inline void storeAction(uint & taskID, ADDRESS & adr,
                  INTEGER & val, INTEGER & linNo,
                  INTEGER & funcID, bool is_write_action_) {
      if ( isEmpty || is_write_action_) {
        action.accessing_task_id   = taskID;
        action.destination_address = adr;
        action.source_func_id      = funcID;
        action.value_written       = val;
        action.source_line_num     = linNo;
        action.is_write_action     = is_write_action_;

        isEmpty             = false;
        accessing_task_id   = action.accessing_task_id;
        destination_address = action.destination_address;
      }
    }

    // Returns true if current action is a write
    bool hasWrite() {
      if ( isEmpty || ( !action.is_write_action )) {
        return false;
      } else {
        return true;
      }
    }

    void printActions(std::ostringstream & os) {
      if ( !isEmpty ) action.printAction( os );
    }
};

#endif // MemoryActions.h
