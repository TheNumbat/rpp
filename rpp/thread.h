
#pragma once

#include "base.h"

namespace rpp {
namespace Thread {

#ifdef OS_WINDOWS
using OS_Thread = void*;
using OS_Thread_Ret = u32;
constexpr OS_Thread OS_Thread_Null = null;
constexpr OS_Thread_Ret OS_Thread_Ret_Null = 0;
#else
using OS_Thread = pthread_t;
using OS_Thread_Ret = void*;
constexpr OS_Thread OS_Thread_Null = 0;
constexpr OS_Thread_Ret OS_Thread_Ret_Null = null;
#endif

Id sys_id(OS_Thread thread);
void sys_join(OS_Thread thread);
void sys_detach(OS_Thread thread);
OS_Thread sys_start(OS_Thread_Ret (*f)(void*), void* data);

template<typename T>
struct Promise {

    Promise() = default;
    ~Promise() = default;

    Promise(const Promise&) = delete;
    Promise(Promise&&) = delete;
    Promise& operator=(const Promise&) = delete;
    Promise& operator=(Promise&&) = delete;

    void fill(T&& val) {
        Lock lock(mut);
        assert(!value);
        value = std::move(val);
        cond.broadcast();
    }

    T& wait() {
        Lock lock(mut);
        while(!value) cond.wait(mut);
        return *value;
    }

    bool ready() {
        Lock lock(mut);
        return value;
    }

private:
    Mutex mut;
    Cond cond;
    Opt<T> value;

    friend struct Reflect<Promise<T>>;
};

template<>
struct Promise<void> {

    Promise() = default;
    ~Promise() = default;

    Promise(const Promise&) = delete;
    Promise(Promise&&) = delete;
    Promise& operator=(const Promise&) = delete;
    Promise& operator=(Promise&&) = delete;

    void fill() {
        Lock lock(mut);
        assert(!done);
        done = true;
        cond.broadcast();
    }

    void wait() {
        Lock lock(mut);
        while(!done) cond.wait(mut);
    }

    bool ready() {
        Lock lock(mut);
        return done;
    }

private:
    Mutex mut;
    Cond cond;
    bool done = false;

    friend struct Reflect<Promise<void>>;
};

template<typename T, Allocator A = Alloc>
using Future = Arc<Promise<T>, A>;

template<Allocator A = Alloc>
struct Thread {

    template<Invocable F>
    explicit Thread(F&& f) {
        F* data = reinterpret_cast<F*>(A::alloc(sizeof(F)));
        new(data) F{std::forward<F>(f)};
        thread = sys_start(&invoke<F>, data);
    }
    ~Thread() {
        if(thread) join();
    }

    Thread(const Thread&) = delete;
    Thread(Thread&& src) {
        thread = src.thread;
        src.thread = OS_Thread_Null;
    }

    Thread& operator=(const Thread&) = delete;
    Thread& operator=(Thread&& src) {
        join();
        thread = src.thread;
        src.thread = OS_Thread_Null;
        return *this;
    }

    Id id() {
        return sys_id(thread);
    }
    void join() {
        assert(thread);
        sys_join(thread);
        thread = OS_Thread_Null;
    }
    void detach() {
        assert(thread);
        sys_detach(thread);
        thread = OS_Thread_Null;
    }

private:
    OS_Thread thread = OS_Thread_Null;

    template<Invocable F>
    static OS_Thread_Ret invoke(void* _f) {
        F* f = static_cast<F*>(_f);
        Profile::start_thread();
        (*f)();
        f->~F();
        A::free(f);
        Profile::end_thread();
        return OS_Thread_Ret_Null;
    }

    friend struct Reflect<Thread<A>>;
};

template<Allocator A = Alloc, typename F, typename... Args>
    requires Invocable<F, Args...>
auto spawn(F&& f, Args&&... args) -> Future<Invoke_Result<F, Args...>, A> {

    using Result = Invoke_Result<F, Args...>;
    auto future = Future<Result, A>::make();

    Thread thread{[future = future.dup(), f = std::forward<F>(f),
                   ... args = std::forward<Args>(args)]() mutable {
        if constexpr(Same<Result, void>) {
            f(std::forward<Args>(args)...);
            future->fill();
        } else {
            future->fill(f(std::forward<Args>(args)...));
        }
    }};
    thread.detach();

    return future;
}

} // namespace Thread

template<typename P>
struct Reflect<Thread::Promise<P>> {
    using T = Thread::Promise<P>;
    static constexpr Literal name = "Promise";
    static constexpr Kind kind = Kind::record_;
    using members = List<FIELD(mut), FIELD(cond), FIELD(value)>;
    static_assert(Record<T>);
};

template<>
struct Reflect<Thread::Promise<void>> {
    using T = Thread::Promise<void>;
    static constexpr Literal name = "Promise";
    static constexpr Kind kind = Kind::record_;
    using members = List<FIELD(mut), FIELD(cond), FIELD(done)>;
    static_assert(Record<T>);
};

template<Allocator A>
struct Reflect<Thread::Thread<A>> {
    using T = Thread::Thread<A>;
    static constexpr Literal name = "Thread";
    static constexpr Kind kind = Kind::record_;
    using members = List<FIELD(thread)>;
    static_assert(Record<T>);
};

} // namespace rpp
