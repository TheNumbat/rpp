
#include "../base.h"

#ifdef RPP_COMPILER_MSVC
#include <intrin.h>
#endif

#include <math.h>

namespace rpp::Math {

f32 cos(f32 v) {
    return ::cosf(v);
}
f64 cos(f64 v) {
    return ::cos(v);
}
f32 sin(f32 v) {
    return ::sinf(v);
}
f64 sin(f64 v) {
    return ::sin(v);
}
f32 tan(f32 v) {
    return ::tanf(v);
}
f64 tan(f64 v) {
    return ::tan(v);
}
f32 acos(f32 v) {
    return ::acosf(v);
}
f64 acos(f64 v) {
    return ::acos(v);
}
f32 asin(f32 v) {
    return ::asinf(v);
}
f64 asin(f64 v) {
    return ::asin(v);
}
f32 atan(f32 v) {
    return ::atanf(v);
}
f64 atan(f64 v) {
    return ::atan(v);
}
f32 atan2(f32 y, f32 x) {
    return ::atan2f(y, x);
}
f64 atan2(f64 y, f64 x) {
    return ::atan2(y, x);
}
f32 hypot(f32 x, f32 y) {
    return ::hypotf(x, y);
}
f64 hypot(f64 x, f64 y) {
    return ::hypot(x, y);
}
f32 pow(f32 x, f32 y) {
    return ::powf(x, y);
}
f64 pow(f64 x, f64 y) {
    return ::pow(x, y);
}
f32 floor(f32 v) {
    return ::floorf(v);
}
f64 floor(f64 v) {
    return ::floor(v);
}
f32 ceil(f32 v) {
    return ::ceilf(v);
}
f64 ceil(f64 v) {
    return ::ceil(v);
}
f32 round(f32 v) {
    return ::roundf(v);
}
f64 round(f64 v) {
    return ::round(v);
}
f32 abs(f32 v) {
    return ::fabsf(v);
}
f64 abs(f64 v) {
    return ::fabs(v);
}
f32 sqrt(f32 v) {
    return ::sqrtf(v);
}
f64 sqrt(f64 v) {
    return ::sqrt(v);
}
f32 sign(f32 v) {
    return v == 0.0f ? 0.0f : v < 0.0f ? -1.0f : 1.0f;
}
f64 sign(f64 v) {
    return v == 0.0 ? 0.0 : v < 0.0 ? -1.0 : 1.0;
}
i32 abs(i32 v) {
    return ::abs(v);
}
i64 abs(i64 v) {
    return ::llabs(v);
}

u32 popcount(u32 val) {
#ifdef RPP_COMPILER_MSVC
    return __popcnt(val);
#else
    return __builtin_popcount(val);
#endif
}

u64 popcount(u64 val) {
#ifdef RPP_COMPILER_MSVC
    return __popcnt64(val);
#else
    return __builtin_popcountll(val);
#endif
}

u32 ctlz(u32 val) {
#ifdef RPP_COMPILER_MSVC
    return __lzcnt(val);
#else
    if(val == 0) return 32;
    return __builtin_clz(val);
#endif
}

u64 ctlz(u64 val) {
#ifdef RPP_COMPILER_MSVC
    return __lzcnt64(val);
#else
    if(val == 0) return 64;
    return __builtin_clzll(val);
#endif
}

u32 log2(u32 val) {
    return 31u - ctlz(val);
}
u64 log2(u64 val) {
    return 63ull - ctlz(val);
}

u32 prev_pow2(u32 val) {
    if(val == 0) return 0;
    return 1u << log2(val);
}
u64 prev_pow2(u64 val) {
    if(val == 0) return 0;
    return 1ull << log2(val);
}

u32 next_pow2(u32 x) {
    u32 prev = prev_pow2(x);
    return x == prev ? x : prev << 1u;
}
u64 next_pow2(u64 x) {
    u64 prev = prev_pow2(x);
    return x == prev ? x : prev << 1ull;
}

} // namespace rpp::Math
