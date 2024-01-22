
#pragma once

#include "base.h"

// The implementation of these functions are compiled with gcc/clang vector intrinsics.

namespace rpp::SIMD {

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

typedef F32x<4> F32x4;
typedef F32x<8> F32x8;

} // namespace rpp::SIMD
