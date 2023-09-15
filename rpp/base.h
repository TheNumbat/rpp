
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
#elif defined __linux__ && defined __x86_64__
#define OS_LINUX
#else
#error "Unsupported OS."
#endif

#ifdef COMPILER_MSVC
#define FORCE_INLINE __forceinline
#include <intrin.h>
#endif

#ifdef COMPILER_CLANG
#define FORCE_INLINE __attribute__((always_inline))
#include <x86intrin.h>
#endif

#ifdef OS_LINUX
#include <pthread.h>
#include <signal.h>
#endif

#include <cerrno>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <limits>
#include <source_location>
#include <tuple>
#include <utility>

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
} // namespace rpp

#include "reflect.h"

#include "math.h"

#include "3d_math.h"

#include "ref0.h"

#include "alloc0.h"

#include "string0.h"

#include "thread0.h"

#include "log.h"

#include "ref1.h"

#include "pair.h"

#include "storage.h"

#include "opt.h"

#include "array.h"

#include "vec.h"

#include "alloc1.h"

#include "string1.h"

#include "box.h"

#include "format.h"

#include "stack.h"

#include "queue.h"

#include "heap.h"

#include "hash.h"

#include "map.h"
