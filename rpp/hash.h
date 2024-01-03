
#pragma once

#ifndef RPP_BASE
#error "Include base.h instead."
#endif

#define LOCATION_HASH ::rpp::hash_literal(__FILE__, __LINE__)

namespace rpp {

namespace Hash {

[[nodiscard]] constexpr u64 squirrel5(u64 at) noexcept {
    constexpr u64 BIT_NOISE1 = 0xA278032FB08BA40Dul;
    constexpr u64 BIT_NOISE2 = 0x9D9FDC30FD876B1Dul;
    constexpr u64 BIT_NOISE3 = 0xEC705118C5FBDA13ul;
    constexpr u64 BIT_NOISE4 = 0xDB8BDB77D7DF9811ul;
    constexpr u64 BIT_NOISE5 = 0xD8081C73F0FAA127ul;
    at *= BIT_NOISE1;
    at ^= (at >> 9);
    at += BIT_NOISE2;
    at ^= (at >> 11);
    at *= BIT_NOISE3;
    at ^= (at >> 13);
    at += BIT_NOISE4;
    at ^= (at >> 15);
    at *= BIT_NOISE5;
    at ^= (at >> 17);
    return at;
}

[[nodiscard]] constexpr u64 hash_combine(u64 h1, u64 h2) noexcept {
    return squirrel5(h1 + h2);
}

template<typename K>
struct Hash;

template<Int I>
struct Hash<I> {
    [[nodiscard]] constexpr static u64 hash(I key) noexcept {
        return squirrel5(static_cast<u64>(key));
    }
};

template<>
struct Hash<char> {
    [[nodiscard]] constexpr static u64 hash(char key) noexcept {
        return squirrel5(static_cast<u64>(key));
    }
};

template<>
struct Hash<f32> {
    [[nodiscard]] static u64 hash(f32 key) noexcept {
        return squirrel5(static_cast<u64>(*reinterpret_cast<u32*>(&key)));
    }
};

template<>
struct Hash<f64> {
    [[nodiscard]] static u64 hash(f64 key) noexcept {
        return squirrel5(*reinterpret_cast<u64*>(&key));
    }
};

template<typename T>
struct Hash<T*> {
    [[nodiscard]] constexpr static u64 hash(T* key) noexcept {
        return squirrel5(static_cast<u64>(reinterpret_cast<uptr>(key)));
    }
};

} // namespace Hash

template<typename K>
concept Hashable = requires(K k) {
    { Hash::Hash<Decay<K>>::hash(k) } -> Same<u64>;
};

template<Hashable T>
[[nodiscard]] constexpr u64 hash(T&& value) noexcept {
    return Hash::Hash<Decay<T>>::hash(forward<T>(value));
}

template<Hashable T>
[[nodiscard]] constexpr u64 hash_nonzero(T&& value) noexcept {
    return Hash::Hash<Decay<T>>::hash(forward<T>(value)) | 1;
}

template<Hashable... Ts>
[[nodiscard]] constexpr u64 hash(Ts&&... values) noexcept {
    return Hash::squirrel5((hash(forward<Ts>(values)) + ...));
}

template<size_t N>
[[nodiscard]] consteval u64 hash_literal(const char (&literal)[N], u64 seed = 0) noexcept {
    for(size_t i = 0; i < N - 1; i++) {
        seed = Hash::hash_combine(seed, hash(literal[i]));
    }
    return seed ? seed : 1;
}

} // namespace rpp
