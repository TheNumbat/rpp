
#pragma once

#ifndef RPP_BASE
#error "Include base.h instead."
#endif

namespace rpp {
namespace Thread {

using Alloc = Mallocator<"Threads">;

using Id = u64;

enum class Priority : u8 { low, normal, high, critical };

Id this_id();
u64 id_len();
void set_priority(Priority p);
void set_affinity(u64 core);
void sleep(u64 ms);

u64 perf_counter();
u64 perf_frequency();
u64 hardware_threads();

struct Flag {
    Flag() = default;
    ~Flag() = default;

    Flag(const Flag&) = delete;
    Flag(Flag&&) = delete;

    Flag& operator=(const Flag&) = delete;
    Flag& operator=(Flag&&) = delete;

    void block();
    void signal();

private:
#ifdef OS_WINDOWS
    i16 value_ = 0;
#else
    i32 value_ = 0;
#endif
};

struct Mutex {

    Mutex();
    ~Mutex();

    Mutex(const Mutex&) = delete;
    Mutex(Mutex&&) = delete;

    Mutex& operator=(Mutex&&) = delete;
    Mutex& operator=(const Mutex&) = delete;

    void lock();
    void unlock();
    bool try_lock();

private:
#ifdef OS_WINDOWS
    void* lock_ = null;
#else
    pthread_mutex_t lock_ = PTHREAD_MUTEX_INITIALIZER;
#endif

    friend struct Cond;
    friend struct Reflect<Mutex>;
};

struct Lock {

    Lock(Mutex& mutex) : mutex_(mutex) {
        mutex_->lock();
    }
    ~Lock() {
        if(mutex_) mutex_->unlock();
    }

    Lock(const Lock&) = delete;
    Lock& operator=(const Lock&) = delete;

    Lock(Lock&& src) = default;
    Lock& operator=(Lock&& src) = default;

private:
    Ref<Mutex> mutex_;

    friend struct Reflect<Lock>;
};

struct Atomic {

    Atomic() = default;
    ~Atomic() = default;

    explicit Atomic(i64 value) : value_(value) {
    }

    Atomic(const Atomic&) = default;
    Atomic(Atomic&&) = default;
    Atomic& operator=(const Atomic&) = default;
    Atomic& operator=(Atomic&&) = default;

    i64 load() const;
    i64 incr();
    i64 decr();
    i64 exchange(i64 value);
    i64 compare_and_swap(i64 compare_with, i64 set_to);

    template<Int I>
    I load() const {
        return static_cast<I>(load());
    }

private:
    i64 value_ = 0;

    friend struct Reflect<Atomic>;
};

struct Cond {

    Cond();
    ~Cond();

    Cond(Cond&&) = delete;
    Cond(const Cond&) = delete;

    Cond& operator=(Cond&&) = delete;
    Cond& operator=(const Cond&) = delete;

    void signal();
    void broadcast();
    void wait(Mutex& mut);

private:
#ifdef OS_WINDOWS
    void* cond_ = null;
#else
    pthread_cond_t cond_ = PTHREAD_COND_INITIALIZER;
#endif

    friend struct Reflect<Cond>;
};

} // namespace Thread

template<>
struct Reflect<Thread::Atomic> {
    using T = Thread::Atomic;
    static constexpr Literal name = "Atomic";
    static constexpr Kind kind = Kind::record_;
    using members = List<FIELD(value_)>;
};

template<>
struct Reflect<Thread::Priority> {
    using T = Thread::Priority;
    using underlying = u8;
    static constexpr Literal name = "Priority";
    static constexpr Kind kind = Kind::enum_;
    static constexpr Thread::Priority default_ = Thread::Priority::normal;
    using members = List<CASE(low), CASE(normal), CASE(high), CASE(critical)>;
    static_assert(Enum<T>);
};

} // namespace rpp
