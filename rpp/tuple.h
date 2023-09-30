
#pragma once

namespace rpp {

template<typename... Ts>
struct Tuple;

template<>
struct Tuple<> {

    Tuple() = default;
    ~Tuple() = default;

    Tuple(const Tuple&) = default;
    Tuple& operator=(const Tuple&) = default;

    Tuple(Tuple&&) = default;
    Tuple& operator=(Tuple&&) = default;

    Tuple<> clone() const {
        return Tuple<>{};
    }

    constexpr u64 length() const {
        return 0;
    }
};

template<typename T, typename... Ts>
struct Tuple<T, Ts...> {

    Tuple()
        requires Default_Constructable<T>
    = default;

    template<typename... Args>
    explicit Tuple(const T& first, Args&&... rest)
        requires Trivial<T> && Constructable<Tuple<Ts...>, Args...>
        : first(T{first}), rest(std::forward<Ts>(rest)...) {
    }

    template<typename... Args>
    explicit Tuple(T&& first, Args&&... rest)
        requires Move_Constructable<T> && Constructable<Tuple<Ts...>, Args...>
        : first(std::move(first)), rest(std::forward<Ts>(rest)...) {
    }

    ~Tuple() = default;

    Tuple(const Tuple&)
        requires Trivial<T> && Trivial<Tuple<Ts...>>
    = default;
    Tuple& operator=(const Tuple&)
        requires Trivial<T> && Trivial<Tuple<Ts...>>
    = default;

    Tuple(Tuple&&) = default;
    Tuple& operator=(Tuple&&) = default;

    constexpr u64 length() const {
        return 1 + sizeof...(Ts);
    }

    Tuple clone() const
        requires(Clone<T> || Trivial<T>) && (Clone<Tuple<Ts...>>)
    {
        if constexpr(Clone<T>) {
            return Tuple{first.clone(), rest.clone()};
        } else {
            static_assert(Trivial<T>);
            return Tuple{T{first}, rest.clone()};
        }
    }

    template<u64 Index>
    auto& get() {
        if constexpr(Index == 0)
            return first;
        else
            return rest.template get<Index - 1>();
    }
    template<u64 Index>
    const auto& get() const {
        if constexpr(Index == 0)
            return first;
        else
            return rest.template get<Index - 1>();
    }

private:
    explicit Tuple(T&& first, Tuple<Ts...>&& rest)
        : first(std::move(first)), rest(std::move(rest)) {
    }

    T first;
    Tuple<Ts...> rest;

    friend struct Reflect<Tuple<T, Ts...>>;
};

template<>
struct Reflect<Tuple<>> {
    using T = Tuple<>;
    static constexpr Literal name = "Tuple";
    static constexpr Kind kind = Kind::record_;
    using members = List<>;
};

template<typename F, typename... Ts>
struct Reflect<Tuple<F, Ts...>> {
    using T = Tuple<F, Ts...>;
    static constexpr Literal name = "Tuple";
    static constexpr Kind kind = Kind::record_;
    using members = List<FIELD(first), FIELD(rest)>;
};

} // namespace rpp

namespace std {

using rpp::Tuple;

template<typename... Ts>
struct tuple_size<Tuple<Ts...>> : std::integral_constant<size_t, sizeof...(Ts)> {};

template<size_t Index, typename... Ts>
struct tuple_element<Index, Tuple<Ts...>> {
    using type = decltype(std::declval<Tuple<Ts...>>().template get<Index>());
};

} // namespace std
