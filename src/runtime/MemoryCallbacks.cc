//===-- tsan_interface.cc -------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// (c) 2017 - 2018 - Hassan Salehe Matar, Koc University
//            Email: hmatar@ku.edu.tr
//
//===----------------------------------------------------------------------===//

#include <stdio.h>
#include <thread>
#include <cassert>
#include <stdlib.h>
#include "OMPTCallbacks.h"
#include "tsan_interface.h"

// #define DEBUG

lint getMemoryValue( address addr, ulong size )
{
  if( size == sizeof(char) )
     return *(static_cast<char *>(addr));

  if( size == sizeof(short) )
     return *(static_cast<short *>(addr));

  if( size == sizeof(int) )
     return *(static_cast<int *>(addr));

  if( size == sizeof(float) )
    return *(static_cast<float *>(addr));

  if( size == sizeof(double) )
    return *(static_cast<double *>(addr));

  if( size == sizeof(long) )
     return *(static_cast<long *>(addr));

   if( size == sizeof(long long) )
     return *(static_cast<long long *>(addr));

   // else, get the best value possible.
   return *(static_cast<long long *>(addr));
}

// to initialize the logger
void __tsan_init() {
  INS::InitFlowsanRuntime();
#ifdef DEBUG
  PRINT_DEBUG("Flowsan: init");
#endif
}

//////////////////////////////////////////////////
static TaskInfo * getTaskInfo(int * _type = NULL) {

  if(!INS::isOMPTinitialized)
    return NULL;

  int ancestor_level = 0;
  int type;
  ompt_data_t *task_data;
  ompt_frame_t *task_frame;
  ompt_data_t *parallel_data;
  int thread_num;

  int success = ompt_get_task_info(
      ancestor_level, &type, &task_data,
      &task_frame, &parallel_data, &thread_num);
  if(_type)
    *_type = type;

  if(success && task_data)
    return (TaskInfo*)task_data->ptr;

  return NULL;
}

/** Callbacks for store operations  */
inline void INS_AdfMemRead( address addr, ulong size, int lineNo, address funcName  ) {

  if(!lineNo) return;

  TaskInfo * taskInfo = getTaskInfo();
  //lint value = getMemoryValue( addr, size );
  //uint threadID = (uint)pthread_self();

  if( taskInfo && taskInfo->active ) {
    INS::Read(*taskInfo, addr, lineNo, (char*)funcName);
#ifdef DEBUG
    std::stringstream ss;
    ss << std::hex << addr;
    PRINT_DEBUG("READ: addr: " + ss.str() +
        " taskID: " + to_string(taskInfo->taskID) +
        " line no: " + to_string(lineNo));
#endif
  }
}

/** Callbacks for store operations  */
inline void INS_AdfMemWrite( address addr, lint value, int lineNo, address funcName ) {

  if(!lineNo) return;

  TaskInfo * taskInfo = getTaskInfo();
  //uint threadID = (uint)pthread_self();

  if( taskInfo && taskInfo->active ) {
    INS::Write(*taskInfo, addr, (lint)value, lineNo, (char*)funcName );
#ifdef DEBUG
    std::stringstream ss;
    ss << std::hex << addr;
    PRINT_DEBUG("= WRITE: addr: " + ss.str() +
        ", value: " + to_string((lint)value) +
        ", taskID: " + to_string(taskInfo->taskID) +
        ", line number: " + to_string(lineNo) +
        ", func name: " + string((char*)funcName));
#endif
  }
}


/**
 * A callback for memory writes of floats */
void __fsan_write_float(
    address addr,
    float value,
    int lineNo,
    address funcName) {
  INS_AdfMemWrite(addr, (lint)value, lineNo, funcName);
}

/**
 * A callback for memory writes of doubles */
void __fsan_write_double(
    address addr,
    double value,
    int lineNo,
    address funcName) {
  INS_AdfMemWrite(addr, (lint)value, lineNo, funcName);
}

void __tsan_flush_memory() {
  printf("  Flowsan: flush memory\n");
}

void __tsan_read1(void *addr, int lineNo, address funcName) {
  INS_AdfMemRead(addr, 1, lineNo, funcName);
}
void __tsan_read2(void *addr, int lineNo, address funcName) {
  INS_AdfMemRead(addr, 2, lineNo, funcName);
}

void __tsan_read4(void *addr, int lineNo, address funcName) {
  INS_AdfMemRead(addr, 4, lineNo, funcName);
}

void __tsan_read8(void *addr, int lineNo, address funcName) {
  INS_AdfMemRead( addr, 8, lineNo, funcName );
}

void __tsan_read16(void *addr, int lineNo, address funcName) {
  INS_AdfMemRead( addr, 16, lineNo, funcName );
}

void __tsan_write1(void *addr, lint value, int lineNo, address funcName) {
  INS_AdfMemWrite((address)addr, value, lineNo, funcName);
}

void __tsan_write2(void *addr, lint value, int lineNo, address funcName) {
  INS_AdfMemWrite((address)addr, value, lineNo, funcName);
}

void __tsan_write4(void *addr, lint value, int lineNo, address funcName) {
  INS_AdfMemWrite((address)addr, value, lineNo, funcName);
}

void __tsan_write8(void *addr, lint value, int lineNo, address funcName) {
  INS_AdfMemWrite((address)addr, value, lineNo, funcName);
}

void __tsan_write16(void *addr, lint value, int lineNo, address funcName) {
  INS_AdfMemWrite((address)addr, value, lineNo, funcName);
}

void __tsan_unaligned_read2(const void *addr) {
  printf("  Flowsan: unaligned read2\n");
}
void __tsan_unaligned_read4(const void *addr) {
  printf("  Flowsan: unaligned read4 %p\n", addr);
}
void __tsan_unaligned_read8(const void *addr) {
  printf("  Flowsan: unaligned read8\n");
}
void __tsan_unaligned_read16(const void *addr) {
  printf("  Flowsan: unaligned read16\n");
}

void __tsan_unaligned_write2(void *addr) {
  printf("  Flowsan: unaligned write2\n");
}
void __tsan_unaligned_write4(void *addr) {
  printf("  Flowsan: unaligned write4\n");
}
void __tsan_unaligned_write8(void *addr) {
  printf("  Flowsan: unaligned write8\n");
}
void __tsan_unaligned_write16(void *addr) {
  printf("  Flowsan: unaligned write16\n");
}

void __tsan_read1_pc(void *addr, void *pc) {
  printf("  Flowsan: read1 pc\n");
}
void __tsan_read2_pc(void *addr, void *pc) {
  printf("  Flowsan: read2 pc\n");
}
void __tsan_read4_pc(void *addr, void *pc) {
  printf("  Flowsan: read4 pc\n");
}
void __tsan_read8_pc(void *addr, void *pc) {
  printf("  Flowsan: read8 pc\n");
}
void __tsan_read16_pc(void *addr, void *pc) {
  printf("  Flowsan: read16 pc\n");
}

void __tsan_write1_pc(void *addr, void *pc) {
  printf("  Flowsan: write1 pc\n");
}
void __tsan_write2_pc(void *addr, void *pc) {
  printf("  Flowsan: write2 pc\n");
}
void __tsan_write4_pc(void *addr, void *pc) {
  printf("  Flowsan: write4 pc\n");
}
void __tsan_write8_pc(void *addr, void *pc) {
  printf("  Flowsan: write8 pc\n");
}
void __tsan_write16_pc(void *addr, void *pc) {
  printf("  Flowsan: write16 pc\n");
}

void __tsan_vptr_read(void **vptr_p) {
  printf("  Flowsan: vptr read\n");
}

void __tsan_vptr_update(void **vptr_p, void *new_val) {
  printf("  Flowsan: vptr update\n");
}

void __tsan_func_entry(void *call_pc) {
#ifdef DEBUG
//  printf("  Flowsan: __tsan_func_entry \n");
#endif
}

void __tsan_func_exit(void * funcPtr) {
  if(funcPtr && string((char*) funcPtr) == "main")
   INS::Finalize();
#ifdef DEBUG
  PRINT_DEBUG("Flowsan: __tsan_func_exit");
#endif
}

void __tsan_ignore_thread_begin() {
  printf("  Flowsan: __tsan_ignore_thread_begin\n");
}
void __tsan_ignore_thread_end() {
  printf("  Flowsan: __tsan_ignore_thread_end\n");
}

void *__tsan_external_register_tag(const char *object_type) {
  printf("  Flowsan: __tsan_external_register_tag\n");
  return NULL;
}

void __tsan_external_assign_tag(void *addr, void *tag) {
  printf("  Flowsan: __tsan_external_assign_tag\n");
}

void __tsan_external_read(void *addr, void *caller_pc, void *tag) {
  printf("  Flowsan: __tsan_external_read\n");
}

void __tsan_external_write(void *addr, void *caller_pc, void *tag) {
  printf("  Flowsan: __tsan_external_write\n");
}


void __tsan_read_range(void *addr, unsigned long size) {
  printf("  Flowsan: __tsan_read_range\n");
}  // NOLINT

void __tsan_write_range(void *addr, unsigned long size) {
  printf("  Flowsan: __tsan_write_range\n");
}  // NOLINT

a8 __tsan_atomic8_load(const volatile a8 *a, morder mo) {
  printf("  Flowsan: __tsan_atomic8_load\n");
  return *a;
}

a16 __tsan_atomic16_load(const volatile a16 *a, morder mo) {
  printf("  Flowsan: __tsan_atomic16_load\n");
  return *a;
}

a32 __tsan_atomic32_load(const volatile a32 *a, morder mo) {
  printf("  Flowsan: __tsan_atomic32_load\n");
  return *a;
}

a64 __tsan_atomic64_load(const volatile a64 *a, morder mo) {
  printf("  Flowsan: __tsan_atomic64_load\n");
  return *a;
}
#if __TSAN_HAS_INT128

a128 __tsan_atomic128_load(const volatile a128 *a, morder mo) {
  printf("  Flowsan: \n");
  return *a;
}
#endif


void __tsan_atomic8_store(volatile a8 *a, a8 v, morder mo) {
  printf("  Flowsan: \n");
}

void __tsan_atomic16_store(volatile a16 *a, a16 v, morder mo) {
  printf("  Flowsan: \n");
}

void __tsan_atomic32_store(volatile a32 *a, a32 v, morder mo) {
  printf("  Flowsan: \n");
}

void __tsan_atomic64_store(volatile a64 *a, a64 v, morder mo) {
  printf("  Flowsan: \n");
}
#if __TSAN_HAS_INT128

void __tsan_atomic128_store(volatile a128 *a, a128 v, morder mo) {
  printf("  Flowsan: \n");
}
#endif


a8 __tsan_atomic8_exchange(volatile a8 *a, a8 v, morder mo) {
  printf("  Flowsan: atomic8 echange\n");
  return v;
}

a16 __tsan_atomic16_exchange(volatile a16 *a, a16 v, morder mo) {
  printf("  Flowsan: atomic exchange\n");
  return v;
}

a32 __tsan_atomic32_exchange(volatile a32 *a, a32 v, morder mo) {
  printf("  Flowsan: atomic exchange\n");
  return v;
}

a64 __tsan_atomic64_exchange(volatile a64 *a, a64 v, morder mo) {
  printf("  Flowsan: atomic exchange\n");
  return v;
}
#if __TSAN_HAS_INT128

a128 __tsan_atomic128_exchange(volatile a128 *a, a128 v, morder mo) {
  printf("  Flowsan: atomic exchange\n");
  return v;
}
#endif


a8 __tsan_atomic8_fetch_add(volatile a8 *a, a8 v, morder mo) {
  printf("  Flowsan: atomic8 fetch_add\n");
  return v;
}

a16 __tsan_atomic16_fetch_add(volatile a16 *a, a16 v, morder mo) {
  printf("  Flowsan: atomic16 fetch_add\n");
  return v;
}

a32 __tsan_atomic32_fetch_add(volatile a32 *a, a32 v, morder mo) {
  // Fetch and add is commutative among tasks.
  // Therefore. no need to check for nondeterminism.
  return __sync_fetch_and_add(a, v);
}

a64 __tsan_atomic64_fetch_add(volatile a64 *a, a64 v, morder mo) {
  printf("  Flowsan: atomic64 fetch_add\n");
  return v;
}
#if __TSAN_HAS_INT128

a128 __tsan_atomic128_fetch_add(volatile a128 *a, a128 v, morder mo) {
  printf("  Flowsan: atomic 128 fetch_add\n");
  return v;
}
#endif


a8 __tsan_atomic8_fetch_sub(volatile a8 *a, a8 v, morder mo) {
  printf("  Flowsan: \n");
  return v;
}

a16 __tsan_atomic16_fetch_sub(volatile a16 *a, a16 v, morder mo) {
  printf("  Flowsan: \n");
  return v;
}

a32 __tsan_atomic32_fetch_sub(volatile a32 *a, a32 v, morder mo) {
  printf("  Flowsan: \n");
  return v;
}

a64 __tsan_atomic64_fetch_sub(volatile a64 *a, a64 v, morder mo) {
  printf("  Flowsan: \n");
  return v;
}
#if __TSAN_HAS_INT128

a128 __tsan_atomic128_fetch_sub(volatile a128 *a, a128 v, morder mo) {
  printf("  Flowsan: \n");
  return v;
}
#endif


a8 __tsan_atomic8_fetch_and(volatile a8 *a, a8 v, morder mo) {
  printf("  Flowsan: \n");
  return v;
}

a16 __tsan_atomic16_fetch_and(volatile a16 *a, a16 v, morder mo) {
  printf("  Flowsan: \n");
  return v;
}

a32 __tsan_atomic32_fetch_and(volatile a32 *a, a32 v, morder mo) {
  printf("  Flowsan: \n");
  return v;
}

a64 __tsan_atomic64_fetch_and(volatile a64 *a, a64 v, morder mo) {
  printf("  Flowsan: \n");
  return v;
}
#if __TSAN_HAS_INT128

a128 __tsan_atomic128_fetch_and(volatile a128 *a, a128 v, morder mo) {
  printf("  Flowsan: \n");
  return v;
}
#endif


a8 __tsan_atomic8_fetch_or(volatile a8 *a, a8 v, morder mo) {
  printf("  Flowsan: \n");
  return v;
}

a16 __tsan_atomic16_fetch_or(volatile a16 *a, a16 v, morder mo) {
  printf("  Flowsan: \n");
  return v;
}

a32 __tsan_atomic32_fetch_or(volatile a32 *a, a32 v, morder mo) {
  printf("  Flowsan: \n");
  return v;
}

a64 __tsan_atomic64_fetch_or(volatile a64 *a, a64 v, morder mo) {
  printf("  Flowsan: \n");
  return v;
}
#if __TSAN_HAS_INT128

a128 __tsan_atomic128_fetch_or(volatile a128 *a, a128 v, morder mo) {
  printf("  Flowsan: \n");
  return v;
}
#endif


a8 __tsan_atomic8_fetch_xor(volatile a8 *a, a8 v, morder mo) {
  printf("  Flowsan: \n");
  return v;
}

a16 __tsan_atomic16_fetch_xor(volatile a16 *a, a16 v, morder mo) {
  printf("  Flowsan: \n");
  return v;
}

a32 __tsan_atomic32_fetch_xor(volatile a32 *a, a32 v, morder mo) {
  printf("  Flowsan: \n");
  return v;
}

a64 __tsan_atomic64_fetch_xor(volatile a64 *a, a64 v, morder mo) {
  printf("  Flowsan: \n");
  return v;
}
#if __TSAN_HAS_INT128

a128 __tsan_atomic128_fetch_xor(volatile a128 *a, a128 v, morder mo) {
  printf("  Flowsan: \n");
  return v;
}
#endif


a8 __tsan_atomic8_fetch_nand(volatile a8 *a, a8 v, morder mo) {
  printf("  Flowsan: \n");
  return v;
}

a16 __tsan_atomic16_fetch_nand(volatile a16 *a, a16 v, morder mo) {
  printf("  Flowsan: \n");
  return v;
}

a32 __tsan_atomic32_fetch_nand(volatile a32 *a, a32 v, morder mo) {
  printf("  Flowsan: \n");
  return v;
}

a64 __tsan_atomic64_fetch_nand(volatile a64 *a, a64 v, morder mo) {
  printf("  Flowsan: \n");
  return v;
}
#if __TSAN_HAS_INT128

a128 __tsan_atomic128_fetch_nand(volatile a128 *a, a128 v, morder mo) {
  printf("  Flowsan: \n");
  return v;
}
#endif


int __tsan_atomic8_compare_exchange_strong(volatile a8 *a, a8 *c, a8 v,
                                           morder mo, morder fmo) {
  printf("  Flowsan: \n");
  return v;
}

int __tsan_atomic16_compare_exchange_strong(volatile a16 *a, a16 *c, a16 v,
                                            morder mo, morder fmo) {
  printf("  Flowsan: \n");
  return v;
}

int __tsan_atomic32_compare_exchange_strong(volatile a32 *a, a32 *c, a32 v,
                                            morder mo, morder fmo) {
  printf("  Flowsan: \n");
  return v;
}

int __tsan_atomic64_compare_exchange_strong(volatile a64 *a, a64 *c, a64 v,
                                            morder mo, morder fmo) {
  printf("  Flowsan: \n");
  return v;
}
#if __TSAN_HAS_INT128

int __tsan_atomic128_compare_exchange_strong(volatile a128 *a, a128 *c, a128 v,
                                             morder mo, morder fmo) {
  printf("  Flowsan: \n");
  return v;
}
#endif


int __tsan_atomic8_compare_exchange_weak(volatile a8 *a, a8 *c, a8 v, morder mo,
                                         morder fmo) {
  printf("  Flowsan: atomic8 cpex weak\n");
  return v;
}

int __tsan_atomic16_compare_exchange_weak(volatile a16 *a, a16 *c, a16 v,
                                          morder mo, morder fmo) {
  printf("  Flowsan: atomic16 cpex weak\n");
  return v;
}

int __tsan_atomic32_compare_exchange_weak(volatile a32 *a, a32 *c, a32 v,
                                          morder mo, morder fmo) {
  printf("  Flowsan: atomic32 cpex weak\n");
  return v;
}

int __tsan_atomic64_compare_exchange_weak(volatile a64 *a, a64 *c, a64 v,
                                          morder mo, morder fmo) {
  printf("  Flowsan: atomic64 cpex weak\n");
  return v;
}
#if __TSAN_HAS_INT128

int __tsan_atomic128_compare_exchange_weak(volatile a128 *a, a128 *c, a128 v,
                                           morder mo, morder fmo) {
  printf("  Flowsan: atomic128 cpex weak\n");
  return v;
}
#endif


a8 __tsan_atomic8_compare_exchange_val(volatile a8 *a, a8 c, a8 v, morder mo,
                                       morder fmo) {
  printf("  Flowsan: atomic8 cpex\n");
  return v;
}

a16 __tsan_atomic16_compare_exchange_val(volatile a16 *a, a16 c, a16 v,
                                         morder mo, morder fmo) {
  printf("  Flowsan: atomic16 cpex\n");
  return v;
}

a32 __tsan_atomic32_compare_exchange_val(volatile a32 *a, a32 c, a32 v,
                                         morder mo, morder fmo) {
  printf("  Flowsan: atomic32 cpex\n");
  return v;
}

a64 __tsan_atomic64_compare_exchange_val(volatile a64 *a, a64 c, a64 v,
                                         morder mo, morder fmo) {
  printf("  Flowsan: atomic64 cpex\n");
  return v;
}
#if __TSAN_HAS_INT128

a128 __tsan_atomic128_compare_exchange_val(volatile a128 *a, a128 c, a128 v,
                                           morder mo, morder fmo) {
  printf("  Flowsan: atomic128 cpex\n");
  return v;
}
#endif


void __tsan_atomic_thread_fence(morder mo) {
  printf("  Flowsan: __tsan_atomic_thread_fence\n");
}

void __tsan_atomic_signal_fence(morder mo) {
  printf("  Flowsan: __tsan_atomic_signal_fence\n");
}
