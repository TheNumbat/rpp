
#pragma once

#ifndef RPP_BASE
#error "Include base.h instead."
#endif

namespace rpp {

[[nodiscard]] void* sys_alloc(u64 size) noexcept;
void sys_free(void* mem) noexcept;
[[nodiscard]] i64 sys_net_allocs() noexcept;

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
    [[nodiscard]] static T* make(Args&&... args) noexcept {
        T* mem = reinterpret_cast<T*>(A::alloc(sizeof(T)));
        new(mem) T{forward<Args>(args)...};
        return mem;
    }

    template<typename T>
        requires Allocator<A>
    static void destroy(T* mem) noexcept {
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
    constexpr static Literal name = N;
    static void* alloc(u64 size) noexcept;
    static void free(void* mem) noexcept;
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
        [[nodiscard]] consteval operator bool() {
            return true;
        }
        Scope(const Scope&) = delete;
        Scope& operator=(const Scope&) = delete;
        Scope(Scope&&) = delete;
        Scope& operator=(Scope&&) = delete;
    };

    [[nodiscard]] static void* alloc(Region region, u64 size) noexcept;
    static void free(Region region, void* mem) noexcept;

    static u64 depth() noexcept;
    static u64 size() noexcept;

private:
    static void begin(Region region) noexcept;
    static void end(Region region) noexcept;

    friend struct Stack_Scope;
};

template<Region R>
struct Mregion {
    constexpr static Literal name = "Region";
    [[nodiscard]] static void* alloc(u64 size) noexcept {
        return Region_Allocator::alloc(R, size);
    }
    static void free(void* mem) noexcept {
        Region_Allocator::free(R, mem);
    }
};

#define RPP_REGION2(COUNTER) __region_##COUNTER
#define RPP_REGION1(R, BRAND, COUNTER)                                                             \
    if constexpr(constexpr u64 R = BRAND)                                                          \
        if(::rpp::Region_Allocator::Scope<R> RPP_REGION2(COUNTER){})

#define Region(R) RPP_REGION1(R, RPP_LOCATION_HASH, __COUNTER__)

using Mdefault = Mallocator<"Default">;
using Mhidden = Mallocator<"Hidden", false>;

template<typename T, Allocator Base>
struct Free_List {

    Free_List() noexcept = default;
    ~Free_List() noexcept {
        clear();
    }

    Free_List(const Free_List&) noexcept = delete;
    Free_List& operator=(const Free_List&) noexcept = delete;

    Free_List(Free_List&& src) noexcept : list_(src.list_) {
        src.list_ = null;
    }
    Free_List& operator=(Free_List&& src) noexcept {
        this->~Free_List();
        list_ = src.list_;
        src.list_ = null;
        return *this;
    }

    template<typename... Args>
        requires Constructable<T, Args...>
    [[nodiscard]] T* make(Args&&... args) noexcept {
        return new(alloc()) T{forward<Args>(args)...};
    }

    void destroy(T* value) noexcept {
        if constexpr(Must_Destruct<T>) {
            value->~T();
        }
        free(value);
    }

    void clear() noexcept {
        while(list_) {
            Free_Node* next = list_->next;
            Base::free(list_);
            list_ = next;
        }
    }

private:
    [[nodiscard]] T* alloc() noexcept {
        if(list_) {
            Free_Node* ret = list_;
            list_ = list_->next;
            return reinterpret_cast<T*>(ret);
        }
        void* new_node = Base::alloc(sizeof(Free_Node));
        return reinterpret_cast<T*>(new_node);
    }

    void free(T* mem) noexcept {
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
