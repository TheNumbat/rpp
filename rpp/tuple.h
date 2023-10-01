
#pragma once

#include "base.h"

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

namespace Format {

template<typename... Ts>
    requires(Reflectable<Ts> && ...)
struct Tuple_Length {
    template<typename Index>
    void apply() {
        length +=
            Measure<typename Decay<decltype(tuple.template get<Index::value>())>::type>::measure(
                tuple.template get<Index::value>());
    }
    const Tuple<Ts...>& tuple;
    u64 length = 0;
};
template<typename... Ts>
    requires(Reflectable<Ts> && ...)
struct Measure<Tuple<Ts...>> {
    static u64 measure(const Tuple<Ts...>& tuple) {
        u64 length = 7;
        constexpr u64 N = sizeof...(Ts);
        if constexpr(N > 1) length += 2 * (N - 1);
        using Indices = rpp::Index_List<Ts...>;
        Tuple_Length<Ts...> iterator{tuple, 0};
        rpp::detail::list::Iter<Tuple_Length<Ts...>, Indices>::apply(iterator);
        return length + iterator.length;
    }
};

template<Allocator O, typename... Ts>
    requires(Reflectable<Ts> && ...)
struct Tuple_Write {
    template<typename Index>
    void apply() {
        constexpr u64 I = Index::value;
        idx = Write<O, typename Decay<decltype(tuple.template get<I>())>::type>::write(
            output, idx, tuple.template get<I>());
        if constexpr(I + 1 < sizeof...(Ts)) idx = output.write(idx, ", "_v);
    }
    const Tuple<Ts...>& tuple;
    String<O>& output;
    u64 idx = 0;
};
template<Allocator O, typename... Ts>
    requires(Reflectable<Ts> && ...)
struct Write<O, Tuple<Ts...>> {
    static u64 write(String<O>& output, u64 idx, const Tuple<Ts...>& tuple) {
        idx = output.write(idx, "Tuple{"_v);
        using Indices = rpp::Index_List<Ts...>;
        Tuple_Write<O, Ts...> iterator{tuple, output, idx};
        rpp::detail::list::Iter<Tuple_Write<O, Ts...>, Indices>::apply(iterator);
        return output.write(iterator.idx, '}');
    }
};

} // namespace Format

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
