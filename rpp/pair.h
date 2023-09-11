
#pragma once

namespace rpp {

template<typename A, typename B>
struct Pair {

    Pair()
        requires Default_Constructable<A> && Default_Constructable<B>
    = default;

    explicit Pair(A&& first, B&& second)
        requires Move_Constructable<A> && Move_Constructable<B>
        : first(std::move(first)), second(std::move(second)) {
    }

    ~Pair() = default;

    Pair(const Pair& src)
        requires Trivial<A> && Trivial<B>
    = default;
    Pair& operator=(const Pair& src)
        requires Trivial<A> && Trivial<B>
    = default;

    Pair(Pair&& src) = default;
    Pair& operator=(Pair&& src) = default;

    Pair<A, B> clone() const
        requires Clone<A> && Clone<B>
    {
        return Pair<A, B>{first.clone(), second.clone()};
    }

    A first;
    B second;
};

template<typename A, typename B>
struct Reflect<Pair<A, B>> {
    using T = Pair<A, B>;
    static constexpr Literal name = "Pair";
    static constexpr Kind kind = Kind::record_;
    using members = List<FIELD(first), FIELD(second)>;
    static_assert(Record<T>);
};

} // namespace rpp

namespace std {

using rpp::Pair;

template<typename A, typename B>
struct tuple_size<Pair<A, B>> {
    static constexpr size_t value = 2;
};
template<typename A, typename B>
struct tuple_element<0, Pair<A, B>> {
    using type = A;
};
template<typename A, typename B>
struct tuple_element<1, Pair<A, B>> {
    using type = B;
};

template<size_t Index, typename A, typename B>
tuple_element_t<Index, Pair<A, B>>& get(Pair<A, B>& p) {
    if constexpr(Index == 0) return p.first;
    if constexpr(Index == 1) return p.second;
}
template<size_t Index, typename A, typename B>
const tuple_element_t<Index, Pair<A, B>>& get(const Pair<A, B>& p) {
    if constexpr(Index == 0) return p.first;
    if constexpr(Index == 1) return p.second;
}
template<size_t Index, typename A, typename B>
tuple_element_t<Index, Pair<A, B>>&& get(Pair<A, B>&& p) {
    if constexpr(Index == 0) return move(p.first);
    if constexpr(Index == 1) return move(p.second);
}
template<size_t Index, typename A, typename B>
const tuple_element_t<Index, Pair<A, B>>&& get(const Pair<A, B>&& p) {
    if constexpr(Index == 0) return move(p.first);
    if constexpr(Index == 1) return move(p.second);
}

} // namespace std
