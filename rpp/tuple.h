
#pragma once

#include "base.h"

namespace rpp {

template<typename... Ts>
struct Tuple;

template<>
struct Tuple<> {

    constexpr Tuple() noexcept = default;
    constexpr ~Tuple() noexcept = default;

    constexpr Tuple(const Tuple&) noexcept = default;
    constexpr Tuple& operator=(const Tuple&) noexcept = default;

    constexpr Tuple(Tuple&&) noexcept = default;
    constexpr Tuple& operator=(Tuple&&) noexcept = default;

    [[nodiscard]] constexpr Tuple<> clone() const noexcept {
        return Tuple<>{};
    }

    [[nodiscard]] constexpr u64 length() const noexcept {
        return 0;
    }

    template<Invocable F>
    [[nodiscard]] constexpr auto invoke(F&& f) noexcept -> Invoke_Result<F> {
        return forward<F>(f)();
    }
};

template<typename T, typename... Ts>
struct Tuple<T, Ts...> {

    constexpr Tuple() noexcept
        requires Default_Constructable<T>
    = default;

    template<typename... Args>
    constexpr explicit Tuple(const T& first, Args&&... rest) noexcept
        requires Copy_Constructable<T> && Constructable<Tuple<Ts...>, Args...>
        : first(T{first}), rest(forward<Args>(rest)...) {
    }

    template<typename... Args>
        requires Move_Constructable<T> && Move_Constructable<Tuple<Args...>>
    constexpr explicit Tuple(T&& first, Tuple<Args...>&& rest) noexcept
        : first(move(first)), rest(move(rest)) {
    }

    template<typename... Args>
        requires Copy_Constructable<T> && Move_Constructable<Tuple<Args...>>
    constexpr explicit Tuple(const T& first, Tuple<Args...>&& rest) noexcept
        : first(T{first}), rest(move(rest)) {
    }

    template<typename... Args>
    constexpr explicit Tuple(T&& first, Args&&... rest) noexcept
        requires Move_Constructable<T> && Constructable<Tuple<Ts...>, Args...>
        : first(move(first)), rest(forward<Args>(rest)...) {
    }

    constexpr ~Tuple() noexcept = default;

    constexpr Tuple(const Tuple&) noexcept
        requires Trivial<T> && Trivial<Tuple<Ts...>>
    = default;
    constexpr Tuple& operator=(const Tuple&) noexcept
        requires Trivial<T> && Trivial<Tuple<Ts...>>
    = default;

    constexpr Tuple(Tuple&&) noexcept = default;
    constexpr Tuple& operator=(Tuple&&) noexcept = default;

    constexpr u64 length() const noexcept {
        return 1 + sizeof...(Ts);
    }

    [[nodiscard]] constexpr Tuple clone() const noexcept
        requires((Clone<T> || Copy_Constructable<T>) && (Clone<Tuple<Ts...>>))
    {
        if constexpr(Clone<T>) {
            return Tuple{first.clone(), rest.clone()};
        } else {
            static_assert(Copy_Constructable<T>);
            return Tuple{T{first}, rest.clone()};
        }
    }

    template<u64 Index>
    [[nodiscard]] constexpr auto& get() noexcept {
        if constexpr(Index == 0)
            return first;
        else
            return rest.template get<Index - 1>();
    }
    template<u64 Index>
    [[nodiscard]] constexpr const auto& get() const noexcept {
        if constexpr(Index == 0)
            return first;
        else
            return rest.template get<Index - 1>();
    }

    template<Invocable<T, Ts...> F>
    [[nodiscard]] constexpr auto invoke(F&& f) noexcept -> Invoke_Result<F, T, Ts...> {
        return invoke_(forward<F>(f), Make_Index_Sequence<1 + sizeof...(Ts)>{});
    }

private:
    template<Invocable<T, Ts...> F, u64... Is>
    [[nodiscard]] constexpr auto invoke_(F&& f, Index_Sequence<Is...>) noexcept
        -> Invoke_Result<F, T, Ts...> {
        return forward<F>(f)(get<Is>()...);
    }

    T first;
    Tuple<Ts...> rest;

    friend struct Reflect::Refl<Tuple<T, Ts...>>;
};

template<>
RPP_TEMPLATE_RECORD(Tuple, RPP_PACK());

template<typename F, typename... Ts>
RPP_TEMPLATE_RECORD(Tuple, RPP_PACK(F, Ts...), RPP_FIELD(first), RPP_FIELD(rest));

namespace Format {

template<typename... Ts>
    requires(Reflectable<Ts> && ...)
struct Tuple_Length {
    template<typename Index>
    constexpr void apply() noexcept {
        length += Measure<Decay<decltype(tuple.template get<Index::value>())>>::measure(
            tuple.template get<Index::value>());
    }
    const Tuple<Ts...>& tuple;
    u64 length = 0;
};
template<typename... Ts>
    requires(Reflectable<Ts> && ...)
struct Measure<Tuple<Ts...>> {
    [[nodiscard]] constexpr static u64 measure(const Tuple<Ts...>& tuple) noexcept {
        u64 length = 7;
        constexpr u64 N = sizeof...(Ts);
        if constexpr(N > 1) length += 2 * (N - 1);
        using Indices = Reflect::Enumerate<Ts...>;
        Tuple_Length<Ts...> iterator{tuple, 0};
        Reflect::Iter<Tuple_Length<Ts...>, Indices>::apply(iterator);
        return length + iterator.length;
    }
};

template<Allocator O, typename... Ts>
    requires(Reflectable<Ts> && ...)
struct Tuple_Write {
    template<typename Index>
    void apply() noexcept {
        constexpr u64 I = Index::value;
        idx = Write<O, Decay<decltype(tuple.template get<I>())>>::write(output, idx,
                                                                        tuple.template get<I>());
        if constexpr(I + 1 < sizeof...(Ts)) idx = output.write(idx, ", "_v);
    }
    const Tuple<Ts...>& tuple;
    String<O>& output;
    u64 idx = 0;
};
template<Allocator O, typename... Ts>
    requires(Reflectable<Ts> && ...)
struct Write<O, Tuple<Ts...>> {
    [[nodiscard]] static u64 write(String<O>& output, u64 idx, const Tuple<Ts...>& tuple) noexcept {
        idx = output.write(idx, "Tuple{"_v);
        using Indices = Reflect::Enumerate<Ts...>;
        Tuple_Write<O, Ts...> iterator{tuple, output, idx};
        Reflect::Iter<Tuple_Write<O, Ts...>, Indices>::apply(iterator);
        return output.write(iterator.idx, '}');
    }
};

} // namespace Format

} // namespace rpp

namespace std {

using rpp::Tuple;

template<typename... Ts>
struct tuple_size<Tuple<Ts...>> : rpp::Constant<rpp::u64, sizeof...(Ts)> {};

template<size_t Index, typename... Ts>
struct tuple_element<Index, Tuple<Ts...>> {
    using type = decltype(rpp::detail::declval<Tuple<Ts...>>().template get<Index>());
};

} // namespace std
