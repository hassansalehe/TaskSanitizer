#include <gtest/gtest.h>

#include "common/instruction.h"

TEST(InstructionTests, CheckTrimHelperFunction) {
  std::string raw_instruction(" store balance r6 ");
  EXPECT_EQ(std::string("store balance r6"), Instruction::trim(raw_instruction));

  EXPECT_TRUE(Instruction::trim("      ").empty());
}

TEST(InstructionTests, CheckSplitInstructionFunction) {
  std::string raw_instruction(" store balance r6 ");
  Instruction instr;
  auto chunks = instr.splitInstruction(raw_instruction);

  EXPECT_EQ(3UL, chunks.size());
  EXPECT_EQ("store", chunks.at(0));
  EXPECT_EQ("balance", chunks.at(1));
  EXPECT_EQ("r6", chunks.at(2));
}

TEST(InstructionTests, CheckSplitInstructionFunctionEmptyString) {
  std::string raw_instruction("   ");
  Instruction instr;
  auto chunks = instr.splitInstruction(raw_instruction);
  EXPECT_EQ(0UL, chunks.size());
}

TEST(InstructionTests, CheckInstructionForStore) {
  std::string raw_instruction(" store i32 %2, i32* %balance ");
  Instruction instr(raw_instruction);
  EXPECT_EQ(STORE, instr.oper);
  EXPECT_EQ("%balance", instr.destination);
  EXPECT_EQ("i32", instr.type);
  EXPECT_EQ("%2", instr.operand1);
  EXPECT_EQ("%2", instr.operand2);
}

TEST(InstructionTests, CheckInstructionForLoad) {
  std::string raw_instruction(" %1 = load i32 * %balance");
  Instruction instr(raw_instruction);
  EXPECT_EQ(LOAD, instr.oper);
  EXPECT_EQ("%1", instr.destination);
  EXPECT_EQ("i32", instr.type);
  EXPECT_EQ("%balance", instr.operand1);
}
