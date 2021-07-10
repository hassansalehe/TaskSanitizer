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

// Defines the OperationSet class which keeps a sequence of
// operations on a variable to determine if the operations commute.

#ifndef _DETECTOR_DETERMINACY_OPERATIONSET_H_
#define _DETECTOR_DETERMINACY_OPERATIONSET_H_

// includes and definitions
#include "common/defs.h"

class OperationSet {
  public:
    void clear() { operations.clear(); }

    void appendOperation(OPERATION op) {
      operations.insert( op );
    }

    // Checks if operation "op" commutes with previous
    // operations which manipulate a shared memory location
    bool isCommutative(const OPERATION op) {
     // compare with other operation
     for (auto i : operations) {
      switch( op )
      {
        case ADD:
        case SUB:
          if (i != ADD && i != SUB)  return false;
          break;
        case MUL:
        case DIV:
          if (i != MUL && i != DIV)  return false;
          break;
        default:
          return false;
        }
      } // end for

      // compatible with all operations in the set
      return true;
    }

    bool isCommutative() {
      for (auto op : operations) {
         if ( !isCommutative(op) ) return false;
      }
      return true;
    }

    int size() {
      return operations.size();
    }

  private:
    std::set<OPERATION> operations;
};

#endif // end operationSet.h
