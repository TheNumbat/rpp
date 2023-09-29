
#pragma once

namespace rpp {

namespace detail {

template<u64 Words, typename F>
struct Function;

template<u64 Words, typename R, typename... Args>
struct Function<Words, R(Args...)> {

    using Result = R;
    using Parameters = List<Args...>;
    using Fn = R(Args...);

    Function() = default;

    template<typename F>
        requires Same<Invoke_Result<F, Args...>, R>
    Function(F&& f) {
        construct(std::forward<F>(f));
    }
    ~Function() {
        destruct();
    }

    Function(const Function& src) = delete;
    Function& operator=(const Function& src) = delete;

    Function(Function&& src) {
        if(src.vtable) src.move(storage);
        vtable = src.vtable;
        src.vtable = null;
    }
    Function& operator=(Function&& src) {
        destruct();
        if(src.vtable) src.move(storage);
        vtable = src.vtable;
        src.vtable = null;
        return *this;
    }

    R operator()(Args... args) {
        return invoke(std::forward<Args>(args)...);
    }

    operator bool() const {
        return vtable != null;
    }

private:
    static constexpr u64 MAX_ALIGN = 16;

    template<typename F>
        requires Same<Invoke_Result<F, Args...>, R>
    void construct(F&& f) {
        static_assert(alignof(F) <= MAX_ALIGN);
        static_assert(sizeof(F) <= Words * 8);
        static void* f_vtable[] = {reinterpret_cast<void*>(&f_destruct<F>),
                                   reinterpret_cast<void*>(&f_move<F>),
                                   reinterpret_cast<void*>(&f_invoke<F>)};
        new(storage) F{std::forward<F>(f)};
        vtable = f_vtable;
    }
    void destruct() {
        if(vtable) {
            void (*destruct)(void*) = reinterpret_cast<void (*)(void*)>(vtable[0]);
            destruct(storage);
        }
        vtable = null;
    }
    void move(void* dst) {
        void (*move)(void*, void*) = reinterpret_cast<void (*)(void*, void*)>(vtable[1]);
        move(dst, storage);
    }
    R invoke(Args... args) {
        R (*invoke)(void*, Args...) = reinterpret_cast<R (*)(void*, Args...)>(vtable[2]);
        return invoke(storage, std::forward<Args>(args)...);
    }

    template<typename F>
    static void f_destruct(void* src) {
        if constexpr(Must_Destruct<F>) {
            reinterpret_cast<F*>(src)->~F();
        }
    }
    template<typename F>
    static void f_move(void* dst, void* src) {
        if constexpr(Trivially_Movable<F>) {
            std::memcpy(dst, src, sizeof(F));
        } else {
            new(dst) F{std::move(*reinterpret_cast<F*>(src))};
        }
    }
    template<typename F>
    static void f_invoke(void* src, Args... args) {
        (*reinterpret_cast<F*>(src))(std::forward<Args>(args)...);
    }

    alignas(MAX_ALIGN) u8 storage[Words * 8] = {};
    void** vtable = null;

    friend struct Reflect<Function<Words, Fn>>;
};

} // namespace detail

template<typename F>
using Function = detail::Function<4, F>;

template<u64 Words, typename Fn>
struct Reflect<detail::Function<Words, Fn>> {
    using T = detail::Function<Words, Fn>;
    static constexpr Literal name = "Function";
    static constexpr Kind kind = Kind::record_;
    using members = List<>;
    static_assert(Record<T>);
};

} // namespace rpp
