
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
#endif

#ifdef COMPILER_CLANG
#define FORCE_INLINE __attribute__((always_inline))
#include <new>
#include <stddef.h>
#include <utility>
#endif

#ifdef OS_LINUX
#include <errno.h>
#include <pthread.h>
#include <signal.h>
#endif

#include <cstdio>
#include <cstring>
#include <float.h>
#include <initializer_list>
#include <smmintrin.h>
#include <source_location>
#include <stdint.h>
#include <stdlib.h>
#include <type_traits>

#define null nullptr

namespace rpp {
typedef uint8_t u8;
typedef int8_t i8;
typedef uint16_t u16;
typedef int16_t i16;
typedef uint32_t u32;
typedef int32_t i32;
typedef uint64_t u64;
typedef int64_t i64;

typedef float f32;
typedef double f64;

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
} // namespace rpp

#define RPP_BASE

#include "reflect.h"

#include "math.h"

#include "ref0.h"

#include "alloc0.h"

#include "string0.h"

#include "thread0.h"

#include "log.h"

#include "format.h"

#include "3d_math.h"

#include "ref1.h"

#include "pair.h"

#include "storage.h"

#include "opt.h"

#include "array.h"

#include "vec.h"

#include "queue.h"

#include "string1.h"

#include "box.h"

#include "hash.h"

#include "map.h"

#include "profile.h"

#include "alloc1.h"

#include "rc.h"

// #include "tuple.h"
// #include "variant.h"
// #include "function.h"
// #include "stack.h"
// #include "heap.h"
// #include "rng.h"