
#pragma once

#include "base.h"
#include "function.h"
#include "thread.h"

#include <coroutine>

namespace rpp::Async {

using Alloc = Mallocator<"Async">;

struct Suspend {
    bool await_ready() noexcept {
        return false;
    }
    void await_resume() noexcept {
    }
    void await_suspend(std::coroutine_handle<> handle) noexcept {
    }
};

struct Continue {
    bool await_ready() noexcept {
        return true;
    }
    void await_resume() noexcept {
    }
    void await_suspend(std::coroutine_handle<> handle) noexcept {
    }
};

template<typename R>
struct Future;

template<typename R, Allocator A = Alloc>
struct Task;

template<typename R, Allocator A>
struct Promise_Base {

    Promise_Base() {
        fut = Arc<Future<R>, A>::make();
    }
    ~Promise_Base() {
        fut->abandon();
    }

    Promise_Base(const Promise_Base&) = delete;
    Promise_Base& operator=(const Promise_Base&) = delete;

    Promise_Base(Promise_Base&&) = delete;
    Promise_Base& operator=(Promise_Base&&) = delete;

    Continue initial_suspend() noexcept {
        return {};
    }
    Continue final_suspend() noexcept {
        return {};
    }
    void unhandled_exception() {
        die("Unhandled exception in coroutine.");
    }

    void* operator new(size_t size) {
        return A::alloc(size);
    }
    void operator delete(void* ptr, size_t) {
        A::free(ptr);
    }

    Arc<Future<R>, A>& future() {
        return fut;
    }

    void continue_with(FunctionN<1, void(std::coroutine_handle<>)> f) {
        fut->continue_with(std::move(f));
    }

protected:
    Arc<Future<R>, A> fut;
};

template<typename R, Allocator A>
struct Promise : Promise_Base<R, A> {
    Task<R, A> get_return_object() {
        return Task<R, A>{*this};
    }
    void return_value(const R& val) {
        this->fut->fill(val);
    }
    void return_value(R&& val) {
        this->fut->fill(std::move(val));
    }
};

template<typename R>
struct Future_Base {

    Future_Base() = default;
    ~Future_Base() = default;

    Future_Base(const Future_Base&) = delete;
    Future_Base& operator=(const Future_Base&) = delete;

    Future_Base(Future_Base&&) = delete;
    Future_Base& operator=(Future_Base&&) = delete;

    static constexpr i64 TASK_START = 0;
    static constexpr i64 TASK_DONE = 1;

protected:
    Thread::Mutex mut;
    Thread::Cond cond;

    void complete() {
        i64 c = continuation.exchange(TASK_DONE);
        if(c != TASK_START && on_continue) {
            on_continue(std::coroutine_handle<>::from_address(reinterpret_cast<void*>(c)));
        }
    }

private:
    Thread::Atomic continuation;
    FunctionN<1, void(std::coroutine_handle<>)> on_continue;

    bool install(std::coroutine_handle<> handle) {
        return continuation.compare_and_swap(TASK_START, reinterpret_cast<i64>(handle.address())) ==
               TASK_START;
    }
    void continue_with(FunctionN<1, void(std::coroutine_handle<>)> f) {
        on_continue = std::move(f);
    }
    void abandon() {
        i64 c = continuation.load();
        if(c != TASK_START && c != TASK_DONE) {
            std::coroutine_handle<>::from_address(reinterpret_cast<void*>(c)).destroy();
        }
    }

    template<typename, Allocator>
    friend struct Promise_Base;
    template<typename, Allocator>
    friend struct Task;
};

template<typename R>
struct Future : Future_Base<R> {

    bool ready() {
        Thread::Lock lock(this->mut);
        return value;
    }

    R block() {
        Thread::Lock lock(this->mut);
        while(!value) this->cond.wait(this->mut);
        return *value;
    }

    void fill(R&& val) {
        Thread::Lock lock(this->mut);
        value = std::move(val);
        this->cond.broadcast();
        this->complete();
    }

private:
    Opt<R> value;
};

template<>
struct Future<void> : Future_Base<void> {

    bool ready() {
        Thread::Lock lock(this->mut);
        return value;
    }

    void block() {
        Thread::Lock lock(this->mut);
        while(!value) this->cond.wait(this->mut);
    }

    void fill() {
        Thread::Lock lock(this->mut);
        value = true;
        this->cond.broadcast();
        this->complete();
    }

private:
    bool value = false;
};

template<typename R, Allocator A>
struct Task {

    using promise_type = Promise<R, A>;

    explicit Task(promise_type& promise) : fut{promise.future().dup()} {
    }

    ~Task() {
        assert(deleted.compare_and_swap(0, 1) == 0);
    }

    Task(const Task&) = delete;
    Task& operator=(const Task&) = delete;

    Task(Task&& src) = delete;
    Task& operator=(Task&& src) = delete;

    bool await_ready() {
        return fut->ready();
    }
    std::coroutine_handle<> await_suspend(std::coroutine_handle<> continuation) {
        if(fut->install(continuation)) {
            return std::noop_coroutine();
        }
        return continuation;
    }
    auto await_resume() {
        return fut->block();
    }

    bool done() {
        return await_ready();
    }
    auto block() {
        return fut->block();
    }

private:
    Arc<Future<R>, A> fut;
    Thread::Atomic deleted{0};
};

template<Allocator A>
struct Promise<void, A> : Promise_Base<void, A> {
    Task<void, A> get_return_object() {
        return Task<void, A>{*this};
    }
    void return_void() {
        this->fut->fill();
    }
};

struct Event {

    Event();
    ~Event();

    Event(const Event&) = delete;
    Event& operator=(const Event&) = delete;

    Event(Event&& src);
    Event& operator=(Event&& src);

    static u64 wait_any(Slice<Event> events);

    void signal() const;
    void reset() const;
    bool try_wait() const;

#ifdef OS_WINDOWS
    static Event of_sys(void* event);
#else
    static Event of_sys(i32 fd, i32 mask);
#endif

private:
#ifdef OS_WINDOWS
    Event(void* event) : event_{event} {
    }
    void* event_ = null;
#else
    Event(i32 fd, i32 mask) : fd{fd}, mask{mask} {
    }
    i32 fd = -1;
    i32 mask = 0;
#endif
};

} // namespace rpp::Async
