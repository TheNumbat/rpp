
#pragma once

#ifndef RPP_BASE
#error "Include base.h instead."
#endif

namespace rpp {
namespace Thread {

using Alloc = Mallocator<"Threading">;

using Id = u64;

enum class Priority : u8 { low, normal, high, critical };

[[nodiscard]] Id this_id() noexcept;
[[nodiscard]] u64 id_len() noexcept;
void set_priority(Priority p) noexcept;
void set_affinity(u64 core) noexcept;
void sleep(u64 ms) noexcept;
void pause() noexcept;

[[nodiscard]] u64 perf_counter() noexcept;
[[nodiscard]] u64 perf_frequency() noexcept;
[[nodiscard]] u64 hardware_threads() noexcept;

struct Flag {
    Flag() noexcept = default;
    ~Flag() noexcept = default;

    Flag(const Flag&) noexcept = delete;
    Flag(Flag&&) noexcept = delete;

    Flag& operator=(const Flag&) noexcept = delete;
    Flag& operator=(Flag&&) noexcept = delete;

    void block() noexcept;
    void signal() noexcept;
    [[nodiscard]] bool ready() noexcept;

private:
#ifdef RPP_OS_WINDOWS
    i16 value_ = 0;
#else
    i32 value_ = 0;
#endif
};

struct Mutex {

    Mutex() noexcept;
    ~Mutex() noexcept;

    Mutex(const Mutex&) noexcept = delete;
    Mutex(Mutex&&) noexcept = delete;

    Mutex& operator=(Mutex&&) noexcept = delete;
    Mutex& operator=(const Mutex&) noexcept = delete;

    void lock() noexcept;
    void unlock() noexcept;
    [[nodiscard]] bool try_lock() noexcept;

private:
#ifdef RPP_OS_WINDOWS
    void* lock_ = null;
#else
    pthread_mutex_t lock_ = PTHREAD_MUTEX_INITIALIZER;
#endif

    friend struct Cond;
    friend struct Reflect::Refl<Mutex>;
};

struct Lock {

    Lock(Mutex& mutex) noexcept : mutex_(mutex) {
        mutex_->lock();
    }
    ~Lock() noexcept {
        if(mutex_) mutex_->unlock();
    }

    Lock(const Lock&) noexcept = delete;
    Lock& operator=(const Lock&) noexcept = delete;

    Lock(Lock&& src) noexcept = default;
    Lock& operator=(Lock&& src) noexcept = default;

private:
    Ref<Mutex> mutex_;

    friend struct Reflect::Refl<Lock>;
};

struct Atomic {

    Atomic() noexcept = default;
    ~Atomic() noexcept = default;

    explicit Atomic(i64 value) noexcept : value_(value) {
    }

    Atomic(const Atomic&) noexcept = default;
    Atomic(Atomic&&) noexcept = default;
    Atomic& operator=(const Atomic&) noexcept = default;
    Atomic& operator=(Atomic&&) noexcept = default;

    [[nodiscard]] i64 load() const noexcept;
    i64 incr() noexcept;
    i64 decr() noexcept;
    i64 exchange(i64 value) noexcept;
    [[nodiscard]] i64 compare_and_swap(i64 compare_with, i64 set_to) noexcept;

    template<Int I>
    [[nodiscard]] I load() const noexcept {
        return static_cast<I>(load());
    }

private:
    i64 value_ = 0;

    friend struct Reflect::Refl<Atomic>;
};

struct Cond {

    Cond() noexcept;
    ~Cond() noexcept;

    Cond(Cond&&) noexcept = delete;
    Cond(const Cond&) noexcept = delete;

    Cond& operator=(Cond&&) noexcept = delete;
    Cond& operator=(const Cond&) noexcept = delete;

    void signal() noexcept;
    void broadcast() noexcept;
    void wait(Mutex& mut) noexcept;

private:
#ifdef RPP_OS_WINDOWS
    void* cond_ = null;
#else
    pthread_cond_t cond_ = PTHREAD_COND_INITIALIZER;
#endif

    friend struct Reflect::Refl<Cond>;
};

} // namespace Thread

RPP_NAMED_RECORD(Thread::Atomic, "Atomic", RPP_FIELD(value_));

RPP_NAMED_ENUM(Thread::Priority, "Priority", normal, RPP_CASE(low), RPP_CASE(normal),
               RPP_CASE(high), RPP_CASE(critical));

} // namespace rpp
