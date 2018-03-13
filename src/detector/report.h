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

#ifndef _CONFLICT_HPP_
#define _CONFLICT_HPP_

// includes and definitions
#include "defs.h"
#include "action.h" // defines Action class

// For storing names of conflicting tasks
// and the addresses they conflict at
class Report {
 public:
  int task1ID;
  int task2ID;
  std::set<Conflict> buggyAccesses;
}; // end Report

#endif // end conflictReport.h
