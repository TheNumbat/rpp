
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
    static constexpr Literal name = N;

    operator T&() {
        return value;
    }
    operator const T&() const {
        return value;
    }

    T value;
};

template<typename... Ts>
    requires(sizeof...(Ts) > 0 && sizeof...(Ts) <= 255 && Distinct<Ts...>)
struct Variant {

    template<typename V>
        requires One_Is<V, Ts...>
    explicit Variant(V&& value) {
        construct(forward<V>(value));
    }
    ~Variant() {
        destruct();
    }

    Variant(const Variant& src)
        requires(Copy_Constructable<Ts> && ...)
    = default;
    Variant& operator=(const Variant& src)
        requires(Copy_Constructable<Ts> && ...)
    = default;

    Variant(Variant&& src)
        requires(Move_Constructable<Ts> && ...)
    {
        if constexpr((Trivially_Movable<Ts> && ...)) {
            index_ = src.index_;
            Libc::memcpy(data_, src.data_, size);
        } else {
            src.match(Overload{[this](Ts& v) { this->construct(move(v)); }...});
        };
        src.index_ = INVALID;
    }

    Variant& operator=(Variant&& src)
        requires(Move_Constructable<Ts> && ...)
    {
        destruct();
        if constexpr((Trivially_Movable<Ts> && ...)) {
            index_ = src.index_;
            Libc::memcpy(data_, src.data_, size);
        } else {
            src.match(Overload{[this](Ts& v) { this->construct(move(v)); }...});
        }
        src.index_ = INVALID;
        return *this;
    }

    Variant clone() const
        requires((Clone<Ts> || Trivial<Ts>) && ...)
    {
        return match([](const auto& v) {
            using T = Decay<decltype(v)>;
            if constexpr(Clone<T>) {
                return Variant{v.clone()};
            } else {
                static_assert(Trivial<T>);
                return Variant{T{v}};
            }
        });
    }

    template<typename F>
        requires(Invocable<F, Ts&> && ...) && All_Same<Invoke_Result<F, Ts&>...>
    auto match(F&& f) {
        return Accessors<false>::apply(forward<F>(f), data_, index_);
    }
    template<typename F>
        requires(Invocable<F, const Ts&> && ...) && All_Same<Invoke_Result<F, const Ts&>...>
    auto match(F&& f) const {
        return Accessors<true>::apply(forward<F>(f), data_, index_);
    }

    u8 index() const {
        assert(index_ != INVALID);
        return index_;
    }

private:
    static constexpr u8 INVALID = 255;

    template<typename V>
        requires One_Is<V, Ts...>
    void construct(V&& value) {
        static_assert(alignof(Variant<Ts...>) == align);
        static_assert(sizeof(Variant<Ts...>) == Math::align(size + 1, align));
        new(data_) V{forward<V>(value)};
        index_ = Index_Of<V, Ts...>;
    }

    void destruct() {
        if(index_ == INVALID) return;
        if constexpr((Must_Destruct<Ts> || ...)) {
            Accessors<false>::apply(Overload{[](Ts& v) {
                                        if constexpr(Must_Destruct<Ts>) {
                                            v.~Ts();
                                        }
                                    }...},
                                    data_, index_);
        }
        index_ = INVALID;
    }

    static constexpr u64 size = Math::max({sizeof(Ts)...});
    static constexpr u64 align = Math::max({alignof(Ts)...});

    alignas(align) u8 data_[size] = {};
    u8 index_ = 0;

    template<bool is_const>
    struct Accessors {
        template<typename T>
        using Ref = If<is_const, const T&, T&>;
        using Data = If<is_const, const u8, u8>;
        static constexpr u64 N = sizeof...(Ts);

        // For variants with up to 8 cases, we branch on the index.
        // Larger variants will use a table of function pointers.

#define INDEX(n)                                                                                   \
    if constexpr(N > n)                                                                            \
        if(index == n) return apply_one<F, Choose<n, Ts...>>(forward<F>(f), data)

        template<typename F>
            requires(N <= 8)
        static auto apply(F&& f, Data* data, u8 index) {
            INDEX(7);
            INDEX(6);
            INDEX(5);
            INDEX(4);
            INDEX(3);
            INDEX(2);
            INDEX(1);
            return apply_one<F, Choose<0, Ts...>>(forward<F>(f), data);
        }

#undef INDEX

        template<typename F>
        static auto apply(F&& f, Data* data, u8 index) {
            return apply_n(forward<F>(f), data, index, Index_Sequence_For<Ts...>{});
        }

    private:
        template<typename F, typename T>
        static auto apply_one(F&& f, Data* data) {
            return forward<F>(f)(reinterpret_cast<Ref<T>>(*data));
        }
        template<typename F, u64... Is>
        static auto apply_n(F&& f, Data* data, u8 index, Index_Sequence<Is...>) {
            using T = Choose<0, Ts...>;
            using R = Invoke_Result<F, Ref<T>>;
            using Apply = R (*)(F&&, Data*);
            static constexpr Apply table[] = {&apply_one<F, Choose<Is, Ts...>>...};
            return table[index](forward<F>(f), data);
        }
    };

    friend struct Reflect<Variant<Ts...>>;
};

namespace detail {

template<Literal N, typename NT>
struct Reflect<Named<N, NT>> {
    using T = Named<N, NT>;
    static constexpr Literal name = N;
    static constexpr Kind kind = Kind::record_;
    using members = List<FIELD(value)>;
};

template<typename... Ts>
struct Reflect<Variant<Ts...>> {
    using T = Variant<Ts...>;
    static constexpr Literal name = "Variant";
    static constexpr Kind kind = Kind::record_;
    using members = List<FIELD(data_), FIELD(index_)>;
};

} // namespace detail

namespace Format {

template<typename... Ts>
    requires(Reflectable<Ts> && ...)
struct Measure<Variant<Ts...>> {
    static u64 measure(const Variant<Ts...>& variant) {
        u64 length = 9;
        length +=
            variant.match(Overload{[](const Ts& value) { return Measure<Ts>::measure(value); }...});
        return length;
    }
};
template<Literal N, Reflectable T>
struct Measure<Named<N, T>> {
    static u64 measure(const Named<N, T>& named) {
        u64 length = String_View{N}.length() + 2;
        return length + Measure<T>::measure(named.value);
    }
};

template<Allocator O, typename... Ts>
    requires(Reflectable<Ts> && ...)
struct Write<O, Variant<Ts...>> {
    static u64 write(String<O>& output, u64 idx, const Variant<Ts...>& variant) {
        idx = output.write(idx, "Variant{"_v);
        idx = variant.match(Overload{[&output, idx](const Ts& value) {
            return Write<O, Ts>::write(output, idx, value);
        }...});
        return output.write(idx, '}');
    }
};
template<Allocator O, Literal N, Reflectable T>
struct Write<O, Named<N, T>> {
    static u64 write(String<O>& output, u64 idx, const Named<N, T>& named) {
        idx = output.write(idx, String_View{N});
        idx = output.write(idx, '{');
        idx = Write<O, T>::write(output, idx, named.value);
        return output.write(idx, '}');
    }
};

} // namespace Format

} // namespace rpp
