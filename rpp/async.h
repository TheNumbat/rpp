
#pragma once

#include "base.h"
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
        Thread::Lock lock{promise.mutex};
        promise.done = true;
        promise.cond.broadcast();
        return promise.continuation ? promise.continuation : std::noop_coroutine();
    }
};

template<typename R, Allocator A>
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

    void block() {
        Thread::Lock lock{mutex};
        while(!done) cond.wait(mutex);
    }

    void* operator new(size_t size) {
        return A::alloc(size);
    }
    void operator delete(void* ptr, size_t) {
        A::free(ptr);
    }

    void reference() {
        references.incr();
    }
    bool unreference() {
        return references.decr() == 0;
    }

protected:
    Thread::Mutex mutex;
    Thread::Cond cond;
    bool done = false;

    std::coroutine_handle<> continuation;
    Thread::Atomic references;

    template<typename, Allocator>
    friend struct Task;
    template<typename, Allocator>
    friend struct Final_Suspend;
};

template<typename R, Allocator A>
struct Promise : Promise_Base<R, A> {
    Task<R, A> get_return_object() {
        return Task<R, A>{*this};
    }
    void return_value(const R& val) {
        assert(!this->done);
        data = val;
    }
    void return_value(R&& val) {
        assert(!this->done);
        data = std::move(val);
    }
    R data;
};

template<typename R, Allocator A>
struct Task {

    using promise_type = Promise<R, A>;

    Task() = default;
    ~Task() {
        if(handle && handle.promise().unreference()) {
            handle.destroy();
        }
    }

    explicit Task(promise_type& promise) {
        promise.reference();
        handle = std::coroutine_handle<promise_type>::from_promise(promise);
    }

    Task(const Task&) = delete;
    Task& operator=(const Task&) = delete;

    Task(Task&& src) : handle{src.handle} {
        src.handle = null;
    }
    Task& operator=(Task&& src) {
        this->~Task();
        handle = src.handle;
        src.handle = null;
        return *this;
    }

    bool await_ready() {
        return !handle || handle.done();
    }
    std::coroutine_handle<> await_suspend(std::coroutine_handle<> continuation) {
        auto& promise = handle.promise();
        Thread::Lock lock{promise.mutex};
        promise.continuation = continuation;
        if(promise.done) {
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
        assert(handle);
        handle.resume();
    }
    bool done() {
        return await_ready();
    }
    auto block() {
        handle.promise().block();
        return await_resume();
    }

private:
    std::coroutine_handle<promise_type> handle;
};

template<Allocator A>
struct Promise<void, A> : Promise_Base<void, A> {
    Task<void, A> get_return_object() {
        return Task<void, A>{*this};
    }
    void return_void() {
        assert(!this->done);
    }
};

struct Event {

#ifdef OS_WINDOWS
    using Sys_Event = void*;
#else
    static_assert(false);
#endif

    Event();
    ~Event();

    Event(const Event&) = delete;
    Event& operator=(const Event&) = delete;

    Event(Event&& src) : event_{src.event_} {
        src.event_ = null;
    }
    Event& operator=(Event&& src) {
        this->~Event();
        event_ = src.event_;
        src.event_ = null;
        return *this;
    }

    static Event of_sys(Sys_Event event);
    static u64 wait_any(Slice<Event> events);
    static void wait_all(Slice<Event> events);

    void wait() const;
    void signal() const;
    bool ready() const;

private:
    Event(Sys_Event event) : event_{event} {
    }

#ifdef OS_WINDOWS
    Sys_Event event_ = null;
#else
    static_assert(false);
#endif
};

} // namespace rpp::Async
