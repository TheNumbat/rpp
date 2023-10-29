
#pragma once

#include "base.h"
#include "thread.h"

#include <coroutine>

namespace rpp::Async {

constexpr i64 TASK_START = 0;
constexpr i64 TASK_DONE = 1;
constexpr i64 TASK_ABANDONED = 2;
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
        assert(state != TASK_DONE);

        // promise.done.signal();

        if(state == TASK_START) {
            return std::noop_coroutine();
        }
        if(state == TASK_ABANDONED) {
            handle.destroy();
            return std::noop_coroutine();
        }

        return std::coroutine_handle<>::from_address(reinterpret_cast<void*>(state));
    }
};

template<typename D, typename R, Allocator A>
struct Promise_Base {

    Promise_Base() = default;
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

    // void block() {
    // done.block();
    // auto handle = std::coroutine_handle<D>::from_promise(*static_cast<D*>(this));
    // while(!handle.done()) continue;
    // }

    void* operator new(size_t size) {
        return A::alloc(size);
    }
    void operator delete(void* ptr, size_t) {
        A::free(ptr);
    }

protected:
    Thread::Atomic state{TASK_START};
    // Thread::Flag done;

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
        data = val;
    }
    void return_value(R&& val) {
        data = std::move(val);
    }
    R data;
};

template<typename R, Allocator A>
struct Task {

    using promise_type = Promise<R, A>;

    explicit Task(promise_type& promise) {
        handle = std::coroutine_handle<promise_type>::from_promise(promise);
    }

    ~Task() {
        auto& promise = handle.promise();

        i64 state = promise.state.exchange(TASK_ABANDONED);
        assert(state != TASK_ABANDONED);

        if(state == TASK_DONE) {
            handle.destroy();
        }
    }

    Task(const Task&) = delete;
    Task& operator=(const Task&) = delete;

    Task(Task&& src) = delete;
    Task& operator=(Task&& src) = delete;

    bool await_ready() {
        return handle.done();
    }
    std::coroutine_handle<> await_suspend(std::coroutine_handle<> continuation) {
        auto& promise = handle.promise();
        i64 cont = reinterpret_cast<i64>(continuation.address());

        i64 state = promise.state.compare_and_swap(TASK_START, cont);
        assert(state == TASK_START || state == TASK_DONE);

        if(state == TASK_DONE) {
            return continuation;
        }
        return std::noop_coroutine();
    }
    auto await_resume() {
        if constexpr(Same<R, void>) {
            return;
        } else {
            return std::move(handle.promise().data);
        }
    }

    void resume() {
        assert(!handle.done());
        handle.resume();
    }
    bool done() {
        return await_ready();
    }
    // auto block() {
    //     handle.promise().block();
    //     return await_resume();
    // }

private:
    std::coroutine_handle<promise_type> handle;
};

template<Allocator A>
struct Promise<void, A> : Promise_Base<Promise<void, A>, void, A> {
    Task<void, A> get_return_object() {
        return Task<void, A>{*this};
    }
    void return_void() {
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
