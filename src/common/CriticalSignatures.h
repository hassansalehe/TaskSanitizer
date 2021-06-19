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

#ifndef _COMMON_CRITICALSIGNATURES_H_
#define _COMMON_CRITICALSIGNATURES_H_

#include <string>

namespace tasksan {

// Returns signature to mark beginning of a critical section
inline std::string getStartCriticalSignature() {
  return "TASKSAN:BeginCriticalSection";
}

// Returns a string signature to mark end of a critical section
inline std::string getEndCriticalSignature() {
  return "TASKSAN:EndCriticalSection";
}
} // end namespace

#endif
