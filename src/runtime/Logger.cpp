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

// implements logging functionalities

#ifndef LOGGER_CPP
#define LOGGER_CPP

#include "Logger.h"

// static attributes redefined
std::mutex INS::guardLock;

FILEPTR INS::logger;
FILEPTR INS::HBlogger;
ostringstream INS::HBloggerBuffer;
atomic<INTEGER> INS::taskIDSeed{ 0 };
unordered_map<STRING, INTEGER> INS::funcNames;
INTEGER INS::funcIDSeed = 1;
unordered_map<pair<ADDRESS,INTEGER>, INTEGER, hash_function> INS::idMap;
unordered_map<INTEGER, INTSET> INS::HB;
unordered_map<ADDRESS, INTEGER> INS::lastWriter;
unordered_map<ADDRESS, INTEGER> INS::lastReader;

#endif

