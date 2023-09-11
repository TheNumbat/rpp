
#pragma once

namespace rpp::Math {

constexpr f32 EPS_F = (16.0f * std::numeric_limits<f32>::epsilon());
constexpr f32 PI32 = 3.14159265358979323846264338327950288f;
constexpr f64 PI64 = 3.14159265358979323846264338327950288;

template<typename T>
constexpr T min(T a, T b) {
    return a < b ? a : b;
}

template<typename T>
constexpr T max(T a, T b) {
    return a > b ? a : b;
}

constexpr u64 KB(u64 n) {
    return n * 1024;
}
constexpr u64 MB(u64 n) {
    return n * KB(1024);
}
constexpr u64 GB(u64 n) {
    return n * MB(1024);
}

template<typename T>
constexpr T radians(T v) {
    return v * T{PI32 / 180.0f};
}
template<typename T>
constexpr T degrees(T v) {
    return v * T{180.0f / PI32};
}

inline f32 cos(f32 v) {
    return ::cosf(v);
}
inline f64 cos(f64 v) {
    return ::cos(v);
}
inline f32 sin(f32 v) {
    return ::sinf(v);
}
inline f64 sin(f64 v) {
    return ::sin(v);
}
inline f32 tan(f32 v) {
    return ::tanf(v);
}
inline f64 tan(f64 v) {
    return ::tan(v);
}
inline f32 acos(f32 v) {
    return ::acosf(v);
}
inline f64 acos(f64 v) {
    return ::acos(v);
}
inline f32 asin(f32 v) {
    return ::asinf(v);
}
inline f64 asin(f64 v) {
    return ::asin(v);
}
inline f32 atan(f32 v) {
    return ::atanf(v);
}
inline f64 atan(f64 v) {
    return ::atan(v);
}
inline f32 atan2(f32 y, f32 x) {
    return ::atan2f(y, x);
}
inline f64 atan2(f64 y, f64 x) {
    return ::atan2(y, x);
}
inline f32 hypot(f32 x, f32 y) {
    return ::hypotf(x, y);
}
inline f64 hypot(f64 x, f64 y) {
    return ::hypot(x, y);
}
inline f32 floor(f32 v) {
    return ::floorf(v);
}
inline f64 floor(f64 v) {
    return ::floor(v);
}
inline f32 ceil(f32 v) {
    return ::ceilf(v);
}
inline f64 ceil(f64 v) {
    return ::ceil(v);
}
inline f32 abs(f32 v) {
    return ::fabsf(v);
}
inline f64 abs(f64 v) {
    return ::fabs(v);
}
inline f32 sqrt(f32 v) {
    return ::sqrtf(v);
}
inline f64 sqrt(f64 v) {
    return ::sqrt(v);
}
inline i32 abs(i32 v) {
    return ::abs(v);
}
inline i64 abs(i64 v) {
    return ::abs(v);
}

inline u32 ctlz(u32 val) {
#ifdef COMPILER_MSVC
    return __lzcnt(val);
#else
    if(val == 0) return 32;
    return __builtin_clz(val);
#endif
}

inline u64 ctlz(u64 val) {
#ifdef COMPILER_MSVC
    return __lzcnt64(val);
#else
    if(val == 0) return 64;
    return __builtin_clzll(val);
#endif
}

inline u32 log2(u32 val) {
    return 31u - ctlz(val);
}
inline u64 log2(u64 val) {
    return 63ull - ctlz(val);
}

inline u32 prev_pow2(u32 val) {
    if(val == 0) return 0;
    return 1u << log2(val);
}
inline u64 prev_pow2(u64 val) {
    if(val == 0) return 0;
    return 1ull << log2(val);
}

inline u32 next_pow2(u32 x) {
    u32 prev = prev_pow2(x);
    return x == prev ? x : prev << 1u;
}
inline u64 next_pow2(u64 x) {
    u64 prev = prev_pow2(x);
    return x == prev ? x : prev << 1ull;
}

template<typename T>
constexpr T lerp(T start, T end, T t) {
    return start + (end - start) * t;
}

template<typename T>
constexpr T clamp(T x, T start, T end) {
    return min(max(x, start), end);
}

constexpr f32 frac(f32 x) {
    return x - (i64)x;
}

constexpr f32 smoothstep(f32 e0, f32 e1, f32 x) {
    f32 t = clamp((x - e0) / (e1 - e0), 0.0f, 1.0f);
    return t * t * (3.0f - 2.0f * t);
}

constexpr u32 align_down_pow2(u32 x, u32 align) {
    return x & ~(align - 1);
}
constexpr u64 align_down_pow2(u64 x, u64 align) {
    return x & ~(align - 1);
}

constexpr u32 align_down(u32 x, u32 align) {
    u32 r = x % align;
    return r == 0 ? x : x - r;
}
constexpr u64 align_down(u64 x, u64 align) {
    u64 r = x % align;
    return r == 0 ? x : x - r;
}

constexpr u32 align_pow2(u32 x, u32 align) {
    return (x + align - 1) & ~(align - 1);
}
constexpr u64 align_pow2(u64 x, u64 align) {
    return (x + align - 1) & ~(align - 1);
}

constexpr u32 align(u32 x, u32 align) {
    u32 r = x % align;
    return r == 0 ? x : x + (align - r);
}
constexpr u64 align(u64 x, u64 align) {
    u64 r = x % align;
    return r == 0 ? x : x + (align - r);
}

} // namespace rpp::Math
