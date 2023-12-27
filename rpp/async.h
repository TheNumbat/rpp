
#pragma once

#define RPP_ASYNC
#include "std/coroutine.h"

#include "base.h"
#include "thread.h"

namespace rpp::Async {

template<typename P = void>
struct Handle { // Thwarts ADL
    std::coroutine_handle<P> handle;
};

using Alloc = Thread::Alloc;

struct Suspend {
    bool await_ready() noexcept {
        return false;
    }
    void await_resume() noexcept {
    }
    void await_suspend(std::coroutine_handle<>) noexcept {
    }
};

struct Continue {
    bool await_ready() noexcept {
        return true;
    }
    void await_resume() noexcept {
    }
    void await_suspend(std::coroutine_handle<>) noexcept {
    }
};

template<typename R = void, Allocator A = Alloc>
struct Promise;

template<typename R = void, Allocator A = Alloc>
struct Task;

constexpr i64 TASK_START = 0;
constexpr i64 TASK_DONE = 1;
constexpr i64 TASK_ABANDONED = 2;

struct Final_Suspend {
    bool await_ready() noexcept {
        return false;
    }
    void await_resume() noexcept {
    }

#ifdef RPP_COMPILER_MSVC
    // Compiler bug: remove this when fix released
    // https://developercommunity.visualstudio.com/t/destroy-coroutine-from-final_suspend-r/10096047
    template<typename R, Allocator A>
    void await_suspend(std::coroutine_handle<Promise<R, A>> handle) noexcept {

        i64 state = handle.promise().state.exchange(TASK_DONE);

        if(state == TASK_ABANDONED) {
            handle.destroy();
        } else if(state != TASK_START) {
            // Can stack overflow
            std::coroutine_handle<>::from_address(reinterpret_cast<void*>(state)).resume();
        }
    }
#else
    template<typename R, Allocator A>
    std::coroutine_handle<> await_suspend(std::coroutine_handle<Promise<R, A>> handle) noexcept {

        i64 state = handle.promise().state.exchange(TASK_DONE);

        if(state == TASK_ABANDONED) {
            handle.destroy();
        } else if(state != TASK_START) {
            return std::coroutine_handle<>::from_address(reinterpret_cast<void*>(state));
        }

        return std::noop_coroutine();
    }
#endif
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
    Final_Suspend final_suspend() noexcept {
        return Final_Suspend{};
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

    void block() {
        flag.block();
    }

protected:
    Thread::Atomic state{TASK_START};
    Thread::Flag flag;

    friend struct Task<R, A>;
    friend struct Final_Suspend;
};

template<typename R, Allocator A>
struct Promise : Promise_Base<R, A> {
    Task<R, A> get_return_object() {
        return Task<R, A>{*this};
    }
    void return_value(const R& val) {
        value = val;
        this->flag.signal();
    }
    void return_value(R&& val) {
        value = rpp::move(val);
        this->flag.signal();
    }

private:
    R value;
    friend struct Task<R, A>;
};

template<typename R, Allocator A>
struct Task {

    using promise_type = Promise<R, A>;
    using return_type = R;

    Task() : handle{null} {
    }
    explicit Task(Promise<R, A>& promise)
        : handle{std::coroutine_handle<promise_type>::from_promise(promise)} {
    }

    ~Task() {
        if(handle && handle.promise().state.exchange(TASK_ABANDONED) == TASK_DONE) {
            handle.destroy();
        }
    }

    Task(const Task&) = delete;
    Task& operator=(const Task&) = delete;

    Task(Task&& src) {
        handle = src.handle;
        src.handle = null;
    }
    Task& operator=(Task&& src) {
        this->~Task();
        handle = src.handle;
        src.handle = null;
        return *this;
    }

    bool await_ready() {
        return handle.promise().state.load() == TASK_DONE;
    }
    bool await_suspend(std::coroutine_handle<> continuation) {
        i64 cont = reinterpret_cast<i64>(continuation.address());
        return handle.promise().state.compare_and_swap(TASK_START, cont) == TASK_START;
    }
    R await_resume() {
        if constexpr(Same<R, void>) {
            return;
        } else {
            return rpp::move(handle.promise().value);
        }
    }

    void resume() {
        handle.resume();
    }
    bool done() {
        return await_ready();
    }
    R block() {
        handle.promise().block();
        return await_resume();
    }
    operator bool() const {
        return handle != null;
    }

private:
    std::coroutine_handle<Promise<R, A>> handle;
};

template<Allocator A>
struct Promise<void, A> : Promise_Base<void, A> {
    Task<void, A> get_return_object() {
        return Task<void, A>{*this};
    }
    void return_void() {
        this->flag.signal();
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

#ifdef RPP_OS_WINDOWS
    static Event of_sys(void* event);
#else
    static Event of_sys(i32 fd, i32 mask);
#endif

private:
#ifdef RPP_OS_WINDOWS
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
