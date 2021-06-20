//===--------------------- -------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// (c) 2015 - 2018 Hassan Salehe Matar
//            Contact: hassansalehe-at-gmail-dot-com
//
//===----------------------------------------------------------------------===//

#include <stdio.h>
#include <thread>
#include <cassert>
#include <stdlib.h>
#include "instrumentor/callbacks/OMPTCallbacks.h"
#include "instrumentor/callbacks/InstrumentationCallbacks.h"

lint getMemoryValue( address addr, ulong size ) {
  if ( size == sizeof(char)   ) return *(static_cast<char *>(addr));
  if ( size == sizeof(short)  ) return *(static_cast<short *>(addr));
  if ( size == sizeof(int)    ) return *(static_cast<int *>(addr));
  if ( size == sizeof(float)  ) return *(static_cast<float *>(addr));
  if ( size == sizeof(double) ) return *(static_cast<double *>(addr));
  if ( size == sizeof(long)   ) return *(static_cast<long *>(addr));
  if ( size == sizeof(long long) ) {
    return *(static_cast<long long *>(addr));
  }
   // else, get the best value possible.
  return *(static_cast<long long *>(addr));
}

// to initialize the logger
void __tasksan_init() {
  INS::InitTaskSanitizerRuntime();
  PRINT_DEBUG("TaskSanitizer: init");
}

//////////////////////////////////////////////////
static TaskInfo * getTaskInfo(int * _type = NULL) {

  if (!INS::isOMPTinitialized) return NULL;

  int ancestor_level = 0;
  int type;
  ompt_data_t *task_data;
  ompt_frame_t *task_frame;
  ompt_data_t *parallel_data;
  int thread_num;

  int success = ompt_get_task_info(
      ancestor_level, &type, &task_data,
      &task_frame, &parallel_data, &thread_num);
  if (_type) {
    *_type = type;
  }
  if (success && task_data) {
    return (TaskInfo*)task_data->ptr;
  } else {
    return NULL;
  }
}

// Callbacks for store operations
inline void INS_MemRead(
  address addr,
    ulong size,
    int source_line_num,
    address funcName) {

  if (!source_line_num) return;

  TaskInfo * taskInfo = getTaskInfo();
  //lint value = getMemoryValue( addr, size );
  //uint threadID = (uint)pthread_self();

  if ( taskInfo && taskInfo->active ) {
    INS::Read(*taskInfo, addr, source_line_num, (char*)funcName);
#ifdef DEBUG
    std::stringstream ss;
    ss << std::hex << addr;
    PRINT_DEBUG("READ: addr: " + ss.str() +
        " taskID: " + std::to_string(taskInfo->taskID) +
        " line no: " + std::to_string(source_line_num));
#endif
  }
}

// Callbacks for store operations
inline void INS_MemWrite(
    address addr,
    lint value,
    int source_line_num,
    address funcName ) {

  if (!source_line_num) return;

  TaskInfo * taskInfo = getTaskInfo();
  //uint threadID = (uint)pthread_self();

  if ( taskInfo && taskInfo->active ) {
    INS::Write(*taskInfo, addr, (lint)value, source_line_num, (char*)funcName );
#ifdef DEBUG
    std::stringstream ss;
    ss << std::hex << addr;
    PRINT_DEBUG("= WRITE: addr: " + ss.str() +
        ", value: " + std::to_string((lint)value) +
        ", taskID: " + std::to_string(taskInfo->taskID) +
        ", line number: " + std::to_string(source_line_num) +
        ", func name: " + std::string((char*)funcName));
#endif
  }
}

// A callback for memory writes of floats
void __tasksan_write_float(
    address addr,
    float value,
    int source_line_num,
    address funcName) {
  INS_MemWrite(addr, (lint)value, source_line_num, funcName);
}

void __tasksan_register_iir_file(void * fileName) {
  INS::initCommutativityChecker( (char *)fileName );
}

// A callback for memory writes of doubles
void __tasksan_write_double(
    address addr,
    double value,
    int source_line_num,
    address funcName) {
  INS_MemWrite(addr, (lint)value, source_line_num, funcName);
}

void __tasksan_flush_memory() {
  PRINT_DEBUG("  TaskSanitizer: flush memory");
}

void __tasksan_read1(void *addr, int source_line_num, address funcName) {
  INS_MemRead(addr, 1, source_line_num, funcName);
}
void __tasksan_read2(void *addr, int source_line_num, address funcName) {
  INS_MemRead(addr, 2, source_line_num, funcName);
}

void __tasksan_read4(void *addr, int source_line_num, address funcName) {
  INS_MemRead(addr, 4, source_line_num, funcName);
}

void __tasksan_read8(void *addr, int source_line_num, address funcName) {
  INS_MemRead( addr, 8, source_line_num, funcName );
}

void __tasksan_read16(void *addr, int source_line_num, address funcName) {
  INS_MemRead( addr, 16, source_line_num, funcName );
}

void __tasksan_write1(void *addr, lint value, int source_line_num, address funcName) {
  INS_MemWrite((address)addr, value, source_line_num, funcName);
}

void __tasksan_write2(void *addr, lint value, int source_line_num, address funcName) {
  INS_MemWrite((address)addr, value, source_line_num, funcName);
}

void __tasksan_write4(void *addr, lint value, int source_line_num, address funcName) {
  INS_MemWrite((address)addr, value, source_line_num, funcName);
}

void __tasksan_write8(void *addr, lint value, int source_line_num, address funcName) {
  INS_MemWrite((address)addr, value, source_line_num, funcName);
}

void __tasksan_write16(void *addr, lint value, int source_line_num, address funcName) {
  INS_MemWrite((address)addr, value, source_line_num, funcName);
}

void __tasksan_unaligned_read2(const void *addr) {
  PRINT_DEBUG("  TaskSanitizer: unaligned read2");
}
void __tasksan_unaligned_read4(const void *addr) {
  std::stringstream ss; ss << addr;
  PRINT_DEBUG( "  TaskSanitizer: unaligned read4 "+ ss.str() );
}
void __tasksan_unaligned_read8(const void *addr) {
  PRINT_DEBUG("  TaskSanitizer: unaligned read8");
}
void __tasksan_unaligned_read16(const void *addr) {
  PRINT_DEBUG("  TaskSanitizer: unaligned read16");
}

void __tasksan_unaligned_write2(void *addr) {
  PRINT_DEBUG("  TaskSanitizer: unaligned write2");
}
void __tasksan_unaligned_write4(void *addr) {
  PRINT_DEBUG("  TaskSanitizer: unaligned write4");
}
void __tasksan_unaligned_write8(void *addr) {
  PRINT_DEBUG("  TaskSanitizer: unaligned write8");
}
void __tasksan_unaligned_write16(void *addr) {
  PRINT_DEBUG("  TaskSanitizer: unaligned write16");
}

void __tasksan_read1_pc(void *addr, void *pc) {
  PRINT_DEBUG("  TaskSanitizer: read1 pc");
}
void __tasksan_read2_pc(void *addr, void *pc) {
  PRINT_DEBUG("  TaskSanitizer: read2 pc");
}
void __tasksan_read4_pc(void *addr, void *pc) {
  PRINT_DEBUG("  TaskSanitizer: read4 pc");
}
void __tasksan_read8_pc(void *addr, void *pc) {
  PRINT_DEBUG("  TaskSanitizer: read8 pc");
}
void __tasksan_read16_pc(void *addr, void *pc) {
  PRINT_DEBUG("  TaskSanitizer: read16 pc");
}

void __tasksan_write1_pc(void *addr, void *pc) {
  PRINT_DEBUG("  TaskSanitizer: write1 pc");
}
void __tasksan_write2_pc(void *addr, void *pc) {
  PRINT_DEBUG("  TaskSanitizer: write2 pc");
}
void __tasksan_write4_pc(void *addr, void *pc) {
  PRINT_DEBUG("  TaskSanitizer: write4 pc");
}
void __tasksan_write8_pc(void *addr, void *pc) {
  PRINT_DEBUG("  TaskSanitizer: write8 pc");
}
void __tasksan_write16_pc(void *addr, void *pc) {
  PRINT_DEBUG("  TaskSanitizer: write16 pc");
}

void __tasksan_vptr_read(void **vptr_p) {
  PRINT_DEBUG("  TaskSanitizer: vptr read");
}

void __tasksan_vptr_update(void **vptr_p, void *new_val) {
  PRINT_DEBUG("  TaskSanitizer: vptr update");
}

void __tasksan_func_entry(void *call_pc) {
//  PRINT_DEBUG("  TaskSanitizer: __tasksan_func_entry ");
}

void __tasksan_func_exit(void * funcPtr) {
  if (funcPtr && std::string((char*) funcPtr) == "main") {
    INS::Finalize();
  }
  PRINT_DEBUG("TaskSanitizer: __tasksan_func_exit: "
      + std::string((char *)funcPtr));
}

void __tasksan_ignore_thread_begin() {
  PRINT_DEBUG("  TaskSanitizer: __tasksan_ignore_thread_begin");
}
void __tasksan_ignore_thread_end() {
  PRINT_DEBUG("  TaskSanitizer: __tasksan_ignore_thread_end");
}

void *__tasksan_external_register_tag(const char *object_type) {
  PRINT_DEBUG("  TaskSanitizer: __tasksan_external_register_tag");
  return NULL;
}

void __tasksan_external_assign_tag(void *addr, void *tag) {
  PRINT_DEBUG("  TaskSanitizer: __tasksan_external_assign_tag");
}

void __tasksan_external_read(void *addr, void *caller_pc, void *tag) {
  PRINT_DEBUG("  TaskSanitizer: __tasksan_external_read");
}

void __tasksan_external_write(void *addr, void *caller_pc, void *tag) {
  PRINT_DEBUG("  TaskSanitizer: __tasksan_external_write");
}


void __tasksan_read_range(void *addr, unsigned long size) {
  PRINT_DEBUG("  TaskSanitizer: __tasksan_read_range");
}  // NOLINT

void __tasksan_write_range(void *addr, unsigned long size) {
  PRINT_DEBUG("  TaskSanitizer: __tasksan_write_range");
}  // NOLINT

a8 __tasksan_atomic8_load(const volatile a8 *a, morder mo) {
  PRINT_DEBUG("  TaskSanitizer: __tasksan_atomic8_load");
  return *a;
}

a16 __tasksan_atomic16_load(const volatile a16 *a, morder mo) {
  PRINT_DEBUG("  TaskSanitizer: __tasksan_atomic16_load");
  return *a;
}

a32 __tasksan_atomic32_load(const volatile a32 *a, morder mo) {
  PRINT_DEBUG("  TaskSanitizer: __tasksan_atomic32_load");
  return *a;
}

a64 __tasksan_atomic64_load(const volatile a64 *a, morder mo) {
  PRINT_DEBUG("  TaskSanitizer: __tasksan_atomic64_load");
  return *a;
}
#if __TSAN_HAS_INT128

a128 __tasksan_atomic128_load(const volatile a128 *a, morder mo) {
  PRINT_DEBUG("  TaskSanitizer: ");
  return *a;
}
#endif


void __tasksan_atomic8_store(volatile a8 *a, a8 v, morder mo) {
  PRINT_DEBUG("  TaskSanitizer: ");
}

void __tasksan_atomic16_store(volatile a16 *a, a16 v, morder mo) {
  PRINT_DEBUG("  TaskSanitizer: ");
}

void __tasksan_atomic32_store(volatile a32 *a, a32 v, morder mo) {
  PRINT_DEBUG("  TaskSanitizer: ");
}

void __tasksan_atomic64_store(volatile a64 *a, a64 v, morder mo) {
  PRINT_DEBUG("  TaskSanitizer: ");
}
#if __TSAN_HAS_INT128

void __tasksan_atomic128_store(volatile a128 *a, a128 v, morder mo) {
  PRINT_DEBUG("  TaskSanitizer: ");
}
#endif


a8 __tasksan_atomic8_exchange(volatile a8 *a, a8 v, morder mo) {
  PRINT_DEBUG("  TaskSanitizer: atomic8 echange");
  return v;
}

a16 __tasksan_atomic16_exchange(volatile a16 *a, a16 v, morder mo) {
  PRINT_DEBUG("  TaskSanitizer: atomic exchange");
  return v;
}

a32 __tasksan_atomic32_exchange(volatile a32 *a, a32 v, morder mo) {
  PRINT_DEBUG("  TaskSanitizer: atomic exchange");
  return v;
}

a64 __tasksan_atomic64_exchange(volatile a64 *a, a64 v, morder mo) {
  PRINT_DEBUG("  TaskSanitizer: atomic exchange");
  return v;
}
#if __TSAN_HAS_INT128

a128 __tasksan_atomic128_exchange(volatile a128 *a, a128 v, morder mo) {
  PRINT_DEBUG("  TaskSanitizer: atomic exchange");
  return v;
}
#endif


a8 __tasksan_atomic8_fetch_add(volatile a8 *a, a8 v, morder mo) {
  PRINT_DEBUG("  TaskSanitizer: atomic8 fetch_add");
  return v;
}

a16 __tasksan_atomic16_fetch_add(volatile a16 *a, a16 v, morder mo) {
  PRINT_DEBUG("  TaskSanitizer: atomic16 fetch_add");
  return v;
}

a32 __tasksan_atomic32_fetch_add(volatile a32 *a, a32 v, morder mo) {
  // Fetch and add is commutative among tasks.
  // Therefore. no need to check for determinacy race.
  return __sync_fetch_and_add(a, v);
}

a64 __tasksan_atomic64_fetch_add(volatile a64 *a, a64 v, morder mo) {
  PRINT_DEBUG("  TaskSanitizer: atomic64 fetch_add");
  return v;
}
#if __TSAN_HAS_INT128

a128 __tasksan_atomic128_fetch_add(volatile a128 *a, a128 v, morder mo) {
  PRINT_DEBUG("  TaskSanitizer: atomic 128 fetch_add");
  return v;
}
#endif


a8 __tasksan_atomic8_fetch_sub(volatile a8 *a, a8 v, morder mo) {
  PRINT_DEBUG("  TaskSanitizer: ");
  return v;
}

a16 __tasksan_atomic16_fetch_sub(volatile a16 *a, a16 v, morder mo) {
  PRINT_DEBUG("  TaskSanitizer: ");
  return v;
}

a32 __tasksan_atomic32_fetch_sub(volatile a32 *a, a32 v, morder mo) {
  PRINT_DEBUG("  TaskSanitizer: ");
  return v;
}

a64 __tasksan_atomic64_fetch_sub(volatile a64 *a, a64 v, morder mo) {
  PRINT_DEBUG("  TaskSanitizer: ");
  return v;
}
#if __TSAN_HAS_INT128

a128 __tasksan_atomic128_fetch_sub(volatile a128 *a, a128 v, morder mo) {
  PRINT_DEBUG("  TaskSanitizer: ");
  return v;
}
#endif


a8 __tasksan_atomic8_fetch_and(volatile a8 *a, a8 v, morder mo) {
  PRINT_DEBUG("  TaskSanitizer: ");
  return v;
}

a16 __tasksan_atomic16_fetch_and(volatile a16 *a, a16 v, morder mo) {
  PRINT_DEBUG("  TaskSanitizer: ");
  return v;
}

a32 __tasksan_atomic32_fetch_and(volatile a32 *a, a32 v, morder mo) {
  PRINT_DEBUG("  TaskSanitizer: ");
  return v;
}

a64 __tasksan_atomic64_fetch_and(volatile a64 *a, a64 v, morder mo) {
  PRINT_DEBUG("  TaskSanitizer: ");
  return v;
}
#if __TSAN_HAS_INT128

a128 __tasksan_atomic128_fetch_and(volatile a128 *a, a128 v, morder mo) {
  PRINT_DEBUG("  TaskSanitizer: ");
  return v;
}
#endif


a8 __tasksan_atomic8_fetch_or(volatile a8 *a, a8 v, morder mo) {
  PRINT_DEBUG("  TaskSanitizer: ");
  return v;
}

a16 __tasksan_atomic16_fetch_or(volatile a16 *a, a16 v, morder mo) {
  PRINT_DEBUG("  TaskSanitizer: ");
  return v;
}

a32 __tasksan_atomic32_fetch_or(volatile a32 *a, a32 v, morder mo) {
  PRINT_DEBUG("  TaskSanitizer: ");
  return v;
}

a64 __tasksan_atomic64_fetch_or(volatile a64 *a, a64 v, morder mo) {
  PRINT_DEBUG("  TaskSanitizer: ");
  return v;
}
#if __TSAN_HAS_INT128

a128 __tasksan_atomic128_fetch_or(volatile a128 *a, a128 v, morder mo) {
  PRINT_DEBUG("  TaskSanitizer: ");
  return v;
}
#endif


a8 __tasksan_atomic8_fetch_xor(volatile a8 *a, a8 v, morder mo) {
  PRINT_DEBUG("  TaskSanitizer: ");
  return v;
}

a16 __tasksan_atomic16_fetch_xor(volatile a16 *a, a16 v, morder mo) {
  PRINT_DEBUG("  TaskSanitizer: ");
  return v;
}

a32 __tasksan_atomic32_fetch_xor(volatile a32 *a, a32 v, morder mo) {
  PRINT_DEBUG("  TaskSanitizer: ");
  return v;
}

a64 __tasksan_atomic64_fetch_xor(volatile a64 *a, a64 v, morder mo) {
  PRINT_DEBUG("  TaskSanitizer: ");
  return v;
}
#if __TSAN_HAS_INT128

a128 __tasksan_atomic128_fetch_xor(volatile a128 *a, a128 v, morder mo) {
  PRINT_DEBUG("  TaskSanitizer: ");
  return v;
}
#endif


a8 __tasksan_atomic8_fetch_nand(volatile a8 *a, a8 v, morder mo) {
  PRINT_DEBUG("  TaskSanitizer: ");
  return v;
}

a16 __tasksan_atomic16_fetch_nand(volatile a16 *a, a16 v, morder mo) {
  PRINT_DEBUG("  TaskSanitizer: ");
  return v;
}

a32 __tasksan_atomic32_fetch_nand(volatile a32 *a, a32 v, morder mo) {
  PRINT_DEBUG("  TaskSanitizer: ");
  return v;
}

a64 __tasksan_atomic64_fetch_nand(volatile a64 *a, a64 v, morder mo) {
  PRINT_DEBUG("  TaskSanitizer: ");
  return v;
}
#if __TSAN_HAS_INT128

a128 __tasksan_atomic128_fetch_nand(volatile a128 *a, a128 v, morder mo) {
  PRINT_DEBUG("  TaskSanitizer: ");
  return v;
}
#endif


int __tasksan_atomic8_compare_exchange_strong(volatile a8 *a, a8 *c, a8 v,
                                           morder mo, morder fmo) {
  PRINT_DEBUG("  TaskSanitizer: ");
  return v;
}

int __tasksan_atomic16_compare_exchange_strong(volatile a16 *a, a16 *c, a16 v,
                                            morder mo, morder fmo) {
  PRINT_DEBUG("  TaskSanitizer: ");
  return v;
}

int __tasksan_atomic32_compare_exchange_strong(volatile a32 *a, a32 *c, a32 v,
                                            morder mo, morder fmo) {
  PRINT_DEBUG("  TaskSanitizer: ");
  return v;
}

int __tasksan_atomic64_compare_exchange_strong(volatile a64 *a, a64 *c, a64 v,
                                            morder mo, morder fmo) {
  PRINT_DEBUG("  TaskSanitizer: ");
  return v;
}
#if __TSAN_HAS_INT128

int __tasksan_atomic128_compare_exchange_strong(volatile a128 *a, a128 *c, a128 v,
                                             morder mo, morder fmo) {
  PRINT_DEBUG("  TaskSanitizer: ");
  return v;
}
#endif


int __tasksan_atomic8_compare_exchange_weak(volatile a8 *a, a8 *c, a8 v, morder mo,
                                         morder fmo) {
  PRINT_DEBUG("  TaskSanitizer: atomic8 cpex weak");
  return v;
}

int __tasksan_atomic16_compare_exchange_weak(volatile a16 *a, a16 *c, a16 v,
                                          morder mo, morder fmo) {
  PRINT_DEBUG("  TaskSanitizer: atomic16 cpex weak");
  return v;
}

int __tasksan_atomic32_compare_exchange_weak(volatile a32 *a, a32 *c, a32 v,
                                          morder mo, morder fmo) {
  PRINT_DEBUG("  TaskSanitizer: atomic32 cpex weak");
  return v;
}

int __tasksan_atomic64_compare_exchange_weak(volatile a64 *a, a64 *c, a64 v,
                                          morder mo, morder fmo) {
  PRINT_DEBUG("  TaskSanitizer: atomic64 cpex weak");
  return v;
}
#if __TSAN_HAS_INT128

int __tasksan_atomic128_compare_exchange_weak(volatile a128 *a, a128 *c, a128 v,
                                           morder mo, morder fmo) {
  PRINT_DEBUG("  TaskSanitizer: atomic128 cpex weak");
  return v;
}
#endif


a8 __tasksan_atomic8_compare_exchange_val(volatile a8 *a, a8 c, a8 v, morder mo,
                                       morder fmo) {
  PRINT_DEBUG("  TaskSanitizer: atomic8 cpex");
  return v;
}

a16 __tasksan_atomic16_compare_exchange_val(volatile a16 *a, a16 c, a16 v,
                                         morder mo, morder fmo) {
  PRINT_DEBUG("  TaskSanitizer: atomic16 cpex");
  return v;
}

a32 __tasksan_atomic32_compare_exchange_val(volatile a32 *a, a32 c, a32 v,
                                         morder mo, morder fmo) {
  PRINT_DEBUG("  TaskSanitizer: atomic32 cpex");
  return v;
}

a64 __tasksan_atomic64_compare_exchange_val(volatile a64 *a, a64 c, a64 v,
                                         morder mo, morder fmo) {
  PRINT_DEBUG("  TaskSanitizer: atomic64 cpex");
  return v;
}
#if __TSAN_HAS_INT128

a128 __tasksan_atomic128_compare_exchange_val(volatile a128 *a, a128 c, a128 v,
                                           morder mo, morder fmo) {
  PRINT_DEBUG("  TaskSanitizer: atomic128 cpex");
  return v;
}
#endif


void __tasksan_atomic_thread_fence(morder mo) {
  PRINT_DEBUG("  TaskSanitizer: __tasksan_atomic_thread_fence");
}

void __tasksan_atomic_signal_fence(morder mo) {
  PRINT_DEBUG("  TaskSanitizer: __tasksan_atomic_signal_fence");
}
