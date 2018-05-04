#ifndef _COMMON_CRITICALSIGNATURES_H_
#define _COMMON_CRITICALSIGNATURES_H_

namespace tasksan {

/**
 * Returns signature to mark beginning of a critical section
 */
inline std::string getStartCriticalSignature() {
  return "TASKSAN:BeginCriticalSection";
}

/*
 * Returns a string signature to mark end of a critical section
 */
inline std::string getEndCriticalSignature() {
  return "TASKSAN:EndCriticalSection";
}
} // end namespace

#endif
