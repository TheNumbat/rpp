
#pragma once

#include "base.h"
#include "thread.h"

#include <coroutine>

namespace rpp::Async {

constexpr i64 TASK_START = 0;
constexpr i64 TASK_DONE = 1;
// Other: awaiter continuation

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

template<typename R, Allocator A = Alloc>
struct Task;

template<typename R, Allocator A = Alloc>
struct Promise;

template<typename R, Allocator A = Alloc>
struct Final_Suspend {

    bool await_ready() noexcept {
        return false;
    }
    void await_resume() noexcept {
    }

    std::coroutine_handle<> await_suspend(std::coroutine_handle<Promise<R, A>> handle) noexcept {
        auto& promise = handle.promise();

        i64 state = promise.state.exchange(TASK_DONE);

        if(state == TASK_START) {
            return std::noop_coroutine();
        }

        return std::coroutine_handle<>::from_address(reinterpret_cast<void*>(state));
    }
};

template<typename D, typename R, Allocator A>
struct Promise_Base {

    Promise_Base() {
        fut = Thread::Future<R, A>::make();
    }
    ~Promise_Base() = default;

    Promise_Base(const Promise_Base&) = delete;
    Promise_Base& operator=(const Promise_Base&) = delete;

    Promise_Base(Promise_Base&&) = delete;
    Promise_Base& operator=(Promise_Base&&) = delete;

    Continue initial_suspend() noexcept {
        return {};
    }
    Final_Suspend<R, A> final_suspend() noexcept {
        return Final_Suspend<R, A>{};
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

    bool done() {
        return state.load() == TASK_DONE;
    }

    Thread::Future<R, A> future() {
        return fut.dup();
    }

protected:
    Thread::Atomic state{TASK_START};
    Thread::Future<R, A> fut;

    template<typename, Allocator>
    friend struct Task;
    template<typename, Allocator>
    friend struct Final_Suspend;
};

template<typename R, Allocator A>
struct Promise : Promise_Base<Promise<R, A>, R, A> {
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

template<typename R, Allocator A>
struct Task {

    using promise_type = Promise<R, A>;

    explicit Task(promise_type& promise) {
        handle = std::coroutine_handle<promise_type>::from_promise(promise);
        fut = promise.future();
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
        auto& promise = handle.promise();
        i64 cont = reinterpret_cast<i64>(continuation.address());

        i64 state = promise.state.compare_and_swap(TASK_START, cont);
        if(state == TASK_DONE) {
            return continuation;
        }
        return std::noop_coroutine();
    }
    auto await_resume() {
        if constexpr(Same<R, void>) {
            return;
        } else {
            return fut->block();
        }
    }

    void resume() {
        handle.resume();
    }
    bool done() {
        return await_ready();
    }
    auto block() {
        return fut->block();
    }

private:
    std::coroutine_handle<promise_type> handle;
    Thread::Future<R, A> fut;
    Thread::Atomic deleted;
};

template<Allocator A>
struct Promise<void, A> : Promise_Base<Promise<void, A>, void, A> {
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
