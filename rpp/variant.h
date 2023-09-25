
#pragma once

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
    requires(sizeof...(Ts) > 0 && sizeof...(Ts) <= 256 && Distinct<Ts...>)
struct Variant {

    template<typename V>
        requires One<V, Ts...>
    explicit Variant(V&& value) {
        construct(std::forward<V>(value));
    }
    ~Variant() {
        destruct();
    }

    Variant(const Variant& src)
        requires(Trivial<Ts> && ...)
    = default;
    Variant& operator=(const Variant& src)
        requires(Trivial<Ts> && ...)
    = default;

    Variant(Variant&& src)
        requires(Move_Constructable<Ts> && ...)
    {
        if constexpr((Trivially_Movable<Ts> && ...)) {
            index_ = src.index_;
            std::memcpy(data_, src.data_, size);
        } else {
            std::move(src).match(Overload{[this](Ts& v) { this->construct(std::move(v)); }...});
        };
    }

    Variant& operator=(Variant&& src)
        requires(Move_Constructable<Ts> && ...)
    {
        destruct();
        if constexpr((Trivially_Movable<Ts> && ...)) {
            index_ = src.index_;
            std::memcpy(data_, src.data_, size);
        } else {
            std::move(src).match(Overload{[this](Ts& v) { this->construct(std::move(v)); }...});
        }
        return *this;
    }

    Variant clone()
        requires((Clone<Ts> || Trivial<Ts>) && ...)
    {
        return match([](const auto& v) {
            using T = typename Decay<decltype(v)>::type;
            if constexpr(Clone<T>) {
                return Variant{v.clone()};
            } else {
                static_assert(Trivial<T>);
                return Variant{T{v}};
            }
        });
    }

    u8 index() const {
        return index_;
    }

    template<typename F>
        requires((Invocable<F, Ts&> && ...) && All_Same<Invoke_Result<F, Ts&>...>)
    auto match(F&& f) {
        return Accessors<false>::apply(std::forward<F>(f), data_, index_);
    }
    template<typename F>
        requires((Invocable<F, const Ts&> && ...) && All_Same<Invoke_Result<F, const Ts&>...>)
    auto match(F&& f) const {
        return Accessors<true>::apply(std::forward<F>(f), data_, index_);
    }

private:
    template<typename V>
        requires One<V, Ts...>
    void construct(V&& value) {
        static_assert(alignof(Variant<Ts...>) == align);
        static_assert(sizeof(Variant<Ts...>) == Math::align(size + 1, align));
        new(data_) V{std::forward<V>(value)};
        index_ = Index_Of<V, Ts...>::value;
    }

    void destruct() {
        if constexpr((Must_Destruct<Ts> || ...)) {
            Accessors<false>::apply(Overload{[](Ts& v) {
                                        if constexpr(Must_Destruct<Ts>) {
                                            v.~Ts();
                                        }
                                    }...},
                                    data_, index_);
        }
    }

    static constexpr u64 size = Math::max({sizeof(Ts)...});
    static constexpr u64 align = Math::max({alignof(Ts)...});

    alignas(align) u8 data_[size] = {};
    u8 index_ = 0;

    template<bool Const>
    struct Accessors {
        template<typename T>
        using Ref = typename If<Const, const T&, T&>::type;
        using Data = typename If<Const, const u8, u8>::type;

        template<typename F>
        static auto apply(F&& f, Data* data, u8 index) -> decltype(auto) {
            return apply_(std::forward<F>(f), data, index, std::index_sequence_for<Ts...>{});
        }

    private:
        template<typename F, u64 I>
        static auto apply_one(F&& f, Data* data) -> decltype(auto) {
            using T = Index<I, Ts...>;
            return std::forward<F>(f)(reinterpret_cast<Ref<T>>(*data));
        }
        template<typename F, u64... Is>
        static auto apply_(F&& f, Data* data, u8 index, std::index_sequence<Is...>)
            -> decltype(auto) {
            using T = Index<0, Ts...>;
            using R = Invoke_Result<F, Ref<T>>;
            using Apply = R (*)(F&&, Data*);
            static constexpr Apply table[] = {&apply_one<F, Is>...};
            return table[index](std::forward<F>(f), data);
        }
    };

    friend struct Reflect<Variant<Ts...>>;
};

template<Literal N, typename NT>
struct Reflect<Named<N, NT>> {
    using T = Named<N, NT>;
    static constexpr Literal name = N;
    static constexpr Kind kind = Kind::record_;
    using members = List<FIELD(value)>;
    static_assert(Record<T>);
};

template<typename... Ts>
struct Reflect<Variant<Ts...>> {
    using T = Variant<Ts...>;
    static constexpr Literal name = "Variant";
    static constexpr Kind kind = Kind::record_;
    using members = List<FIELD(data_), FIELD(index_)>;
    static_assert(Record<T>);
};

} // namespace rpp
