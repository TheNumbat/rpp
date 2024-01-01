
#pragma once

#ifndef RPP_BASE
#error "Include base.h instead."
#endif

namespace rpp {

namespace detail {

static consteval Literal pool_name(u64 N) {
    Literal ret{"Pool<"};
    u64 n = 0;
    for(u64 i = N; i > 0; i /= 10) {
        n++;
    }
    u64 i = 5;
    for(; n > 0 && i < Literal::max_len - 1; n--, i++) {
        ret.c_string[i] = '0' + (N / Math::pow<u64>(10, n - 1)) % 10;
    }
    ret.c_string[i] = '>';
    return ret;
}

template<u64 N>
struct Pool {
    static constexpr Literal name = pool_name(N);

    template<typename T, typename... Args>
        requires(sizeof(T) == N) && Constructable<T, Args...>
    static T* make(Args&&... args) {
        finalizer.keep_alive();
        Block* block = null;
        {
            Thread::Lock lock(mutex);
            block = list.make();
        }
        new(block->data) T{forward<Args>(args)...};
        return reinterpret_cast<T*>(block);
    }

    template<typename T>
        requires(sizeof(T) == N)
    static void destroy(T* value) {
        finalizer.keep_alive();
        if constexpr(Must_Destruct<T>) {
            value->~T();
        }
        {
            Thread::Lock lock(mutex);
            list.destroy(reinterpret_cast<Block*>(value));
        }
    }

private:
    using Backing = Mallocator<name>;

    struct Block {
        alignas(Math::min<u64>(N, 16)) u8 data[N];
    };

    struct Finalizer {
        Finalizer(Free_List<Block, Backing>& l) {
            Profile::finalizer([&l]() { l.clear(); });
        }
        RPP_FORCE_INLINE void keep_alive() {
        }
    };

    static inline Thread::Mutex mutex;
    static inline Free_List<Block, Backing> list;
    static inline Finalizer finalizer{list};
};

} // namespace detail

struct Mpool {
    template<typename T, typename... Args>
        requires Constructable<T, Args...>
    static T* make(Args&&... args) {
        return detail::Pool<sizeof(T)>::template make<T, Args...>(forward<Args>(args)...);
    }

    template<typename T>
    static void destroy(T* value) {
        detail::Pool<sizeof(T)>::template destroy<T>(value);
    }
};

template<Literal N, bool log>
void* Mallocator<N, log>::alloc(u64 size) {
    if(!size) return null;
    void* ret = sys_alloc(size);
    if constexpr(log) {
        Profile::alloc({String_View{N}, ret, size});
    }
    return ret;
}

template<Literal N, bool log>
void Mallocator<N, log>::free(void* mem) {
    if(!mem) return;
    if constexpr(log) {
        Profile::alloc({String_View{N}, mem, 0});
    }
    sys_free(mem);
}

} // namespace rpp
