/////////////////////////////////////////////////////////////////
//  TaskSanitizer: a lightweight determinacy race checking
//          tool for OpenMP task applications
//
//    Copyright (c) 2015 - 2021 Hassan Salehe Matar
//      Copying or using this code by any means whatsoever
//      without consent of the owner is strictly prohibited.
//
//   Contact: hassansalehe-at-gmail-dot-com
//
/////////////////////////////////////////////////////////////////

// This file contains common type defs

#ifndef _COMMON_DEFS_H_
#define _COMMON_DEFS_H_

#include <algorithm>
#include <cctype>
#include <chrono>
#include <ctime>
#include <fstream>
#include <iostream>
#include <map>
#include <mutex>
#include <regex>
#include <set>
#include <sstream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

using   BOOL         = bool;
using   VOID         = void;
using   ADDRESS      = void *;
using   FILEPTR      = std::ofstream;
using   INTEGER      = long int;
using   VALUE        = long int;
using   STRING       = const char*;
using   INTVECTOR    = std::vector<int>;
using   INTSET       = std::set<int>;
using   UNORD_INTSET = std::unordered_set<int>;
using   STRVECTOR    = std::vector<std::string>;

// for conflict task pairs
using IDPAIR         = std::pair<int,int>;
using LINE_PAIRS     = std::set<std::pair<int,int>>;
using CONFLICT_PAIRS = std::map<IDPAIR, LINE_PAIRS>;

using uint    = unsigned int;
using ulong   = unsigned long;
using lint    = long int;
using address = void *;
using Void    = void;

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

static std::string OperRepresentation(OPERATION op) {

  switch( op )
  {
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

//////////////////////////////////////////////////
/// Definition below is for debugging printfs   //
//////////////////////////////////////////////////
// #define DEBUG

static std::mutex printLock;
void inline PRINT_DEBUG(const std::string msg) {
#ifdef DEBUG
  printLock.lock();
  std::cout << static_cast<uint>(pthread_self()) << ": " << msg << std::endl;
  printLock.unlock();
#endif // debug
}

#endif // guard
