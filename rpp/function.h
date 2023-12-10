
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
        new(storage) F{std::forward<F>(f)};
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
        return invoke(storage, std::forward<Args>(args)...);
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
            new(dst) F{std::move(*src)};
        }
    }
    template<typename F>
    static R f_invoke(F* src, Args... args) {
        return src->operator()(std::forward<Args>(args)...);
    }

    alignas(MAX_ALIGN) u8 storage[Words * 8] = {};
    VoidFn* vtable = null;

    friend struct Reflect<Function<Words, Fn>>;
};

} // namespace detail

template<typename F>
using Function = detail::Function<4, F>;

template<u64 Words, typename F>
using FunctionN = detail::Function<Words, F>;

template<u64 Words, typename Fn>
struct rpp::detail::Reflect<detail::Function<Words, Fn>> {
    using T = detail::Function<Words, Fn>;
    static constexpr Literal name = "Function";
    static constexpr Kind kind = Kind::record_;
    using members = List<>;
};

namespace Format {

template<Reflectable R, typename... Args>
    requires(Reflectable<Args> && ...)
struct Measure<Function<R(Args...)>> {
    using Fn = R(Args...);
    static u64 measure(const Function<Fn>& function) {
        u64 length = 10;
        if(function) {
            Region_Scope(Rg);
            length += String_View{Reflect<R>::name}.length();
            length += 2;
            if constexpr(sizeof...(Args) > 0)
                length += (format_typename<Args, Mregion<Rg>>().length() + ...);
            if constexpr(sizeof...(Args) > 1) length += 2 * (sizeof...(Args) - 1);
        } else {
            length += 4;
        }
        return length;
    }
};

template<Allocator O, typename... Ts>
    requires(Reflectable<Ts> && ...)
struct Type_Write {
    template<typename I>
    void apply() {
        Region_Scope(R);
        using T = Index<I::value, Ts...>;
        idx = output.write(idx, format_typename<T, Mregion<R>>());
        if constexpr(I::value + 1 < sizeof...(Ts)) idx = output.write(idx, ", "_v);
    }
    String<O>& output;
    u64 idx = 0;
};
template<Allocator O, Reflectable R, typename... Args>
    requires(Reflectable<Args> && ...)
struct Write<O, Function<R(Args...)>> {

    using Fn = R(Args...);
    using Indices = rpp::Index_List<Args...>;

    static u64 write(String<O>& output, u64 idx, const Function<Fn>& function) {
        idx = output.write(idx, "Function{"_v);
        if(function) {
            idx = output.write(idx, String_View{Reflect<R>::name});
            idx = output.write(idx, '(');
            Type_Write<O, Args...> iterator{output, idx};
            rpp::detail::list::Iter<Type_Write<O, Args...>, Indices>::apply(iterator);
            idx = output.write(iterator.idx, ')');
        } else {
            idx = output.write(idx, "null"_v);
        }
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
