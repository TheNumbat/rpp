
#pragma once

#ifndef RPP_BASE
#error "Include base.h instead."
#endif

namespace rpp {

namespace Hash {

constexpr u64 squirrel5(u64 at) {
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

constexpr u64 combine(u64 h1, u64 h2) {
    return squirrel5(h1 + h2);
}

constexpr u64 primitive(char key) {
    return squirrel5(static_cast<u64>(key));
}

template<Int I>
constexpr u64 primitive(I key) {
    return squirrel5(static_cast<u64>(key));
}

inline u64 primitive(f32 key) {
    return primitive(static_cast<u64>(*reinterpret_cast<u32*>(&key)));
}

inline u64 primitive(f64 key) {
    return squirrel5(*reinterpret_cast<u64*>(&key));
}

template<typename T>
constexpr u64 primitive(T* key) {
    return primitive(static_cast<u64>(reinterpret_cast<uintptr_t>(key)));
}

template<typename K>
concept Primitive = requires(K k) {
    { Hash::primitive(k) } -> Same<u64>;
};

} // namespace Hash

template<typename K>
struct Hasher;

template<Hash::Primitive K>
struct Hasher<K> {
    static constexpr u64 hash(const K& key) {
        return Hash::primitive(key);
    }
};

template<typename K>
concept Hashable = requires(K k) {
    { Hasher<typename Decay<K>::type>::hash(k) } -> Same<u64>;
};

template<Hashable T>
constexpr u64 hash(T&& value) {
    return Hasher<typename Decay<T>::type>::hash(std::forward<T>(value));
}

template<Hashable T>
constexpr u64 hash_nonzero(T&& value) {
    return Hasher<typename Decay<T>::type>::hash(std::forward<T>(value)) | 1;
}

#define LOCATION_HASH (::rpp::hash_literal(__FILE__, __LINE__))

template<size_t N>
consteval u64 hash_literal(const char (&literal)[N], u64 seed = 0) {
    for(size_t i = 0; i < N - 1; i++) {
        seed = Hash::combine(seed, hash(literal[i]));
    }
    return seed;
}

} // namespace rpp
