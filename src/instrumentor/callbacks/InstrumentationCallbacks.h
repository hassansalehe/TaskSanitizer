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

// callbacks for instrumentation

#ifndef _INSTRUMENTOR_CALLBACKS_INSTRUMENTATIONCALLBACKS_H_
#define _INSTRUMENTOR_CALLBACKS_INSTRUMENTATIONCALLBACKS_H_

#include "common/defs.h"
#include <iostream>
#include <pthread.h>
#include <unordered_map>
#include <sstream>

extern "C" {

  ////////////////////////////////////////////////////////////////////
  ///
  /// Callbacks below are used by OMPT callbacks and instrumentation
  /// callbacks.
  ///
  ///////////////////////////////////////////////////////////////////

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

  void __tasksan_write_float(void *addr, float v, int lnNo, void *fName);
  void __tasksan_write_double(void *addr, double v, int lnNo, void *fName);

  // task begin and end callbacks
  void INS_TaskBeginFunc(void *addr);
  void INS_TaskFinishFunc(void *addr);

  void toolVptrUpdate(void *addr, void *value);
  void toolVptrLoad(void *addr, void *value);


  ////////////////////////////////////////////////////////////////////
  ///
  /// Callbacks below are used by TaskSanitizer.cc instrumentation
  ///
  ///////////////////////////////////////////////////////////////////

  // This function should be called at the very beginning of the process,
  // before any instrumented code is executed and before any call to malloc.
  void __tasksan_init();

  void __tasksan_register_iir_file(void *);

  void __tasksan_flush_memory();

  void __tasksan_read1(void *addr, int lineNo, address funcName);
  void __tasksan_read2(void *addr, int lineNo, address funcName);
  void __tasksan_read4(void *addr, int lineNo, address funcName);
  void __tasksan_read8(void *addr, int lineNo, address funcName);
  void __tasksan_read16(void *addr, int lineNo, address funcName);

  void __tasksan_write1(void *addr, long int value, int lineNo, address funcName);
  void __tasksan_write2(void *addr, long int value, int lineNo, address funcName);
  void __tasksan_write4(void *addr, long int value, int lineNo, address funcName);
  void __tasksan_write8(void *addr, long int value, int lineNo, address funcName);
  void __tasksan_write16(void *addr, long int value, int lineNo, address funcName);

  void __tasksan_unaligned_read2(const void *addr);
  void __tasksan_unaligned_read4(const void *addr);
  void __tasksan_unaligned_read8(const void *addr);
  void __tasksan_unaligned_read16(const void *addr);

  void __tasksan_unaligned_write2(void *addr);
  void __tasksan_unaligned_write4(void *addr);
  void __tasksan_unaligned_write8(void *addr);
  void __tasksan_unaligned_write16(void *addr);

  void __tasksan_read1_pc(void *addr, void *pc);
  void __tasksan_read2_pc(void *addr, void *pc);
  void __tasksan_read4_pc(void *addr, void *pc);
  void __tasksan_read8_pc(void *addr, void *pc);
  void __tasksan_read16_pc(void *addr, void *pc);

  void __tasksan_write1_pc(void *addr, void *pc);
  void __tasksan_write2_pc(void *addr, void *pc);
  void __tasksan_write4_pc(void *addr, void *pc);
  void __tasksan_write8_pc(void *addr, void *pc);
  void __tasksan_write16_pc(void *addr, void *pc);

  void __tasksan_vptr_read(void **vptr_p);
  void __tasksan_vptr_update(void **vptr_p, void *new_val);

  void __tasksan_func_entry(void *call_pc);
  void __tasksan_func_exit(void * funcPtr);

  void __tasksan_ignore_thread_begin();
  void __tasksan_ignore_thread_end();

  void *__tasksan_external_register_tag(const char *object_type);
  void __tasksan_external_assign_tag(void *addr, void *tag);
  void __tasksan_external_read(void *addr, void *caller_pc, void *tag);
  void __tasksan_external_write(void *addr, void *caller_pc, void *tag);

  void __tasksan_read_range(void *addr, unsigned long size);  // NOLINT
  void __tasksan_write_range(void *addr, unsigned long size);  // NOLINT

  #ifdef __cplusplus
  }  // extern "C"
  #endif

  // These should match declarations from public tasksan_interface_atomic.h header.
  typedef unsigned char      a8;
  typedef unsigned short     a16;  // NOLINT
  typedef unsigned int       a32;
  typedef unsigned long long a64;  // NOLINT
  #if !SANITIZER_GO && (defined(__SIZEOF_INT128__) \
      || (__clang_major__ * 100 + __clang_minor__ >= 302)) && !defined(__mips64)
  __extension__ typedef __int128 a128;
  # define __TSAN_HAS_INT128 1
  #else
  # define __TSAN_HAS_INT128 0
  #endif

  // Part of ABI, do not change.
  // http://llvm.org/viewvc/llvm-project/libcxx/trunk/include/atomic?view=markup
  typedef enum {
    mo_relaxed,
    mo_consume,
    mo_acquire,
    mo_release,
    mo_acq_rel,
    mo_seq_cst
  } morder;

  extern "C" {

  a8 __tasksan_atomic8_load(const volatile a8 *a,    morder mo);
  a16 __tasksan_atomic16_load(const volatile a16 *a, morder mo);
  a32 __tasksan_atomic32_load(const volatile a32 *a, morder mo);
  a64 __tasksan_atomic64_load(const volatile a64 *a, morder mo);
  #if __TSAN_HAS_INT128
  a128 __tasksan_atomic128_load(const volatile a128 *a, morder mo);
  #endif

  void __tasksan_atomic8_store(volatile a8 *a,   a8 v,  morder mo);
  void __tasksan_atomic16_store(volatile a16 *a, a16 v, morder mo);
  void __tasksan_atomic32_store(volatile a32 *a, a32 v, morder mo);
  void __tasksan_atomic64_store(volatile a64 *a, a64 v, morder mo);
  #if __TSAN_HAS_INT128
  void __tasksan_atomic128_store(volatile a128 *a, a128 v, morder mo);
  #endif

  a8  __tasksan_atomic8_exchange(volatile a8 *a,    a8 v,  morder mo);
  a16 __tasksan_atomic16_exchange(volatile a16 *a, a16 v, morder mo);
  a32 __tasksan_atomic32_exchange(volatile a32 *a, a32 v, morder mo);
  a64 __tasksan_atomic64_exchange(volatile a64 *a, a64 v, morder mo);
  #if __TSAN_HAS_INT128
  a128 __tasksan_atomic128_exchange(volatile a128 *a, a128 v, morder mo);
  #endif

  a8  __tasksan_atomic8_fetch_add(volatile a8 *a, a8 v, morder mo);
  a16 __tasksan_atomic16_fetch_add(volatile a16 *a, a16 v, morder mo);
  a32 __tasksan_atomic32_fetch_add(volatile a32 *a, a32 v, morder mo);
  a64 __tasksan_atomic64_fetch_add(volatile a64 *a, a64 v, morder mo);
  #if __TSAN_HAS_INT128
  a128 __tasksan_atomic128_fetch_add(volatile a128 *a, a128 v, morder mo);
  #endif

  a8  __tasksan_atomic8_fetch_sub(volatile a8 *a, a8 v, morder mo);
  a16 __tasksan_atomic16_fetch_sub(volatile a16 *a, a16 v, morder mo);
  a32 __tasksan_atomic32_fetch_sub(volatile a32 *a, a32 v, morder mo);
  a64 __tasksan_atomic64_fetch_sub(volatile a64 *a, a64 v, morder mo);
  #if __TSAN_HAS_INT128
  a128 __tasksan_atomic128_fetch_sub(volatile a128 *a, a128 v, morder mo);
  #endif

  a8  __tasksan_atomic8_fetch_and(volatile a8 *a, a8 v, morder mo);
  a16 __tasksan_atomic16_fetch_and(volatile a16 *a, a16 v, morder mo);
  a32 __tasksan_atomic32_fetch_and(volatile a32 *a, a32 v, morder mo);
  a64 __tasksan_atomic64_fetch_and(volatile a64 *a, a64 v, morder mo);
  #if __TSAN_HAS_INT128
  a128 __tasksan_atomic128_fetch_and(volatile a128 *a, a128 v, morder mo);
  #endif

  a8  __tasksan_atomic8_fetch_or(volatile a8 *a, a8 v, morder mo);
  a16 __tasksan_atomic16_fetch_or(volatile a16 *a, a16 v, morder mo);
  a32 __tasksan_atomic32_fetch_or(volatile a32 *a, a32 v, morder mo);
  a64 __tasksan_atomic64_fetch_or(volatile a64 *a, a64 v, morder mo);
  #if __TSAN_HAS_INT128
  a128 __tasksan_atomic128_fetch_or(volatile a128 *a, a128 v, morder mo);
  #endif

  a8  __tasksan_atomic8_fetch_xor(volatile a8 *a, a8 v, morder mo);
  a16 __tasksan_atomic16_fetch_xor(volatile a16 *a, a16 v, morder mo);
  a32 __tasksan_atomic32_fetch_xor(volatile a32 *a, a32 v, morder mo);
  a64 __tasksan_atomic64_fetch_xor(volatile a64 *a, a64 v, morder mo);
  #if __TSAN_HAS_INT128
  a128 __tasksan_atomic128_fetch_xor(volatile a128 *a, a128 v, morder mo);
  #endif

  a8  __tasksan_atomic8_fetch_nand(volatile a8 *a, a8 v, morder mo);
  a16 __tasksan_atomic16_fetch_nand(volatile a16 *a, a16 v, morder mo);
  a32 __tasksan_atomic32_fetch_nand(volatile a32 *a, a32 v, morder mo);
  a64 __tasksan_atomic64_fetch_nand(volatile a64 *a, a64 v, morder mo);
  #if __TSAN_HAS_INT128
  a128 __tasksan_atomic128_fetch_nand(volatile a128 *a, a128 v, morder mo);
  #endif

  int __tasksan_atomic8_compare_exchange_strong(volatile a8 *a, a8 *c, a8 v,
                                             morder mo, morder fmo);
  int __tasksan_atomic16_compare_exchange_strong(volatile a16 *a, a16 *c, a16 v,
                                              morder mo, morder fmo);
  int __tasksan_atomic32_compare_exchange_strong(volatile a32 *a, a32 *c, a32 v,
                                              morder mo, morder fmo);
  int __tasksan_atomic64_compare_exchange_strong(volatile a64 *a, a64 *c, a64 v,
                                              morder mo, morder fmo);
  #if __TSAN_HAS_INT128
  int __tasksan_atomic128_compare_exchange_strong(volatile a128 *a, a128 *c, a128 v,
                                               morder mo, morder fmo);
  #endif

  int __tasksan_atomic8_compare_exchange_weak(volatile a8 *a, a8 *c, a8 v, morder mo,
                                           morder fmo);
  int __tasksan_atomic16_compare_exchange_weak(volatile a16 *a, a16 *c, a16 v,
                                            morder mo, morder fmo);
  int __tasksan_atomic32_compare_exchange_weak(volatile a32 *a, a32 *c, a32 v,
                                            morder mo, morder fmo);
  int __tasksan_atomic64_compare_exchange_weak(volatile a64 *a, a64 *c, a64 v,
                                            morder mo, morder fmo);
  #if __TSAN_HAS_INT128
  int __tasksan_atomic128_compare_exchange_weak(volatile a128 *a, a128 *c, a128 v,
                                             morder mo, morder fmo);
  #endif

  a8 __tasksan_atomic8_compare_exchange_val(volatile a8 *a, a8 c, a8 v, morder mo,
                                         morder fmo);
  a16 __tasksan_atomic16_compare_exchange_val(volatile a16 *a, a16 c, a16 v,
                                           morder mo, morder fmo);
  a32 __tasksan_atomic32_compare_exchange_val(volatile a32 *a, a32 c, a32 v,
                                           morder mo, morder fmo);
  a64 __tasksan_atomic64_compare_exchange_val(volatile a64 *a, a64 c, a64 v,
                                           morder mo, morder fmo);
  #if __TSAN_HAS_INT128
  a128 __tasksan_atomic128_compare_exchange_val(volatile a128 *a, a128 c, a128 v,
                                             morder mo, morder fmo);
  #endif

  void __tasksan_atomic_thread_fence(morder mo);
  void __tasksan_atomic_signal_fence(morder mo);

}; // second extern "C"

#endif // InstrumentationCallback.h
