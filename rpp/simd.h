
#pragma once

#include "base.h"

// The implementation of these functions are compiled with SSE on MSVC x86-64 and gcc/clang vector
// intrinsics on everything else.

namespace rpp::SIMD {

#ifdef RPP_COMPILER_MSVC

struct F32x4 {
    alignas(16) f32 data[4];

    [[nodiscard]] static F32x4 set1(f32 v) noexcept;
    [[nodiscard]] static F32x4 set(f32 x, f32 y, f32 z, f32 w) noexcept;
    [[nodiscard]] static F32x4 zero() noexcept;
    [[nodiscard]] static F32x4 one() noexcept;
    [[nodiscard]] static F32x4 add(F32x4 a, F32x4 b) noexcept;
    [[nodiscard]] static F32x4 sub(F32x4 a, F32x4 b) noexcept;
    [[nodiscard]] static F32x4 mul(F32x4 a, F32x4 b) noexcept;
    [[nodiscard]] static F32x4 div(F32x4 a, F32x4 b) noexcept;
    [[nodiscard]] static F32x4 min(F32x4 a, F32x4 b) noexcept;
    [[nodiscard]] static F32x4 max(F32x4 a, F32x4 b) noexcept;
    [[nodiscard]] static F32x4 floor(F32x4 a) noexcept;
    [[nodiscard]] static F32x4 ceil(F32x4 a) noexcept;
    [[nodiscard]] static F32x4 abs(F32x4 a) noexcept;
    [[nodiscard]] static f32 dp(F32x4 a, F32x4 b) noexcept;
    [[nodiscard]] static i32 cmpeq(F32x4 a, F32x4 b) noexcept;
};

#else

template<i32 N>
struct F32x {
    using f32_N = f32 __attribute__((ext_vector_type(N)));
    alignas(4 * N) f32_N data;

    constexpr F32x<N>(f32_N in = {}) : data(in){};

    [[nodiscard]] static F32x<N> set1(f32 v) noexcept;
    [[nodiscard]] static F32x<N> zero() noexcept;
    [[nodiscard]] static F32x<N> one() noexcept;
    [[nodiscard]] static F32x<N> add(F32x<N> a, F32x<N> b) noexcept;
    [[nodiscard]] static F32x<N> sub(F32x<N> a, F32x<N> b) noexcept;
    [[nodiscard]] static F32x<N> mul(F32x<N> a, F32x<N> b) noexcept;
    [[nodiscard]] static F32x<N> div(F32x<N> a, F32x<N> b) noexcept;
    [[nodiscard]] static F32x<N> min(F32x<N> a, F32x<N> b) noexcept;
    [[nodiscard]] static F32x<N> max(F32x<N> a, F32x<N> b) noexcept;
    [[nodiscard]] static F32x<N> floor(F32x<N> a) noexcept;
    [[nodiscard]] static F32x<N> ceil(F32x<N> a) noexcept;
    [[nodiscard]] static F32x<N> abs(F32x<N> a) noexcept;
    [[nodiscard]] static f32 dp(F32x<N> a, F32x<N> b) noexcept;
    [[nodiscard]] static i32 cmpeq(F32x<N> a, F32x<N> b) noexcept;

    template<typename... Args>
        requires All_Are<f32, Args...>
    [[nodiscard]] static F32x<N> set(Args... args) noexcept {
        return {(f32_N){args...}};
    }
};

// by being templated, the F32x<N> class can easily extend to more sizes!
typedef F32x<4> F32x4;
typedef F32x<8> F32x8;

#endif

} // namespace rpp::SIMD
