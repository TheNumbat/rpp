
#pragma once

#include "base.h"

// The implementation of these functions are compiled with gcc/clang vector intrinsics.

namespace rpp::SIMD {

template<i32 T>
struct F32x {
    alignas(4 * T) f32 data[T];
    typedef f32 floatT __attribute__((ext_vector_type(T)));

    [[nodiscard]] static F32x<T> set1(f32 v) noexcept;
    [[nodiscard]] static F32x<T> zero() noexcept;
    [[nodiscard]] static F32x<T> one() noexcept;
    [[nodiscard]] static F32x<T> add(F32x<T> a, F32x<T> b) noexcept;
    [[nodiscard]] static F32x<T> sub(F32x<T> a, F32x<T> b) noexcept;
    [[nodiscard]] static F32x<T> mul(F32x<T> a, F32x<T> b) noexcept;
    [[nodiscard]] static F32x<T> div(F32x<T> a, F32x<T> b) noexcept;
    [[nodiscard]] static F32x<T> min(F32x<T> a, F32x<T> b) noexcept;
    [[nodiscard]] static F32x<T> max(F32x<T> a, F32x<T> b) noexcept;
    [[nodiscard]] static F32x<T> floor(F32x<T> a) noexcept;
    [[nodiscard]] static F32x<T> ceil(F32x<T> a) noexcept;
    [[nodiscard]] static F32x<T> abs(F32x<T> a) noexcept;
    [[nodiscard]] static f32 dp(F32x<T> a, F32x<T> b) noexcept;
    [[nodiscard]] static F32x<T> cmpeq(F32x<T> a, F32x<T> b) noexcept;
    [[nodiscard]] static i32 movemask(F32x<T> a) noexcept;

    template<typename... Args> // Variadic function for setting the parameters
    [[nodiscard]] static F32x<T> set(Args... args) noexcept {
        return to((floatT){args...});
    }
};

typedef F32x<4> F32x4;
typedef F32x<8> F32x8;

} // namespace rpp::SIMD
