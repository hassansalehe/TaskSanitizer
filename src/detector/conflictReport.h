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

//
//

#ifndef _CONFLICT_HPP_
#define _CONFLICT_HPP_

// includes and definitions
#include "defs.h"

#include "action.h" // defines Action class

using namespace std;

// This struct keeps the line information of the
// address with determinism conflict
class Conflict {
 public:
  ADDRESS addr;

  Action action1;
  Action action2;

  Conflict(const Action& curMemAction, const Action& prevMemAction) {

    action1 = curMemAction;
    action2 = prevMemAction;
    addr = curMemAction.addr;
  }

  bool operator<(const Conflict &RHS) const {
    return addr < RHS.addr || action1.taskId < action2.taskId;
  }
}; // end Conflict


// For storing names of conflicting tasks
// and the addresses they conflict at
class Report {
 public:
  string task1Name;
  string  task2Name;
  set<Conflict> buggyAccesses;
}; // end Report

#endif // end conflictReport.h
