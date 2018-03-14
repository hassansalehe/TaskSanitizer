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

// This module funds and remove duplicates from the list
// of nondeterminism errors found.
//
// Two warnings are duplicate if they report on the same memory
// location and source code line number.

#ifndef DETECTOR_DUPLICATE_MANAGER_H
#define DETECTOR_DUPLICATE_MANAGER_H

#include "checker.h"

class DuplicateManager {
private:
  /**
   * Removes the empty sets of errors of each memory location
   * After removing the duplicates.
   */
  static void removeEmptyEntries(
      std::map<std::pair<int, int>, Report> &conflictTable) {
    // a pair of conflicting task body with a set of line numbers
    for (auto it = conflictTable.begin(); it != conflictTable.end(); ) {
      Report & report = it->second;
      // validator.validate( report );
      if ( !report.buggyAccesses.size() ) {
         it = conflictTable.erase(it);
      } else {
        ++it;
      }
    } // end for
  }

public:
  static VOID removeDuplicates(
      std::map<std::pair<int, int>, Report> &conflictTable) {

    // generate simplified version of conflicts from scratch
    CONFLICT_PAIRS conflictTasksAndLines;
    conflictTasksAndLines.clear();

    // a pair of conflicting task body with a set of line numbers
    for (auto it = conflictTable.begin(); it != conflictTable.end();
        ++it) {
      std::pair<int, int> taskIdPair =
          std::make_pair(it->second.task1ID, it->second.task2ID);

      // erase duplicates
      for (auto conf = it->second.buggyAccesses.begin();
          conf != it->second.buggyAccesses.end(); ) {
        std::pair<int, int> lines =
            std::make_pair(conf->action1.lineNo, conf->action2.lineNo);
        auto inserted = conflictTasksAndLines[taskIdPair].insert(lines);
        if (inserted.second == false) {
          conf = it->second.buggyAccesses.erase( conf );
        } else {
          conf++;
        }
      } // end for
    }
    removeEmptyEntries(conflictTable);
  }
};

#endif // DETECTOR_DUPLICATE_MANAGER_H
