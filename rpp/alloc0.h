
#pragma once

#ifndef RPP_BASE
#error "Include base.h instead."
#endif

#ifdef RPP_COMPILER_MSVC
void* operator new(rpp::u64, std::align_val_t, void* ptr) noexcept;
void* operator new[](rpp::u64, std::align_val_t, void* ptr) noexcept;
void operator delete(void*, std::align_val_t, void*) noexcept;
void operator delete[](void*, std::align_val_t, void*) noexcept;
#endif

namespace rpp {

void* sys_alloc(u64 size);
void sys_free(void* mem);
i64 sys_net_allocs();

template<typename A>
concept Allocator = requires(u64 size, void* address) {
    Same<Literal, decltype(A::name)>;
    { A::alloc(size) } -> Same<void*>;
    { A::free(address) } -> Same<void>;
};

template<typename A>
concept Pool = requires(Empty<> t) { // Can't express forall types T
    { A::template make<Empty<>>(t) } -> Same<Empty<>*>;
    { A::template destroy<Empty<>>(&t) } -> Same<void>;
};

template<typename A>
concept Scalar_Allocator = Allocator<A> || Pool<A>;

namespace detail {

template<typename A>
struct Scalar_Adaptor {
    template<typename T, typename... Args>
        requires Allocator<A> && Constructable<T, Args...>
    static T* make(Args&&... args) {
        T* mem = reinterpret_cast<T*>(A::alloc(sizeof(T)));
        new(mem) T{forward<Args>(args)...};
        return mem;
    }

    template<typename T>
        requires Allocator<A>
    static void destroy(T* mem) {
        if constexpr(Must_Destruct<T>) {
            mem->~T();
        }
        A::free(mem);
    }
};

} // namespace detail

template<typename P>
using Pool_Adaptor = If<Allocator<P>, detail::Scalar_Adaptor<P>, P>;

template<Literal N, bool Log = true>
struct Mallocator {
    static constexpr Literal name = N;
    static void* alloc(u64 size);
    static void free(void* mem);
};

using Region = u64;

struct Region_Allocator {
    template<Region R>
    struct Scope {
        Scope() {
            begin(R);
        }
        ~Scope() {
            end(R);
        }

        Scope(const Scope&) = delete;
        Scope& operator=(const Scope&) = delete;
        Scope(Scope&&) = delete;
        Scope& operator=(Scope&&) = delete;
    };

    static void* alloc(Region region, u64 size);
    static void free(Region region, void* mem);

    static u64 depth();
    static u64 size();

private:
    static void begin(Region region);
    static void end(Region region);

    friend struct Stack_Scope;
};

template<Region R>
struct Mregion {
    static constexpr Literal name = "Region";
    static void* alloc(u64 size) {
        return Region_Allocator::alloc(R, size);
    }
    static void free(void* mem) {
        Region_Allocator::free(R, mem);
    }
};

#define REGION_SCOPE2(counter) __region_scope_##counter
#define REGION_SCOPE1(R, brand, counter)                                                           \
    ::rpp::Region_Allocator::Scope<brand> REGION_SCOPE2(counter);                                  \
    static constexpr u64 R = brand;

#define Region_Scope(R) REGION_SCOPE1(R, LOCATION_HASH, __COUNTER__)

using Mdefault = Mallocator<"Default">;
using Mhidden = Mallocator<"Hidden", false>;

template<Allocator A, typename T, typename... Args>
    requires Constructable<T, Args...>
T* make(Args&&... args) {
    return new(A::alloc(sizeof(T))) T{forward<Args>(args)...};
}

template<Allocator A, typename T>
void destroy(T* value) {
    if constexpr(Must_Destruct<T>) {
        value->~T();
    }
    A::free(value);
}

template<typename T, Allocator Base>
struct Free_List {

    Free_List() = default;
    ~Free_List() {
        clear();
    }

    Free_List(const Free_List&) = delete;
    Free_List& operator=(const Free_List&) = delete;

    Free_List(Free_List&& src) : list_(src.list_) {
        src.list_ = null;
    }
    Free_List& operator=(Free_List&& src) {
        this->~Free_List();
        list_ = src.list_;
        src.list_ = null;
        return *this;
    }

    template<typename... Args>
        requires Constructable<T, Args...>
    T* make(Args&&... args) {
        return new(alloc()) T{forward<Args>(args)...};
    }

    void destroy(T* value) {
        if constexpr(Must_Destruct<T>) {
            value->~T();
        }
        free(value);
    }

    void clear() {
        while(list_) {
            Free_Node* next = list_->next;
            Base::free(list_);
            list_ = next;
        }
    }

private:
    T* alloc() {
        if(list_) {
            Free_Node* ret = list_;
            list_ = list_->next;
            Libc::memset(ret, 0, sizeof(Free_Node));
            return reinterpret_cast<T*>(ret);
        }
        void* new_node = Base::alloc(sizeof(Free_Node));
        return reinterpret_cast<T*>(new_node);
    }

    void free(T* mem) {
        Free_Node* node = reinterpret_cast<Free_Node*>(mem);
        node->next = list_;
        list_ = node;
    }

    union Free_Node {
        ~Free_Node() = delete;
        T value;
        Free_Node* next = null;
    };

    Free_Node* list_ = null;
};

} // namespace rpp
