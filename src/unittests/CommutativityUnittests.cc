#include <CriticalSections.h>
#include <CriticalSectionBody.h>
#include <cassert>
#include <iostream>
#include <vector>
//#include <>

int main() {
  tasan::commute::CriticalSections cs;

  // first body
  Instruction instr1;
  instr1.lineNo = 3;
  Instruction instr2;
  instr2.lineNo = 6;
  std::vector<Instruction> body = {instr1, instr2};
  tasan::commute::CriticalSectionBody body1(body);
  body1.setStartLineNo(3);
  body1.setEndLineNo(6);
  cs.insert(body1);

  assert(cs.find(2) == nullptr);
  assert(cs.find(7) == nullptr);
  assert(cs.find(3) != nullptr);
  assert(cs.find(6) != nullptr);
  std::cout << cs.find(3)->to_string() << std::endl;

  // second body
  body1.setStartLineNo(3);
  body1.setEndLineNo(4);
  cs.insert(body1);
  assert(cs.getSize() == 1);

  // third body
  body1.setStartLineNo(113);
  body1.setEndLineNo(411);
  cs.insert(body1);
  assert(cs.getSize() == 2);

  assert(cs.find(500) == nullptr);
  assert(cs.find(399) != nullptr);
  std::cout << cs.find(411)->to_string() << std::endl;

  return 0;
}
