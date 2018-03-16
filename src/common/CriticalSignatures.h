#ifndef TASAN_CRITICALSIGNATURES_H
#define TASAN_CRITICALSIGNATURES_H

namespace tasan {

/**
 * Returns signature to mark beginning of a critical section
 */
inline std::string getStartCriticalSignature() {
  return "TASAN:BeginCriticalSection";
}

/*
 * Returns a string signature to mark end of a critical section
 */
inline std::string getEndCriticalSignature() {
  return "TASAN:EndCriticalSection";
}
} // end namespace

#endif
