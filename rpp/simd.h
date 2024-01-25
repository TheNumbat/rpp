
#pragma once

#include "base.h"

// The implementation of these functions are compiled with SSE on MSVC x86-64 and gcc/clang vector intrinsics on everything else.

namespace rpp::SIMD {

template<i32 N>
struct F32x {
#ifdef RPP_COMPILER_MSVC
    alignas(4 * N) f32 data[N];
#else
    using f32_N = f32 __attribute__((ext_vector_type(N)));
    f32_N data;
#endif

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

#ifdef RPP_COMPILER_MSVC
    [[nodiscard]] static F32x<N> set(f32 a, f32 b, f32 c, f32 d) noexcept;
    [[nodiscard]] static F32x<N> set(f32 a, f32 b, f32 c, f32 d, f32 e, f32 f, f32 g, f32 h) noexcept;
#else
    template<typename... Args>
        requires All_Are<f32, Args...>
    [[nodiscard]] static F32x<N> set(Args... args) noexcept {
        return {(f32_N){args...}};
    }
#endif
};

typedef F32x<4> F32x4;
typedef F32x<8> F32x8;

} // namespace rpp::SIMD
