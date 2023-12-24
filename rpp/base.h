
#pragma once

#ifdef _MSC_VER
#define COMPILER_MSVC
#elif defined __clang__
#define COMPILER_CLANG
#else
#error "Unsupported compiler."
#endif

#ifdef _WIN64
#define OS_WINDOWS
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#elif defined __linux__
#define OS_LINUX
#else
#error "Unsupported OS."
#endif

#if defined __x86_64__ || defined _M_X64
#define ARCH_X64
#else
#error "Unsupported architecture."
#endif

#ifdef COMPILER_MSVC
#define FORCE_INLINE __forceinline
#define MSVC_INTRINSIC [[msvc::intrinsic]]
#else
#define MSVC_INTRINSIC
#endif

#ifdef COMPILER_CLANG
#define FORCE_INLINE __attribute__((always_inline))
#include <new>
#endif

#ifdef OS_LINUX
#include <pthread.h>
#endif

#define null nullptr

namespace rpp {

typedef signed char i8;
typedef short i16;
typedef int i32;
typedef long long i64;
typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef float f32;
typedef double f64;

#ifdef COMPILER_MSVC
typedef unsigned __int64 uptr;
typedef unsigned long long u64;
#else
typedef __UINTPTR_TYPE__ uptr;
typedef unsigned long u64;
#endif

static_assert(sizeof(i8) == 1);
static_assert(sizeof(i16) == 2);
static_assert(sizeof(i32) == 4);
static_assert(sizeof(i64) == 8);
static_assert(sizeof(u8) == 1);
static_assert(sizeof(u16) == 2);
static_assert(sizeof(u32) == 4);
static_assert(sizeof(u64) == 8);
static_assert(sizeof(f32) == 4);
static_assert(sizeof(f64) == 8);
static_assert(sizeof(char) == 1);
static_assert(sizeof(bool) == 1);
static_assert(sizeof(void*) == 8);
static_assert(sizeof(uptr) == 8);

namespace Libc {

[[noreturn]] void exit(i32 code);
i32 strncmp(const char* a, const char* b, u64 bytes);
u64 strlen(const char* str);
void* memset(void* dest, i32 value, u64 bytes);
void* memcpy(void* dest, const void* src, u64 bytes);
i32 memcmp(const void* a, const void* b, u64 bytes);
i32 snprintf(u8* buffer, u64 buffer_size, const char* fmt, ...);
i64 strtoll(const char* str, char** endptr, i32 base);
f32 strtof(const char* str, char** endptr);

} // namespace Libc

} // namespace rpp

#define RPP_BASE

#include "std/initializer_list.h"

#include "utility.h"

#include "limits.h"

#include "reflect.h"

#include "hash.h"

#include "math.h"

#include "ref0.h"

#include "alloc0.h"

#include "string0.h"

#include "thread0.h"

#include "log.h"

#include "format.h"

#include "ref1.h"

#include "pair.h"

#include "storage.h"

#include "opt.h"

#include "array.h"

#include "vec.h"

#include "queue.h"

#include "string1.h"

#include "box.h"

#include "map.h"

#include "profile.h"

#include "alloc1.h"

#include "format1.h"
