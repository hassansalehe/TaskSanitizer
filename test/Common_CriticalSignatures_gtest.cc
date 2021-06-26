#include "common/CriticalSignatures.h"
#include <gtest/gtest.h>

//! \brief Unit tests for TaskSanitizer helper functions
TEST(CriticalSignature, CheckReturnOfGetStartCriticalSignature) {
  EXPECT_EQ("TASKSAN:BeginCriticalSection", tasksan::getStartCriticalSignature());
}

TEST(CriticalSignature, CheckReturnOfGetEndCriticalSignature) {
  EXPECT_EQ("TASKSAN:EndCriticalSection", tasksan::getEndCriticalSignature());
}
