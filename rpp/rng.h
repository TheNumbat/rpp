
#pragma once

#include "base.h"

namespace rpp::RNG {

struct Stream {

    Stream() noexcept {
        seed();
    }
    constexpr Stream(u64 seed) noexcept : state(seed) {
    }

    [[nodiscard]] constexpr u64 operator()() noexcept {
        state = hash(state);
        return state;
    }

    void seed() noexcept {
        state = hash(hash(Thread::this_id()), hash(Log::sys_time()));
    }
    constexpr void seed(u64 seed) noexcept {
        state = seed;
    }

    template<Float F>
    [[nodiscard]] constexpr F unit() noexcept {
        if constexpr(Same<F, f32>) {
            u64 r = operator()() >> 40;
            return r * 1e-24f;
        } else {
            static_assert(Same<F, f64>);
            u64 r = operator()() >> 11;
            return r * 1e-53;
        }
    }

    template<Float F>
    [[nodiscard]] constexpr bool coin_flip(F p) noexcept {
        return unit<F>() < p;
    }

    template<Allocator A, Move_Constructable T>
    constexpr void shuffle(Vec<T, A>& vec) noexcept {
        for(u64 i = 0; i < vec.length() - 1; i++) {
            u64 j = range(i, vec.length());
            swap(vec[i], vec[j]);
        }
    }

    template<Int I>
    [[nodiscard]] constexpr I integer() noexcept {
        return static_cast<I>(operator()());
    }

    template<Int I>
    [[nodiscard]] constexpr I range(I min, I max) noexcept {
        I r = max - min;
        return min + static_cast<I>(operator()()) % r;
    }

private:
    u64 state = 0;
};

} // namespace rpp::RNG
