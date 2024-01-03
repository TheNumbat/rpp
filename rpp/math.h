
#pragma once

#ifndef RPP_BASE
#error "Include base.h instead."
#endif

namespace rpp::Math {

constexpr f32 EPS_F = (16.0f * Limits<f32>::epsilon());
constexpr f32 PI32 = 3.141592653589793f;
constexpr f64 PI64 = 3.141592653589793;
constexpr f32 PHI32 = 1.618033988749895f;
constexpr f64 PHI64 = 1.618033988749895;

template<typename T>
[[nodiscard]] constexpr T min(T a, T b) noexcept {
    return a < b ? a : b;
}

template<typename T>
[[nodiscard]] constexpr T max(T a, T b) noexcept {
    return a > b ? a : b;
}

template<typename T>
[[nodiscard]] constexpr T min(std::initializer_list<T> list) noexcept {
    T result = Limits<T>::max();
    for(T v : list) {
        result = min(result, v);
    }
    return result;
}

template<typename T>
[[nodiscard]] constexpr T max(std::initializer_list<T> list) noexcept {
    T result = Limits<T>::min();
    for(T v : list) {
        result = max(result, v);
    }
    return result;
}

[[nodiscard]] constexpr u64 KB(u64 n) noexcept {
    return n * 1024;
}
[[nodiscard]] constexpr u64 MB(u64 n) noexcept {
    return n * KB(1024);
}
[[nodiscard]] constexpr u64 GB(u64 n) noexcept {
    return n * MB(1024);
}

template<typename T>
[[nodiscard]] constexpr T radians(T v) noexcept {
    return v * T{PI32 / 180.0f};
}
template<typename T>
[[nodiscard]] constexpr T degrees(T v) noexcept {
    return v * T{180.0f / PI32};
}

template<Int I>
[[nodiscard]] constexpr I pow(I base, I exp) noexcept {
    I result = 1;
    while(exp) {
        if(exp & 1) {
            result *= base;
        }
        exp >>= 1;
        base *= base;
    }
    return result;
}

[[nodiscard]] f32 cos(f32 v) noexcept;
[[nodiscard]] f64 cos(f64 v) noexcept;
[[nodiscard]] f32 sin(f32 v) noexcept;
[[nodiscard]] f64 sin(f64 v) noexcept;
[[nodiscard]] f32 tan(f32 v) noexcept;
[[nodiscard]] f64 tan(f64 v) noexcept;
[[nodiscard]] f32 acos(f32 v) noexcept;
[[nodiscard]] f64 acos(f64 v) noexcept;
[[nodiscard]] f32 asin(f32 v) noexcept;
[[nodiscard]] f64 asin(f64 v) noexcept;
[[nodiscard]] f32 atan(f32 v) noexcept;
[[nodiscard]] f64 atan(f64 v) noexcept;
[[nodiscard]] f32 atan2(f32 y, f32 x) noexcept;
[[nodiscard]] f64 atan2(f64 y, f64 x) noexcept;
[[nodiscard]] f32 hypot(f32 x, f32 y) noexcept;
[[nodiscard]] f64 hypot(f64 x, f64 y) noexcept;
[[nodiscard]] f32 pow(f32 x, f32 y) noexcept;
[[nodiscard]] f64 pow(f64 x, f64 y) noexcept;
[[nodiscard]] f32 floor(f32 v) noexcept;
[[nodiscard]] f64 floor(f64 v) noexcept;
[[nodiscard]] f32 ceil(f32 v) noexcept;
[[nodiscard]] f64 ceil(f64 v) noexcept;
[[nodiscard]] f32 round(f32 v) noexcept;
[[nodiscard]] f64 round(f64 v) noexcept;
[[nodiscard]] f32 abs(f32 v) noexcept;
[[nodiscard]] f64 abs(f64 v) noexcept;
[[nodiscard]] f32 sqrt(f32 v) noexcept;
[[nodiscard]] f64 sqrt(f64 v) noexcept;
[[nodiscard]] f32 sign(f32 v) noexcept;
[[nodiscard]] f64 sign(f64 v) noexcept;
[[nodiscard]] i32 abs(i32 v) noexcept;
[[nodiscard]] i64 abs(i64 v) noexcept;

[[nodiscard]] u32 popcount(u32 val) noexcept;
[[nodiscard]] u64 popcount(u64 val) noexcept;
[[nodiscard]] u32 ctlz(u32 val) noexcept;
[[nodiscard]] u64 ctlz(u64 val) noexcept;
[[nodiscard]] u32 log2(u32 val) noexcept;
[[nodiscard]] u64 log2(u64 val) noexcept;
[[nodiscard]] u32 prev_pow2(u32 val) noexcept;
[[nodiscard]] u64 prev_pow2(u64 val) noexcept;
[[nodiscard]] u32 next_pow2(u32 x) noexcept;
[[nodiscard]] u64 next_pow2(u64 x) noexcept;

template<typename T>
[[nodiscard]] constexpr T lerp(T start, T end, T t) noexcept {
    return start + (end - start) * t;
}

template<typename T>
[[nodiscard]] constexpr T clamp(T x, T start, T end) noexcept {
    return min(max(x, start), end);
}

[[nodiscard]] constexpr f32 frac(f32 x) noexcept {
    return x - (i64)x;
}

[[nodiscard]] constexpr f32 smoothstep(f32 e0, f32 e1, f32 x) noexcept {
    f32 t = clamp((x - e0) / (e1 - e0), 0.0f, 1.0f);
    return t * t * (3.0f - 2.0f * t);
}

[[nodiscard]] constexpr u32 align_down_pow2(u32 x, u32 align) noexcept {
    return x & ~(align - 1);
}
[[nodiscard]] constexpr u64 align_down_pow2(u64 x, u64 align) noexcept {
    return x & ~(align - 1);
}

[[nodiscard]] constexpr u32 align_down(u32 x, u32 align) noexcept {
    u32 r = x % align;
    return r == 0 ? x : x - r;
}
[[nodiscard]] constexpr u64 align_down(u64 x, u64 align) noexcept {
    u64 r = x % align;
    return r == 0 ? x : x - r;
}

[[nodiscard]] constexpr u32 align_pow2(u32 x, u32 align) noexcept {
    return (x + align - 1) & ~(align - 1);
}
[[nodiscard]] constexpr u64 align_pow2(u64 x, u64 align) noexcept {
    return (x + align - 1) & ~(align - 1);
}

[[nodiscard]] constexpr u32 align(u32 x, u32 align) noexcept {
    u32 r = x % align;
    return r == 0 ? x : x + (align - r);
}
[[nodiscard]] constexpr u64 align(u64 x, u64 align) noexcept {
    u64 r = x % align;
    return r == 0 ? x : x + (align - r);
}

} // namespace rpp::Math
