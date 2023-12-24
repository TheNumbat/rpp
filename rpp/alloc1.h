
#pragma once

#ifndef RPP_BASE
#error "Include base.h instead."
#endif

namespace rpp {

namespace detail {

static consteval Literal pool_name(const Literal& name) {
    Literal ret;
    ret.c_string[0] = 'P';
    ret.c_string[1] = 'o';
    ret.c_string[2] = 'o';
    ret.c_string[3] = 'l';
    ret.c_string[4] = '<';
    u64 i = 0;
    for(; i < Literal::max_len - 6 && name.c_string[i]; i++) {
        ret.c_string[i + 5] = name.c_string[i];
    }
    ret.c_string[i + 5] = '>';
    return ret;
}

template<typename T>
struct Pool {
    static constexpr Literal name = pool_name(Reflect::Refl<T>::name);

    template<typename... Args>
        requires Constructable<T, Args...>
    static T* make(Args&&... args) {
        finalizer.keep_alive();
        Thread::Lock lock(mutex);
        return list.make(forward<Args>(args)...);
    }

    static void destroy(T* value) {
        finalizer.keep_alive();
        Thread::Lock lock(mutex);
        list.destroy(value);
    }

private:
    using Backing = Mallocator<name>;

    struct Finalizer {
        Finalizer(Free_List<T, Backing>& l) {
            Profile::finalizer([&l]() { l.clear(); });
        }
        RPP_FORCE_INLINE void keep_alive() {
        }
    };

    static inline Thread::Mutex mutex;
    static inline Free_List<T, Backing> list;
    static inline Finalizer finalizer{list};
};

} // namespace detail

struct Mpool {
    template<typename T, typename... Args>
        requires Constructable<T, Args...>
    static T* make(Args&&... args) {
        return detail::Pool<T>::make(forward<Args>(args)...);
    }

    template<typename T>
    static void destroy(T* value) {
        detail::Pool<T>::destroy(value);
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
