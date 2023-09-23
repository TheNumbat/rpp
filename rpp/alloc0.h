
#pragma once

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

template<Literal N, bool Log = true>
struct Mallocator {
    static constexpr Literal name = N;
    static void* alloc(u64 size);
    static void free(void* mem);
};

#define Region_Scope Mregion::Scope region_scope__##__COUNTER__

struct Mregion {
    static constexpr Literal name = "Region";

    static void* alloc(u64 size);
    static void free(void* mem);

    struct Scope {
        Scope();
        ~Scope();
    };

    static u64 depth();
    static u64 size();

private:
    static void begin();
    static void end();

    static constexpr u64 REGION_COUNT = 64;
    static constexpr u64 REGION_STACK_SIZE = Math::MB(64);

    static inline thread_local u64 current_region = 0;
    static inline thread_local u64 current_offset = 0;
    static inline thread_local u64 region_offsets[REGION_COUNT] = {};
    static inline thread_local u8 stack_memory[REGION_STACK_SIZE] = {};
};

using Mdefault = Mallocator<"Default">;

using Mhidden = Mallocator<"Hidden", false>;

template<Allocator A, typename T, typename... Args>
    requires Constructable<T, Args...>
T* make(Args&&... args) {
    return new(A::alloc(sizeof(T))) T{std::forward<Args>(args)...};
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
        return new(alloc()) T{std::forward<Args>(args)...};
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
            Base::dealloc(list_);
            list_ = next;
        }
    }

private:
    T* alloc() {
        if(list_) {
            Free_Node* ret = list_;
            list_ = list_->next;
            std::memset(ret, 0, sizeof(Free_Node));
            return reinterpret_cast<T*>(ret);
        }
        void* new_node = Base::alloc(sizeof(Free_Node));
        return reinterpret_cast<T*>(new_node);
    }

    void free(T* mem) {
        Free_Node* node = reinterpret_cast<T*>(mem);
        node->next = list_;
        list_ = node;
    }

    union Free_Node {
        T value;
        Free_Node* next = null;
    };

    Free_Node* list_ = null;
};

} // namespace rpp
