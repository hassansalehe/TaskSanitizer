/////////////////////////////////////////////////////////////////
//  TaskSanitizer: a lightweight non-determinism checking
//          tool for OpenMP task applications
//
//    Copyright (c) 2015 - 2018 Hassan Salehe Matar
//      Copying or using this code by any means whatsoever
//      without consent of the owner is strictly prohibited.
//
//   Contact: hassansalehe-at-gmail-dot-com
//
/////////////////////////////////////////////////////////////////

// This file contains common type defs

#ifndef _COMMON_DEFS_H_
#define _COMMON_DEFS_H_

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

typedef   bool                      BOOL;
typedef   void                      VOID;
typedef   void *                    ADDRESS;
typedef   std::ofstream             FILEPTR;
typedef   long int                  INTEGER;
typedef   long int                  VALUE;
typedef   const char*               STRING;
typedef   std::vector<int>          INTVECTOR;
typedef   std::set<int>             INTSET;
typedef   std::unordered_set<int>   UNORD_INTSET;
typedef   std::vector<std::string>  STRVECTOR;

using uint      =    unsigned int;
using ulong     =    unsigned long;
using lint      =    long int;
using address   =    void *;
using Void      =   void;

// for conflict task pairs
typedef  std::pair<int,int>            IDPAIR;
typedef  std::set<std::pair<int,int>>  LINE_PAIRS;
typedef  std::map<IDPAIR, LINE_PAIRS>  CONFLICT_PAIRS;

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

/**
  * Definition below is for debugging printfs */
// #define DEBUG

static std::mutex printLock;
void inline PRINT_DEBUG(const std::string msg) {
#ifdef DEBUG
  printLock.lock();
  std::cout << static_cast<uint>(pthread_self()) << ": " << msg << std::endl;
  printLock.unlock();
#endif // debug
}

#endif
