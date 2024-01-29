
#pragma once

#include "base.h"

namespace rpp {

template<typename... Ts>
struct Overload : Ts... {
    using Ts::operator()...;
};

template<typename... Ts>
Overload(Ts...) -> Overload<Ts...>;

template<Literal N, typename T>
struct Named {
    constexpr static Literal name = N;

    [[nodiscard]] constexpr operator T&() noexcept {
        return value;
    }
    [[nodiscard]] constexpr operator const T&() const noexcept {
        return value;
    }

    T value;
};

template<typename... Ts>
    requires(sizeof...(Ts) > 0 && sizeof...(Ts) <= 255 && Distinct<Ts...>)
struct Variant {

    template<typename V>
        requires One_Is<V, Ts...>
    Variant(V&& value) noexcept {
        construct(rpp::forward<V>(value));
    }
    ~Variant() noexcept {
        destruct();
    }

    Variant(const Variant& src) noexcept
        requires(Copy_Constructable<Ts> && ...)
    = delete;
    Variant& operator=(const Variant& src) noexcept
        requires(Copy_Constructable<Ts> && ...)
    = delete;

    Variant(Variant&& src) noexcept
        requires(Move_Constructable<Ts> && ...)
    {
        if constexpr((Trivially_Movable<Ts> && ...)) {
            index_ = src.index_;
            Libc::memcpy(data_, src.data_, size);
        } else {
            src.match(Overload{[this](Ts& v) { this->construct(rpp::move(v)); }...});
        };
        src.index_ = INVALID;
    }

    Variant& operator=(Variant&& src) noexcept
        requires(Move_Constructable<Ts> && ...)
    {
        destruct();
        if constexpr((Trivially_Movable<Ts> && ...)) {
            index_ = src.index_;
            Libc::memcpy(data_, src.data_, size);
        } else {
            src.match(Overload{[this](Ts& v) { this->construct(rpp::move(v)); }...});
        }
        src.index_ = INVALID;
        return *this;
    }

    [[nodiscard]] Variant clone() const noexcept
        requires((Clone<Ts> || Copy_Constructable<Ts>) && ...)
    {
        if constexpr((Trivially_Copyable<Ts> && ...)) {
            Variant result;
            result.index_ = index_;
            Libc::memcpy(result.data_, data_, size);
            return result;
        } else {
            return match(Overload{[](const Ts& v) {
                if constexpr(Clone<Ts>) {
                    return Variant{v.clone()};
                } else {
                    static_assert(Copy_Constructable<Ts>);
                    return Variant{Ts{v}};
                }
            }...});
        }
    }

    template<typename F>
        requires((Invocable<F, Ts&> && ...) && All_Same<Invoke_Result<F, Ts&>...>)
    [[nodiscard]] auto match(F&& f) noexcept {
        return Accessors<false>::apply(rpp::forward<F>(f), data_, index_);
    }
    template<typename F>
        requires((Invocable<F, const Ts&> && ...) && All_Same<Invoke_Result<F, const Ts&>...>)
    [[nodiscard]] auto match(F&& f) const noexcept {
        return Accessors<true>::apply(rpp::forward<F>(f), data_, index_);
    }

    [[nodiscard]] u8 index() const noexcept {
        assert(index_ != INVALID);
        return index_;
    }

private:
    Variant() = default;

    constexpr static u8 INVALID = 255;

    template<typename V>
        requires One_Is<V, Ts...>
    void construct(V&& value) noexcept {
        static_assert(alignof(Variant<Ts...>) == align);
        static_assert(sizeof(Variant<Ts...>) == Math::align(size + 1, align));
        new(data_) V{rpp::forward<V>(value)};
        index_ = Index_Of<V, Ts...>;
    }

    void destruct() noexcept {
        if(index_ == INVALID) return;
        if constexpr((Must_Destruct<Ts> || ...)) {
            Accessors<false>::apply(Overload{[](Ts& v) {
                                        if constexpr(Must_Destruct<Ts>) {
                                            v.~Ts();
                                        }
                                        static_cast<void>(v);
                                    }...},
                                    data_, index_);
        }
        index_ = INVALID;
    }

    constexpr static u64 size = Math::max({sizeof(Ts)...});
    constexpr static u64 align = Math::max({alignof(Ts)...});

    alignas(align) u8 data_[size] = {};
    u8 index_ = INVALID;

    template<bool is_const>
    struct Accessors {
        template<typename T>
        using Ref = If<is_const, const T&, T&>;
        using Data = If<is_const, const u8, u8>;
        constexpr static u64 N = sizeof...(Ts);

        // For variants with up to 8 cases, we branch on the index.
        // Larger variants will use a table of function pointers.

#define INDEX(n)                                                                                   \
    if constexpr(N > n)                                                                            \
        if(index == n) return apply_one<F, Choose<n, Ts...>>(rpp::forward<F>(f), data)

        template<typename F>
            requires(N <= 8)
        [[nodiscard]] static auto apply(F&& f, Data* data, u8 index) noexcept {
            INDEX(7);
            INDEX(6);
            INDEX(5);
            INDEX(4);
            INDEX(3);
            INDEX(2);
            INDEX(1);
            return apply_one<F, Choose<0, Ts...>>(rpp::forward<F>(f), data);
        }

#undef INDEX

        template<typename F>
        [[nodiscard]] static auto apply(F&& f, Data* data, u8 index) noexcept {
            return apply_n(rpp::forward<F>(f), data, index, Index_Sequence_For<Ts...>{});
        }

    private:
        template<typename F, typename T>
        [[nodiscard]] static auto apply_one(F&& f, Data* data) noexcept {
            return rpp::forward<F>(f)(reinterpret_cast<Ref<T>>(*data));
        }
        template<typename F, u64... Is>
        [[nodiscard]] static auto apply_n(F&& f, Data* data, u8 index,
                                          Index_Sequence<Is...>) noexcept {
            using T = Choose<0, Ts...>;
            using R = Invoke_Result<F, Ref<T>>;
            using Apply = R (*)(F&&, Data*);
            constexpr static Apply table[] = {&apply_one<F, Choose<Is, Ts...>>...};
            return table[index](rpp::forward<F>(f), data);
        }
    };

    friend struct Reflect::Refl<Variant<Ts...>>;
};

template<Literal N, typename T>
RPP_NAMED_TEMPLATE_RECORD(Named, RPP_PACK(Named<N, T>)::name, RPP_PACK(N, T), RPP_FIELD(value));

template<typename... Ts>
RPP_TEMPLATE_RECORD(Variant, RPP_PACK(Ts...), RPP_FIELD(data_), RPP_FIELD(index_));

namespace Format {

template<typename... Ts>
    requires(Reflectable<Ts> && ...)
struct Measure<Variant<Ts...>> {
    [[nodiscard]] static u64 measure(const Variant<Ts...>& variant) noexcept {
        u64 length = 9;
        length +=
            variant.match(Overload{[](const Ts& value) { return Measure<Ts>::measure(value); }...});
        return length;
    }
};
template<Literal N, Reflectable T>
struct Measure<Named<N, T>> {
    [[nodiscard]] static u64 measure(const Named<N, T>& named) noexcept {
        u64 length = String_View{N}.length() + 2;
        return length + Measure<T>::measure(named.value);
    }
};

template<Allocator O, typename... Ts>
    requires(Reflectable<Ts> && ...)
struct Write<O, Variant<Ts...>> {
    [[nodiscard]] static u64 write(String<O>& output, u64 idx,
                                   const Variant<Ts...>& variant) noexcept {
        idx = output.write(idx, "Variant{"_v);
        idx = variant.match(Overload{[&output, idx](const Ts& value) {
            return Write<O, Ts>::write(output, idx, value);
        }...});
        return output.write(idx, '}');
    }
};
template<Allocator O, Literal N, Reflectable T>
struct Write<O, Named<N, T>> {
    [[nodiscard]] static u64 write(String<O>& output, u64 idx, const Named<N, T>& named) noexcept {
        idx = output.write(idx, String_View{N});
        idx = output.write(idx, '{');
        idx = Write<O, T>::write(output, idx, named.value);
        return output.write(idx, '}');
    }
};

} // namespace Format

} // namespace rpp
