
#pragma once

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
        state = Hash::combine(hash(Thread::this_id()), hash(std::time(null)));
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

    template<Movable T>
    void shuffle(Slice<T> arr) {
        for(u64 i = 0; i < arr.size() - 1; i++) {
            u64 j = range(i, arr.size());
            std::swap(arr[i], arr[j]);
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
