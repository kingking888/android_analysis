#pragma once

#include <jni.h>

#if defined(__arm64__) || defined(__aarch64__)
#define REG_COUNT       8
#define REG_BIT_SIZE    8
#define REG_ID(i) x##i
#else
#define REG_ID(i) r##i
#define REG_COUNT       4
#define REG_BIT_SIZE    4
#endif

#define ALIGN_FLOOR(type, address, range) ((type)address & ~((type)range - 1))
#define MAKE_1BIT(type, x) ( ( (type)~0)  >> (sizeof(type)*8 - x))
#define GET_BIT(type, b, offset, size)  (((type)b >> offset) & MAKE_1BIT(type,size))


#if defined(__arm64__) || defined(__aarch64__)
#define SET_MEMORY_PROTECT(addr, protect) mprotect((void *)  ALIGN_FLOOR(uint64_t, addr, sysconf(_SC_PAGESIZE)), sysconf(_SC_PAGESIZE),protect)

extern "C" JNIEXPORT int64_t JNICALL
route_jni_func(int64_t x0,
               int64_t x1,
               int64_t x2,
               int64_t x3,
               int64_t x4,
               int64_t x5,
               int64_t x6,
               int64_t x7,
               int64_t argc, //0x0
               void *stack_params_start, //0x8
               void *pcall);//0x10

using new_pfunc_t = int64_t(*)(int64_t x0,
                               int64_t x1,
                               int64_t x2,
                               int64_t x3,
                               int64_t x4,
                               int64_t x5,
                               int64_t x6,
                               int64_t x7,
                               void *context,
                               void *stack_args);

#else
#define SET_MEMORY_PROTECT(addr, protect) mprotect((void *)  ALIGN_FLOOR(uint32_t, addr, sysconf(_SC_PAGESIZE)), sysconf(_SC_PAGESIZE),protect)

extern "C" JNIEXPORT int32_t
route_jni_func(uint32_t r0,
               uint32_t r1,
               uint32_t r2,
               uint32_t r3,
               int32_t argc, //0x0
               void *stack_params_start, //0x8
               void *pcall);//0xc

using new_pfunc_t = uint32_t(*)(uint32_t r0,
                                uint32_t r1,
                                uint32_t r2,
                                uint32_t r3,
                                void *context,
                                void *stack_args);
#endif

unsigned char *create_shellcode_hooking_func(new_pfunc_t new_pfunc, void *context);
