
#pragma once

#ifndef RPP_BASE
#error "Include base.h instead."
#endif

namespace rpp {

namespace detail {

template<u64 Words, typename F>
struct Function;

template<u64 Words, typename R, typename... Args>
struct Function<Words, R(Args...)> {

    using Result = R;
    using Parameters = List<Args...>;
    using Fn = R(Args...);

    template<typename F>
        requires Same<Invoke_Result<F, Args...>, R>
    Function(F&& f) noexcept {
        construct(forward<F>(f));
    }
    ~Function() noexcept {
        destruct();
    }

    Function(const Function& src) noexcept = delete;
    Function& operator=(const Function& src) noexcept = delete;

    Function(Function&& src) noexcept {
        if(src.vtable) src.move(storage);
        vtable = src.vtable;
        src.vtable = null;
    }
    Function& operator=(Function&& src) noexcept {
        destruct();
        if(src.vtable) src.move(storage);
        vtable = src.vtable;
        src.vtable = null;
        return *this;
    }

    [[nodiscard]] R operator()(Args... args) noexcept {
        assert(vtable);
        return invoke(forward<Args>(args)...);
    }

private:
    using VoidFn = void (*)();
    constexpr static u64 MAX_ALIGN = 16;

    template<typename F>
        requires Same<Invoke_Result<F, Args...>, R>
    void construct(F&& f) noexcept {
        static_assert(alignof(F) <= MAX_ALIGN);
        static_assert(sizeof(F) <= Words * 8);
        static VoidFn f_vtable[] = {reinterpret_cast<VoidFn>(&f_destruct<F>),
                                    reinterpret_cast<VoidFn>(&f_move<F>),
                                    reinterpret_cast<VoidFn>(&f_invoke<F>)};
        new(storage) F{forward<F>(f)};
        vtable = static_cast<VoidFn*>(f_vtable);
    }
    void destruct() noexcept {
        if(vtable) {
            void (*destruct)(void*) = reinterpret_cast<void (*)(void*)>(vtable[0]);
            destruct(storage);
        }
        vtable = null;
    }
    void move(void* dst) noexcept {
        void (*move)(void*, void*) = reinterpret_cast<void (*)(void*, void*)>(vtable[1]);
        move(dst, storage);
    }
    [[nodiscard]] R invoke(Args... args) noexcept {
        R (*invoke)(void*, Args...) = reinterpret_cast<R (*)(void*, Args...)>(vtable[2]);
        return invoke(storage, forward<Args>(args)...);
    }

    template<typename F>
    static void f_destruct(F* src) noexcept {
        if constexpr(Must_Destruct<F>) {
            src->~F();
        }
    }
    template<typename F>
    static void f_move(F* dst, F* src) noexcept {
        if constexpr(Trivially_Movable<F>) {
            Libc::memcpy(dst, src, sizeof(F));
        } else {
            new(dst) F{rpp::move(*src)};
        }
    }
    template<typename F>
    [[nodiscard]] static R f_invoke(F* src, Args... args) noexcept {
        return src->operator()(forward<Args>(args)...);
    }

    alignas(MAX_ALIGN) u8 storage[Words * 8] = {};
    VoidFn* vtable = null;

    friend struct Reflect::Refl<Function<Words, Fn>>;
};

} // namespace detail

template<typename F>
using Function = detail::Function<4, F>;

template<u64 Words, typename F>
using FunctionN = detail::Function<Words, F>;

namespace Format {

template<Reflectable R, typename... Args>
    requires(Reflectable<Args> && ...)
struct Measure<Function<R(Args...)>> {
    using Fn = R(Args...);
    [[nodiscard]] static u64 measure(const Function<Fn>&) noexcept {
        u64 length = 10;
        length += String_View{Reflect::Refl<R>::name}.length();
        length += 2;
        Region(Rg) {
            if constexpr(sizeof...(Args) > 0)
                length += (format_typename<Args, Mregion<Rg>>().length() + ...);
        }
        if constexpr(sizeof...(Args) > 1) length += 2 * (sizeof...(Args) - 1);
        return length;
    }
};

template<Allocator O, u64 N>
struct Write_Type {
    template<typename Arg>
    void apply() noexcept {
        Region(R) {
            idx = output.write(idx, format_typename<Arg, Mregion<R>>());
        }
        if(n + 1 < N) idx = output.write(idx, ", "_v);
        n++;
    }
    u64 n = 0;
    u64 idx = 0;
    String<O>& output;
};

template<Allocator O, Reflectable R, typename... Args>
    requires(Reflectable<Args> && ...)
struct Write<O, Function<R(Args...)>> {
    using Fn = R(Args...);
    constexpr static u64 N = sizeof...(Args);
    [[nodiscard]] static u64 write(String<O>& output, u64 idx, const Function<Fn>&) noexcept {
        idx = output.write(idx, "Function{"_v);
        idx = output.write(idx, String_View{Reflect::Refl<R>::name});
        idx = output.write(idx, '(');
        Write_Type<O, N> iterator{0, idx, output};
        Reflect::Iter<Write_Type<O, N>, List<Args...>>::apply(iterator);
        idx = output.write(iterator.idx, ')');
        return output.write(idx, '}');
    }
};

template<Reflectable R, Reflectable... Args>
struct Typename<Function<R(Args...)>> {
    template<Allocator A>
    [[nodiscard]] static String<A> name() noexcept {
        return format<A>("Function<%(%)>"_v, Typename<R>::template name<A>(),
                         concat<A>(", "_v, Typename<Args>::template name<A>()...));
    }
};

} // namespace Format

template<typename F>
RPP_TEMPLATE_RECORD(Function, F);

template<u64 N, typename F>
RPP_TEMPLATE_RECORD(FunctionN, RPP_PACK(N, F));

} // namespace rpp
