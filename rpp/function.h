
#pragma once

#include "base.h"

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
    Function(F&& f) {
        construct(forward<F>(f));
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
        assert(vtable);
        return invoke(forward<Args>(args)...);
    }

private:
    using VoidFn = void (*)();
    static constexpr u64 MAX_ALIGN = 16;

    template<typename F>
        requires Same<Invoke_Result<F, Args...>, R>
    void construct(F&& f) {
        static_assert(alignof(F) <= MAX_ALIGN);
        static_assert(sizeof(F) <= Words * 8);
        static VoidFn f_vtable[] = {reinterpret_cast<VoidFn>(&f_destruct<F>),
                                    reinterpret_cast<VoidFn>(&f_move<F>),
                                    reinterpret_cast<VoidFn>(&f_invoke<F>)};
        new(storage) F{forward<F>(f)};
        vtable = static_cast<VoidFn*>(f_vtable);
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
        return invoke(storage, forward<Args>(args)...);
    }

    template<typename F>
    static void f_destruct(F* src) {
        if constexpr(Must_Destruct<F>) {
            src->~F();
        }
    }
    template<typename F>
    static void f_move(F* dst, F* src) {
        if constexpr(Trivially_Movable<F>) {
            Libc::memcpy(dst, src, sizeof(F));
        } else {
            new(dst) F{rpp::move(*src)};
        }
    }
    template<typename F>
    static R f_invoke(F* src, Args... args) {
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

namespace Reflect {

template<u64 Words, typename Fn>
struct Refl<::rpp::detail::Function<Words, Fn>> {
    using T = ::rpp::detail::Function<Words, Fn>;
    static constexpr Literal name = "Function";
    static constexpr Kind kind = Kind::record_;
    using members = List<>;
};

} // namespace Reflect

namespace Format {

template<Reflectable R, typename... Args>
    requires(Reflectable<Args> && ...)
struct Measure<Function<R(Args...)>> {
    using Fn = R(Args...);
    static u64 measure(const Function<Fn>& function) {
        Region_Scope(Rg);
        u64 length = 10;
        length += String_View{Reflect::Refl<R>::name}.length();
        length += 2;
        if constexpr(sizeof...(Args) > 0)
            length += (format_typename<Args, Mregion<Rg>>().length() + ...);
        if constexpr(sizeof...(Args) > 1) length += 2 * (sizeof...(Args) - 1);
        return length;
    }
};

template<Allocator O, u64 N>
struct Write_Type {
    template<typename Arg>
    void apply() {
        Region_Scope(R);
        idx = output.write(idx, format_typename<Arg, Mregion<R>>());
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
    static constexpr u64 N = sizeof...(Args);
    static u64 write(String<O>& output, u64 idx, const Function<Fn>& function) {
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
    static String<A> name() {
        return format<A>("Function<%(%)>"_v, Typename<R>::template name<A>(),
                         concat<A>(", "_v, Typename<Args>::template name<A>()...));
    }
};

} // namespace Format

} // namespace rpp
