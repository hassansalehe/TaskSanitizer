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


// This file contains common type defs

#ifndef DEFS_H_ADFinspec
#define DEFS_H_ADFinspec

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <unordered_set>
#include <unordered_map>
#include <algorithm>
#include <cctype>
#include <vector>
#include <set>
#include <map>
#include <ctime>
#include <chrono>
#include <regex>
#include <sstream>

#include <mutex>          // std::mutex
//#include<//pthread.h>
//#include <thread>       // std::thread

using namespace std;

using Void = void;
typedef        void        VOID;
typedef      void *        ADDRESS;
typedef    ofstream        FILEPTR;
typedef    long int        INTEGER;
typedef    long int        VALUE;
typedef const char*        STRING;
typedef vector<int>        INTVECTOR;
typedef    set<int>        INTSET;
typedef unordered_set<int> UNORD_INTSET;
typedef        bool        BOOL;
typedef vector<string>     STRVECTOR;
using uint    =  unsigned int;
using ulong   =  unsigned long;
using lint    =  long int;
using address =  void *;

// for conflict task pairs
typedef pair<string,string>     STRPAIR;
typedef set<pair<int,int>>      INT_PAIRS;
typedef map<STRPAIR, INT_PAIRS> CONFLICT_PAIRS;

enum OPERATION {
  ALLOCA,
  BITCAST,
  CALL,
  GETELEMENTPTR,
  STORE,
  LOAD,
  RET,
  ADD,
  SUB,
  MUL,
  DIV,
  SHL,
};

static string OperRepresentation(OPERATION op) {

  switch(op) {
    case ALLOCA: return "ALLOCA";
    case BITCAST: return "BITCAST";
    case CALL: return "CALL";
    case GETELEMENTPTR: return "GETELEMENTPTR";
    case STORE: return "STORE";
    case LOAD: return "LOAD";
    case RET: return "RET";
    case ADD: return "ADD";
    case SUB: return "SUB";
    case MUL: return "MUL";
    case DIV: return "DIV";
    case SHL: return "SHL";
    default:
      return "UNKNOWN";
  }
};
#endif
