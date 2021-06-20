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

#ifndef _DETECTOR_DETERMINACY_CONFLICT_H_
#define _DETECTOR_DETERMINACY_CONFLICT_H_

// includes and definitions
#include "common/defs.h"
#include "common/action.h" // defines Action class

// This struct keeps the line information of the
// address with determinacy race conflict
class Conflict {
 public:
  ADDRESS addr;

  Action action1;
  Action action2;

  Conflict(const Action& curMemAction, const Action& prevMemAction) {
    action1 = curMemAction;
    action2 = prevMemAction;
    addr    = curMemAction.destination_address;
  }

  inline int getTask1Id() {
    return action1.accessing_task_id;
  }

  inline int getTask2Id() {
    return action2.accessing_task_id;
  }

  bool operator<(const Conflict &RHS) const {
    return addr < RHS.addr;
  }
}; // end Conflict

#endif // end conflict.h
