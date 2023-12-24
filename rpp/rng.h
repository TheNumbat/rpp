
#pragma once

#include "base.h"

namespace rpp::RNG {

struct Stream {

    Stream() {
        seed();
    }
    Stream(u64 seed) : state(seed) {
    }

    u64 operator()() {
        state = hash(state);
        return state;
    }

    void seed() {
        state = hash(hash(Thread::this_id()), hash(Log::sys_time()));
    }
    void seed(u64 seed) {
        state = seed;
    }

    template<Float F>
    F unit() {
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
    bool coin_flip(F p) {
        return unit<F>() < p;
    }

    template<Allocator A, Movable T>
    void shuffle(Vec<T, A>& vec) {
        for(u64 i = 0; i < vec.length() - 1; i++) {
            u64 j = range(i, vec.length());
            swap(vec[i], vec[j]);
        }
    }

    template<Int I>
    I integer() {
        return static_cast<I>(operator()());
    }

    template<Int I>
    I range(I min, I max) {
        I r = max - min;
        return min + static_cast<I>(operator()()) % r;
    }

private:
    u64 state = 0;
};

} // namespace rpp::RNG
