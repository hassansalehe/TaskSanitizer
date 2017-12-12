//===-- tsan_interface.h ----------------------------------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file is a part of ThreadSanitizer (TSan), a race detector.
//
// The functions declared in this header will be inserted by the instrumentation
// module.
// This header can be included by the instrumented program or by TSan tests.
//===----------------------------------------------------------------------===//
#ifndef TSAN_INTERFACE_H
#define TSAN_INTERFACE_H

// This header should NOT include any other headers.
// All functions in this header are extern "C" and start with __tsan_.

#include "defs.h"

#ifdef __cplusplus
extern "C" {
#endif

// This function should be called at the very beginning of the process,
// before any instrumented code is executed and before any call to malloc.
void __tsan_init();

void __tsan_flush_memory();

void __tsan_read1(void *addr, int lineNo, address funcName);
void __tsan_read2(void *addr, int lineNo, address funcName);
void __tsan_read4(void *addr, int lineNo, address funcName);
void __tsan_read8(void *addr, int lineNo, address funcName);
void __tsan_read16(void *addr, int lineNo, address funcName);

void __tsan_write1(void *addr, long int value, int lineNo, address funcName);
void __tsan_write2(void *addr, long int value, int lineNo, address funcName);
void __tsan_write4(void *addr, long int value, int lineNo, address funcName);
void __tsan_write8(void *addr, long int value, int lineNo, address funcName);
void __tsan_write16(void *addr, long int value, int lineNo, address funcName);

void __tsan_unaligned_read2(const void *addr);
void __tsan_unaligned_read4(const void *addr);
void __tsan_unaligned_read8(const void *addr);
void __tsan_unaligned_read16(const void *addr);

void __tsan_unaligned_write2(void *addr);
void __tsan_unaligned_write4(void *addr);
void __tsan_unaligned_write8(void *addr);
void __tsan_unaligned_write16(void *addr);

void __tsan_read1_pc(void *addr, void *pc);
void __tsan_read2_pc(void *addr, void *pc);
void __tsan_read4_pc(void *addr, void *pc);
void __tsan_read8_pc(void *addr, void *pc);
void __tsan_read16_pc(void *addr, void *pc);

void __tsan_write1_pc(void *addr, void *pc);
void __tsan_write2_pc(void *addr, void *pc);
void __tsan_write4_pc(void *addr, void *pc);
void __tsan_write8_pc(void *addr, void *pc);
void __tsan_write16_pc(void *addr, void *pc);

void __tsan_vptr_read(void **vptr_p);

void __tsan_vptr_update(void **vptr_p, void *new_val);

void __tsan_func_entry(void *call_pc);
void __tsan_func_exit();

void __tsan_ignore_thread_begin();
void __tsan_ignore_thread_end();

void *__tsan_external_register_tag(const char *object_type);

void __tsan_external_assign_tag(void *addr, void *tag);

void __tsan_external_read(void *addr, void *caller_pc, void *tag);

void __tsan_external_write(void *addr, void *caller_pc, void *tag);


void __tsan_read_range(void *addr, unsigned long size);  // NOLINT

void __tsan_write_range(void *addr, unsigned long size);  // NOLINT

#ifdef __cplusplus
}  // extern "C"
#endif

// These should match declarations from public tsan_interface_atomic.h header.
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

a8 __tsan_atomic8_load(const volatile a8 *a, morder mo);

a16 __tsan_atomic16_load(const volatile a16 *a, morder mo);

a32 __tsan_atomic32_load(const volatile a32 *a, morder mo);

a64 __tsan_atomic64_load(const volatile a64 *a, morder mo);
#if __TSAN_HAS_INT128

a128 __tsan_atomic128_load(const volatile a128 *a, morder mo);
#endif


void __tsan_atomic8_store(volatile a8 *a, a8 v, morder mo);

void __tsan_atomic16_store(volatile a16 *a, a16 v, morder mo);

void __tsan_atomic32_store(volatile a32 *a, a32 v, morder mo);

void __tsan_atomic64_store(volatile a64 *a, a64 v, morder mo);
#if __TSAN_HAS_INT128

void __tsan_atomic128_store(volatile a128 *a, a128 v, morder mo);
#endif


a8 __tsan_atomic8_exchange(volatile a8 *a, a8 v, morder mo);

a16 __tsan_atomic16_exchange(volatile a16 *a, a16 v, morder mo);

a32 __tsan_atomic32_exchange(volatile a32 *a, a32 v, morder mo);

a64 __tsan_atomic64_exchange(volatile a64 *a, a64 v, morder mo);
#if __TSAN_HAS_INT128

a128 __tsan_atomic128_exchange(volatile a128 *a, a128 v, morder mo);
#endif


a8 __tsan_atomic8_fetch_add(volatile a8 *a, a8 v, morder mo);

a16 __tsan_atomic16_fetch_add(volatile a16 *a, a16 v, morder mo);

a32 __tsan_atomic32_fetch_add(volatile a32 *a, a32 v, morder mo);

a64 __tsan_atomic64_fetch_add(volatile a64 *a, a64 v, morder mo);
#if __TSAN_HAS_INT128

a128 __tsan_atomic128_fetch_add(volatile a128 *a, a128 v, morder mo);
#endif


a8 __tsan_atomic8_fetch_sub(volatile a8 *a, a8 v, morder mo);

a16 __tsan_atomic16_fetch_sub(volatile a16 *a, a16 v, morder mo);

a32 __tsan_atomic32_fetch_sub(volatile a32 *a, a32 v, morder mo);

a64 __tsan_atomic64_fetch_sub(volatile a64 *a, a64 v, morder mo);
#if __TSAN_HAS_INT128

a128 __tsan_atomic128_fetch_sub(volatile a128 *a, a128 v, morder mo);
#endif


a8 __tsan_atomic8_fetch_and(volatile a8 *a, a8 v, morder mo);

a16 __tsan_atomic16_fetch_and(volatile a16 *a, a16 v, morder mo);

a32 __tsan_atomic32_fetch_and(volatile a32 *a, a32 v, morder mo);

a64 __tsan_atomic64_fetch_and(volatile a64 *a, a64 v, morder mo);
#if __TSAN_HAS_INT128

a128 __tsan_atomic128_fetch_and(volatile a128 *a, a128 v, morder mo);
#endif


a8 __tsan_atomic8_fetch_or(volatile a8 *a, a8 v, morder mo);

a16 __tsan_atomic16_fetch_or(volatile a16 *a, a16 v, morder mo);

a32 __tsan_atomic32_fetch_or(volatile a32 *a, a32 v, morder mo);

a64 __tsan_atomic64_fetch_or(volatile a64 *a, a64 v, morder mo);
#if __TSAN_HAS_INT128

a128 __tsan_atomic128_fetch_or(volatile a128 *a, a128 v, morder mo);
#endif


a8 __tsan_atomic8_fetch_xor(volatile a8 *a, a8 v, morder mo);

a16 __tsan_atomic16_fetch_xor(volatile a16 *a, a16 v, morder mo);

a32 __tsan_atomic32_fetch_xor(volatile a32 *a, a32 v, morder mo);

a64 __tsan_atomic64_fetch_xor(volatile a64 *a, a64 v, morder mo);
#if __TSAN_HAS_INT128

a128 __tsan_atomic128_fetch_xor(volatile a128 *a, a128 v, morder mo);
#endif


a8 __tsan_atomic8_fetch_nand(volatile a8 *a, a8 v, morder mo);

a16 __tsan_atomic16_fetch_nand(volatile a16 *a, a16 v, morder mo);

a32 __tsan_atomic32_fetch_nand(volatile a32 *a, a32 v, morder mo);

a64 __tsan_atomic64_fetch_nand(volatile a64 *a, a64 v, morder mo);
#if __TSAN_HAS_INT128

a128 __tsan_atomic128_fetch_nand(volatile a128 *a, a128 v, morder mo);
#endif


int __tsan_atomic8_compare_exchange_strong(volatile a8 *a, a8 *c, a8 v,
                                           morder mo, morder fmo);

int __tsan_atomic16_compare_exchange_strong(volatile a16 *a, a16 *c, a16 v,
                                            morder mo, morder fmo);

int __tsan_atomic32_compare_exchange_strong(volatile a32 *a, a32 *c, a32 v,
                                            morder mo, morder fmo);

int __tsan_atomic64_compare_exchange_strong(volatile a64 *a, a64 *c, a64 v,
                                            morder mo, morder fmo);
#if __TSAN_HAS_INT128

int __tsan_atomic128_compare_exchange_strong(volatile a128 *a, a128 *c, a128 v,
                                             morder mo, morder fmo);
#endif


int __tsan_atomic8_compare_exchange_weak(volatile a8 *a, a8 *c, a8 v, morder mo,
                                         morder fmo);

int __tsan_atomic16_compare_exchange_weak(volatile a16 *a, a16 *c, a16 v,
                                          morder mo, morder fmo);

int __tsan_atomic32_compare_exchange_weak(volatile a32 *a, a32 *c, a32 v,
                                          morder mo, morder fmo);

int __tsan_atomic64_compare_exchange_weak(volatile a64 *a, a64 *c, a64 v,
                                          morder mo, morder fmo);
#if __TSAN_HAS_INT128

int __tsan_atomic128_compare_exchange_weak(volatile a128 *a, a128 *c, a128 v,
                                           morder mo, morder fmo);
#endif


a8 __tsan_atomic8_compare_exchange_val(volatile a8 *a, a8 c, a8 v, morder mo,
                                       morder fmo);

a16 __tsan_atomic16_compare_exchange_val(volatile a16 *a, a16 c, a16 v,
                                         morder mo, morder fmo);

a32 __tsan_atomic32_compare_exchange_val(volatile a32 *a, a32 c, a32 v,
                                         morder mo, morder fmo);

a64 __tsan_atomic64_compare_exchange_val(volatile a64 *a, a64 c, a64 v,
                                         morder mo, morder fmo);
#if __TSAN_HAS_INT128

a128 __tsan_atomic128_compare_exchange_val(volatile a128 *a, a128 c, a128 v,
                                           morder mo, morder fmo);
#endif


void __tsan_atomic_thread_fence(morder mo);

void __tsan_atomic_signal_fence(morder mo);

}  // extern "C"

#endif  // TSAN_INTERFACE_H
