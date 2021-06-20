#include <gtest/gtest.h>

#include <ostream>
#include <string>

#include "common/MemoryActions.h"

class TestMemoryActionsFixture : public ::testing::Test {
protected:
  INTEGER taskId = 123;
  ADDRESS addr = 0x033;
  VALUE value = 42;
  VALUE lineNo = 100;
  INTEGER funcId = 6;
  std::string funcName = "some_function";
  bool isWrite = true;

  Action m_act;

  virtual void SetUp() {
    m_act = Action(taskId, addr, value, lineNo, funcId);
  }
};

TEST_F(TestMemoryActionsFixture, CheckConstructorIsEmptyTrue) {
  MemoryActions m_actions;
  EXPECT_TRUE(m_actions.isEmpty);
}

TEST_F(TestMemoryActionsFixture, CheckConstructorWithActionArgument) {
  MemoryActions m_actions(m_act);
  EXPECT_FALSE(m_actions.isEmpty);
  EXPECT_EQ(taskId, m_actions.taskId);
  EXPECT_EQ(addr, m_actions.destination_address);
}

TEST_F(TestMemoryActionsFixture, CheckStoreActionFunction) {
  MemoryActions m_actions(m_act);
  EXPECT_FALSE(m_actions.isEmpty);
  EXPECT_EQ(taskId, m_actions.taskId);
  EXPECT_EQ(addr, m_actions.destination_address);
}

TEST_F(TestMemoryActionsFixture, CheckStoreActionFunctionWithParamsRead) {
  MemoryActions m_actions;
  EXPECT_TRUE(m_actions.isEmpty);

  // store
  uint ut = taskId;
  m_actions.storeAction(ut, addr, value, lineNo, funcId, false);

  EXPECT_EQ(taskId, m_actions.taskId);
  EXPECT_EQ(addr, m_actions.destination_address);
  EXPECT_FALSE(m_actions.isEmpty);

  EXPECT_EQ(taskId, m_actions.action.taskId);
  EXPECT_EQ(addr, m_actions.action.destination_address);
  EXPECT_EQ(funcId, m_actions.action.funcId);
  EXPECT_EQ(value, m_actions.action.value_written);
  EXPECT_EQ(lineNo, m_actions.action.source_line_num);
}

TEST_F(TestMemoryActionsFixture, CheckHasWrite) {
  // Empty action has no write
  MemoryActions m_actions;
  EXPECT_FALSE(m_actions.hasWrite());
  
  // Adding read action does not set acction to write 
  m_actions.storeAction(m_act);
  EXPECT_FALSE(m_actions.hasWrite());

  // Setting action to write
  m_actions.action.isWrite = true;
  EXPECT_TRUE(m_actions.hasWrite());

  // adding a write action
  m_actions.action.isWrite = false;
  m_act.isWrite = true;
  m_actions.storeAction(m_act);
  EXPECT_TRUE(m_actions.hasWrite());
}

TEST_F(TestMemoryActionsFixture, ChecPrintEmptyAction) {
  MemoryActions m_actions;
  std::ostringstream os;
  m_actions.printActions(os);
  EXPECT_EQ(0UL, os.str().size());
}

TEST_F(TestMemoryActionsFixture, ChecPrintExistingAction) {
  m_act.isWrite = true;
  MemoryActions m_actions(m_act);
  std::ostringstream os;
  m_actions.printActions(os);

  std::string expected_message = "123 W 0x33 42 100 6\n";
  EXPECT_EQ(expected_message, os.str());
}