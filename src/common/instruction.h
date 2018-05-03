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

// Defines the Instruction class.
// on a variable to determine if two operations commute.

#ifndef _COMMON_INSTRUCTION_H_
#define _COMMON_INSTRUCTION_H_

#include "common/defs.h"

class Instruction {
  public:
  INTEGER lineNo;
  std::string destination;
  std::string type;
  OPERATION oper;
  std::string operand1;
  std::string operand2;

  // raw representation of instruction
  std::string raw;

  /**
   * Default constructor
   */
  Instruction() {}
  /**
   * This constructor takes in IIR representation of an
   * instruction and constructs an object representaion of it. */
  Instruction(std::string stmt) {

    raw = trim( stmt );
    std::vector<std::string> contents = splitInstruction( raw );

    if (contents[0] == "store") {
      oper = STORE;
      destination = contents[4];
      operand1 = contents[2];
      operand2 = contents[2];
      type = contents[1];
    } else if (contents[2] == "load") {
      oper = LOAD;
      destination = contents[0];
      operand1 = contents[5];
      type = contents[3];
    } else if (std::regex_search(contents[2], std::regex("[fidb]add")) ||
               std::regex_search(contents[2], std::regex("[fidb]sub")) ||
               std::regex_search(contents[2], std::regex("[fidb]mul")) ||
               std::regex_search(contents[2], std::regex("[fidb]div")) ) {
      destination = contents[0];
      type = contents[3];
      operand1 = contents[4];
      operand2 = contents[5];
      if (std::regex_search(contents[2], std::regex("[fidb]add")))  oper = ADD;
      if (std::regex_search(contents[2], std::regex("[fidb]sub")))  oper = SUB;
      if (std::regex_search(contents[2], std::regex("[fidb]mul")))  oper = MUL;
      if (std::regex_search(contents[2], std::regex("[fidb]div")))  oper = DIV;
      // ...
    } else if (contents[2] == "add" || contents[2] == "sub" ||
             contents[2] == "mul" || contents[2] == "shl") {
      destination = contents[0];
      oper = (contents[2] == "add") ? ADD :
               (oper = (contents[2] == "sub") ? SUB :
                  (oper = (contents[2] == "mul") ? MUL : SHL));
      // <result> = add nuw nsw <ty> <op1>, <op2>  ; yields {ty}:result
      if (contents[3] == "nuw" && contents[4] == "nsw") {
        type = contents[5];
        operand1 = contents[6];
        operand2 = contents[7];
      } else if (contents[3] == "nuw" || contents[3] == "nsw") {
        // <result> = add nuw <ty> <op1>, <op2>      ; yields {ty}:result
        // <result> = add nsw <ty> <op1>, <op2>      ; yields {ty}:result
        type = contents[4];
        operand1 = contents[5];
        operand2 = contents[6];
      } else {
        // <result> = add <ty> <op1>, <op2>          ; yields {ty}:result
        type = contents[3];
        operand1 = contents[4];
        operand2 = contents[5];
      }
    } else if (contents[2] == "alloca") {
      destination = contents[0];
      oper = ALLOCA;
      type = contents[3];
    } else if (contents[2] == "bitcast") {
      destination = contents[0];
      oper = BITCAST;
      operand1 = contents[4];
      operand2 = contents[4];
    } else if (contents[0] == "call") {
      oper = CALL;
    }

  /*
  // find operation
  if (std::regex_search(segments[0], std::regex("store ")) {
     oper = STORE;
     std::string tmp = ;
     std::stringstream a(trim(segments[0]));
     tmp = ""'
     (getline(ss, tok, ','); //store
  }
  if (std::regex_search(segments[0], std::regex("load "))
     oper = LOAD;

  if (std::regex_search(segments[0], std::regex("call "))
     oper = CALL;

  if (std::regex_search(segments[0], std::regex("alloca "))
     oper = ALLOCA;

  if (std::regex_search(segments[0], std::regex("bitcast "))
     oper = BITCAST;

  if (std::regex_search(segments[0], std::regex("[fidb]add "))
     oper = ADD;

  if (std::regex_search(segments[0], std::regex("[fidb]mull "))
     oper = MULT;
  */
  }



  void print() {
    std::cout << "LineNo: " << lineNo
         << ", type: " << type
         << ", oper: " << OperRepresentation(oper)
         << ", dest: " << destination
         << ", op1: " << operand1
         << ", op2: " << operand2
    << std::endl;
  }

  /**
   * Trims the left and right spaces from a std::string. */
  static std::string trim(std::string sentence) {
    size_t start = sentence.find_first_not_of(' ');
    size_t end = sentence.find_last_not_of(' ');
    return sentence.substr(start, (end -start)+1);
  }

  /**
   * Splits std::string into tokens substrings */
  std::vector<std::string> splitInstruction(std::string stmt) {

    // split statements
    std::vector<std::string> segments;
    std::stringstream ss( stmt ); // make std::string stream.
    std::string tok;

    while ( getline(ss, tok, ',') ) {
      segments.push_back(tok);
    }

    std::vector<std::string> segments2;
    // make individual words
    for (auto stmt : segments) {
      std::stringstream sg2(trim(stmt));
      while ( getline(sg2, tok, ' ') ) {
        segments2.push_back(tok);
      }
    }
    return segments2;
  }
};

#endif // end instruction.h
