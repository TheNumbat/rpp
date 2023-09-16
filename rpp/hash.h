
#pragma once

namespace rpp {

namespace Hash {

inline u64 squirrel5(u64 at) {
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

inline u64 combine(u64 h1, u64 h2) {
    return squirrel5(h1 + h2);
}

template<Int I>
u64 hash(I key) {
    return squirrel5(static_cast<u64>(key));
}

inline u64 hash(f32 key) {
    return hash(static_cast<u64>(*reinterpret_cast<u32*>(&key)));
}

inline u64 hash(f64 key) {
    return squirrel5(*reinterpret_cast<u64*>(&key));
}

template<typename T>
u64 hash(T* key) {
    return hash(static_cast<u64>(reinterpret_cast<uintptr_t>(key)));
}

template<Allocator A = Mdefault>
u64 hash(const String<A>& string) {
    u64 h = 0;
    for(u8 c : string) h = combine(h, hash(static_cast<u64>(c)));
    return h;
}

inline u64 hash(const String_View& string) {
    u64 h = 0;
    for(u8 c : string) h = combine(h, hash(static_cast<u64>(c)));
    return h;
}

inline u64 hash(Log::Location l) {
    return combine(combine(hash(l.file), hash(l.function)), combine(hash(l.line), hash(l.column)));
}

template<typename T, u64 N>
u64 hash(Math::Vect<T, N> v) {
    u64 h = 0;
    for(u64 i = 0; i < N; i++) h = combine(h, hash(v[i]));
    return h;
}

} // namespace Hash

template<typename K>
concept Hashable = requires(K k) {
    { Hash::hash(k) } -> Same<u64>;
};

template<Hashable T>
u64 hash(T&& value) {
    return Hash::hash(std::forward<T>(value));
}

template<Hashable T>
u64 hash_nonzero(T&& value) {
    return Hash::hash(std::forward<T>(value)) | 1;
}

} // namespace rpp
