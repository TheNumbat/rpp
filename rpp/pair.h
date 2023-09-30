
#pragma once

namespace rpp {

template<typename A, typename B>
struct Pair {

    Pair()
        requires Default_Constructable<A> && Default_Constructable<B>
    = default;

    explicit Pair(const A& first, const B& second)
        requires Trivial<A> && Trivial<B>
        : first(A{first}), second(B{second}) {
    }

    explicit Pair(A&& first, B&& second)
        requires Move_Constructable<A> && Move_Constructable<B>
        : first(std::move(first)), second(std::move(second)) {
    }

    explicit Pair(const A& first, B&& second)
        requires Trivial<A> && Move_Constructable<B>
        : first(A{first}), second(std::move(second)) {
    }

    explicit Pair(A&& first, const B& second)
        requires Move_Constructable<A> && Trivial<B>
        : first(std::move(first)), second(B{second}) {
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
        requires(Clone<A> || Trivial<A>) && (Clone<B> || Trivial<B>)
    {
        if constexpr(Clone<A> && Clone<B>)
            return Pair<A, B>{first.clone(), second.clone()};
        else if constexpr(Clone<A> && Trivial<B>)
            return Pair<A, B>{first.clone(), second};
        else if constexpr(Trivial<A> && Clone<B>)
            return Pair<A, B>{first, second.clone()};
        else
            return Pair<A, B>{first, second};
    }

    template<u64 Index>
    auto& get() {
        if constexpr(Index == 0) return first;
        if constexpr(Index == 1) return second;
    }
    template<u64 Index>
    const auto& get() const {
        if constexpr(Index == 0) return first;
        if constexpr(Index == 1) return second;
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
};

} // namespace rpp

namespace std {

using rpp::Pair;

template<typename T>
struct tuple_size;

template<size_t I, typename T>
struct tuple_element;

template<typename L, typename R>
struct tuple_size<Pair<L, R>> : std::integral_constant<size_t, 2> {};

template<typename L, typename R>
struct tuple_element<0, Pair<L, R>> {
    using type = L;
};
template<typename L, typename R>
struct tuple_element<1, Pair<L, R>> {
    using type = R;
};

} // namespace std
