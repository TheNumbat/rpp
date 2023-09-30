
#ifdef COMPILER_MSVC
#include <intrin.h>
#endif

#include "base.h"
#include <cmath>

namespace rpp::Math {

f32 cos(f32 v) {
    return std::cos(v);
}
f64 cos(f64 v) {
    return std::cos(v);
}
f32 sin(f32 v) {
    return std::sin(v);
}
f64 sin(f64 v) {
    return std::sin(v);
}
f32 tan(f32 v) {
    return std::tan(v);
}
f64 tan(f64 v) {
    return std::tan(v);
}
f32 acos(f32 v) {
    return std::acos(v);
}
f64 acos(f64 v) {
    return std::acos(v);
}
f32 asin(f32 v) {
    return std::asin(v);
}
f64 asin(f64 v) {
    return std::asin(v);
}
f32 atan(f32 v) {
    return std::atan(v);
}
f64 atan(f64 v) {
    return std::atan(v);
}
f32 atan2(f32 y, f32 x) {
    return std::atan2(y, x);
}
f64 atan2(f64 y, f64 x) {
    return std::atan2(y, x);
}
f32 hypot(f32 x, f32 y) {
    return std::hypot(x, y);
}
f64 hypot(f64 x, f64 y) {
    return std::hypot(x, y);
}
f32 floor(f32 v) {
    return std::floor(v);
}
f64 floor(f64 v) {
    return std::floor(v);
}
f32 ceil(f32 v) {
    return std::ceil(v);
}
f64 ceil(f64 v) {
    return std::ceil(v);
}
f32 abs(f32 v) {
    return std::abs(v);
}
f64 abs(f64 v) {
    return std::abs(v);
}
f32 sqrt(f32 v) {
    return std::sqrt(v);
}
f64 sqrt(f64 v) {
    return std::sqrt(v);
}
i32 abs(i32 v) {
    return std::abs(v);
}
i64 abs(i64 v) {
    return std::abs(v);
}

u32 ctlz(u32 val) {
#ifdef COMPILER_MSVC
    return __lzcnt(val);
#else
    if(val == 0) return 32;
    return __builtin_clz(val);
#endif
}

u64 ctlz(u64 val) {
#ifdef COMPILER_MSVC
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
