/////////////////////////////////////////////////////////////////
//  ADFinspec: a lightweight non-determinism checking
//          tool for ADF applications
//
//    Copyright (c) 2015 - 2017 Hassan Salehe Matar & MSRC at Koc University
//      Copying or using this code by any means whatsoever
//      without consent of the owner is strictly prohibited.
//
//   Contact: hmatar-at-ku-dot-edu-dot-tr
//
/////////////////////////////////////////////////////////////////

// Defines the Instruction class.
// on a variable to determine if the operations commute with each other.

#ifndef _INSTRUCTION_HPP_
#define _INSTRUCTION_HPP_

#include "defs.h"

class Instruction {
  public:
  INTEGER lineNo;
  string destination;
  string type;
  OPERATION oper;
  string operand1;
  string operand2;

  // raw representation of instruction
  string raw;

  /**
   * Default constructor
   */
  Instruction() {}
  /**
   * This constructor takes in IIR representation of
   * an instruction and constructs an object representaion
   * of it.
   */
  Instruction(string stmt) {

  raw = trim( stmt );
  vector<string> contents = splitInstruction( raw );

  if(contents[0] == "store") {
    oper = STORE;
    destination = contents[4];
    operand1 = contents[2];
    operand2 = contents[2];
    type = contents[1];
  }
  else if(contents[2] == "load") {
    oper = LOAD;
    destination = contents[0];
    operand1 = contents[5];
    type = contents[3];
  }
  else if (regex_search(contents[2], regex("[fidb]add")) ||
           regex_search(contents[2], regex("[fidb]sub")) ||
           regex_search(contents[2], regex("[fidb]mul")) ||
           regex_search(contents[2], regex("[fidb]div")) )
  {
    destination = contents[0];
    type = contents[3];
    operand1 = contents[4];
    operand2 = contents[5];
    if (regex_search(contents[2], regex("[fidb]add")))
        oper = ADD;
    if (regex_search(contents[2], regex("[fidb]sub")))
        oper = SUB;
    if (regex_search(contents[2], regex("[fidb]mul")))
        oper = MUL;
    if (regex_search(contents[2], regex("[fidb]div")))
        oper = DIV;
    // ...

  }
  else if(contents[2] == "add" || contents[2] == "sub" ||
          contents[2] == "mul" || contents[2] == "shl")
  {
     destination = contents[0];
     oper = contents[2] == "add" ? ADD :
     (oper = contents[2] == "sub"? SUB :
     (oper = contents[2] == "mul"? MUL : SHL));
     // <result> = add nuw nsw <ty> <op1>, <op2>  ; yields {ty}:result
     if(contents[3] == "nuw" && contents[4] == "nsw") {
        type = contents[5];
        operand1 = contents[6];
        operand2 = contents[7];
     }
     else if(contents[3] == "nuw" || contents[3] == "nsw") {
        // <result> = add nuw <ty> <op1>, <op2>      ; yields {ty}:result
        // <result> = add nsw <ty> <op1>, <op2>      ; yields {ty}:result
        type = contents[4];
        operand1 = contents[5];
        operand2 = contents[6];
     }
     else {
        // <result> = add <ty> <op1>, <op2>          ; yields {ty}:result
        type = contents[3];
        operand1 = contents[4];
        operand2 = contents[5];
     }
  }
  else if(contents[2] == "alloca") {
    destination = contents[0];
    oper = ALLOCA;
    type = contents[3];
  }
  else if(contents[2] == "bitcast") {
    destination = contents[0];
    oper = BITCAST;
    operand1 = contents[4];
    operand2 = contents[4];
  }
  else if(contents[0] == "call") {
    oper = CALL;
  }

  /*
  // find operation
  if(regex_search(segments[0], regex("store ")) {
     oper = STORE;
     string tmp = ;
     stringstream a(trim(segments[0]));
     tmp = ""'
     (getline(ss, tok, ','); //store
  }
  if(regex_search(segments[0], regex("load "))
     oper = LOAD;

  if(regex_search(segments[0], regex("call "))
     oper = CALL;

  if(regex_search(segments[0], regex("alloca "))
     oper = ALLOCA;

  if(regex_search(segments[0], regex("bitcast "))
     oper = BITCAST;

  if(regex_search(segments[0], regex("[fidb]add "))
     oper = ADD;

  if(regex_search(segments[0], regex("[fidb]mull "))
     oper = MULT;
  */
}



  void print() {
    cout << "LineNo: " << lineNo
         << ", type: " << type
         << ", oper: " << OperRepresentation(oper)
         << ", dest: " << destination
         << ", op1: " << operand1
         << ", op2: " << operand2
    << endl;
  }

  /**
   * Trims the left and right spaces from a string.
   */
  static string trim(string sentence) {
    size_t start = sentence.find_first_not_of(' ');
    size_t end = sentence.find_last_not_of(' ');
    return sentence.substr(start, (end -start)+1);
  }

  /**
   * Splits string into tokens substrings
   */
  vector<string> splitInstruction(string stmt) {

    // split statements
    vector<string> segments;
    stringstream ss( stmt ); // make string stream.
    string tok;

    while(getline(ss, tok, ',')) {
      segments.push_back(tok);
    }

    vector<string> segments2;
    // make individual words
    for(auto stmt : segments) {
      stringstream sg2(trim(stmt));
      while( getline(sg2, tok, ' ') ) {
        segments2.push_back(tok);
      }
    }
    return segments2;
  }
};

#endif // end instruction.h

