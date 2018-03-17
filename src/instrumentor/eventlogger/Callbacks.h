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

// callbacks for instrumentation

#ifndef _INSTRUMENTOR_EVENTLOGGER_CALLBACKS_H_
#define _INSTRUMENTOR_EVENTLOGGER_CALLBACKS_H_

#include <iostream>
#include <pthread.h>
#include <unordered_map>
#include <sstream>

extern "C" {

  // to initialize the logger
  void INS_Init();

  // to finalize and book-keep the logger
  void INS_Fini();

  // callbacks for tokens
  void INS_RegReceiveToken(void *tokenAddr, unsigned long size);
  void INS_RegSendToken(void *bufferAddr, void *tokenAddr,
                        unsigned long size);

  // callbacks for memory access, race detection
  void INS_MemRead8(void *addr, int lineNo, void *fName);
  void INS_MemRead4(void *addr, int lineNo, void *fName);
  void INS_MemRead1(void *addr, int lineNo, void *fName);
  void INS_MemWrite8(void *addr, long int v, int lnNo, void *fName);
  void INS_MemWrite4(void *addr, long int v, int lnNo, void *fName);
  void INS_MemWrite1(void *addr, long int v, int lnNo, void *fName);

  void __fsan_write_float(void *addr, float v, int lnNo, void *fName);
  void __fsan_write_double(void *addr, double v, int lnNo, void *fName);

  // task begin and end callbacks
  void INS_TaskBeginFunc(void *addr);
  void INS_TaskFinishFunc(void *addr);

  void toolVptrUpdate(void *addr, void *value);
  void toolVptrLoad(void *addr, void *value);
};
#endif // Callback.h
