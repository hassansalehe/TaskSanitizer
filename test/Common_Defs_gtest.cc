#include <gtest/gtest.h>

#include "common/defs.h"

//! \brief Test that OperRepresentation converts operations
//!        to approriate string representations.
TEST(OperRepresentation, TestConversion) {
  ASSERT_EQ("ADD", OperRepresentation(OPERATION::ADD));
  ASSERT_EQ("SUB", OperRepresentation(OPERATION::SUB));
  ASSERT_EQ("MUL", OperRepresentation(OPERATION::MUL));
  ASSERT_EQ("DIV", OperRepresentation(OPERATION::DIV));
  ASSERT_EQ("RET", OperRepresentation(OPERATION::RET));
  ASSERT_EQ("SHL", OperRepresentation(OPERATION::SHL));
  ASSERT_EQ("CALL", OperRepresentation(OPERATION::CALL));
  ASSERT_EQ("LOAD", OperRepresentation(OPERATION::LOAD));
  ASSERT_EQ("STORE", OperRepresentation(OPERATION::STORE));
  ASSERT_EQ("ALLOCA", OperRepresentation(OPERATION::ALLOCA));
  ASSERT_EQ("BITCAST", OperRepresentation(OPERATION::BITCAST));
  ASSERT_EQ("GETELEMENTPTR", OperRepresentation(OPERATION::GETELEMENTPTR));
  ASSERT_EQ("UNKNOWN", OperRepresentation(OPERATION(OPERATION::SHL + OPERATION::MUL)));
}

