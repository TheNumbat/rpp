
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
    [[nodiscard]] bool await_ready() noexcept {
        return false;
    }
    void await_resume() noexcept {
    }
    void await_suspend(std::coroutine_handle<>) noexcept {
    }
};

struct Continue {
    [[nodiscard]] bool await_ready() noexcept {
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
    [[nodiscard]] bool await_ready() noexcept {
        return false;
    }
    void await_resume() noexcept {
    }

    template<typename R, Allocator A>
    [[nodiscard]] std::coroutine_handle<>
    await_suspend(std::coroutine_handle<Promise<R, A>> handle) noexcept {

        i64 state = handle.promise().state.exchange(TASK_DONE);

        if(state == TASK_ABANDONED) {
            handle.destroy();
        } else if(state != TASK_START) {
            return std::coroutine_handle<>::from_address(reinterpret_cast<void*>(state));
        }

        return std::noop_coroutine();
    }
};

template<typename R, Allocator A>
struct Promise_Base {

    Promise_Base() noexcept = default;
    ~Promise_Base() noexcept = default;

    Promise_Base(const Promise_Base&) noexcept = delete;
    Promise_Base& operator=(const Promise_Base&) noexcept = delete;

    Promise_Base(Promise_Base&&) noexcept = delete;
    Promise_Base& operator=(Promise_Base&&) noexcept = delete;

    [[nodiscard]] Continue initial_suspend() noexcept {
        return {};
    }
    [[nodiscard]] Final_Suspend final_suspend() noexcept {
        return Final_Suspend{};
    }
    void unhandled_exception() noexcept {
        die("Unhandled exception in coroutine.");
    }

    [[nodiscard]] void* operator new(size_t size) noexcept {
        return A::alloc(size);
    }
    void operator delete(void* ptr, size_t) noexcept {
        A::free(ptr);
    }

    void block() noexcept {
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
    [[nodiscard]] Task<R, A> get_return_object() noexcept {
        return Task<R, A>{*this};
    }
    void return_value(const R& val) noexcept {
        value = val;
        this->flag.signal();
    }
    void return_value(R&& val) noexcept {
        value = rpp::move(val);
        this->flag.signal();
    }

private:
    R value;
    friend struct Task<R, A>;
};

template<typename T>
concept Is_Task = requires(T t) {
    typename T::promise_type;
    typename T::return_type;
    { t.await_ready() } -> Same<bool>;
    { t.await_suspend(std::coroutine_handle<>()) } -> Same<bool>;
    { t.await_resume() } -> Same<typename T::return_type>;
};

template<typename R, Allocator A>
struct Task {

    using promise_type = Promise<R, A>;
    using return_type = R;

    Task() noexcept : handle{null} {
    }
    explicit Task(Promise<R, A>& promise) noexcept
        : handle{std::coroutine_handle<promise_type>::from_promise(promise)} {
    }

    ~Task() noexcept {
        if(handle) {
            auto state = handle.promise().state.exchange(TASK_ABANDONED);
            if(state == TASK_DONE) {
                handle.destroy();
            } else if(state != TASK_START) {
                die("Task abandoned while being waited upon.");
            }
        }
        handle = null;
    }

    Task(const Task&) noexcept = delete;
    Task& operator=(const Task&) noexcept = delete;

    Task(Task&& src) noexcept {
        handle = src.handle;
        src.handle = null;
    }
    Task& operator=(Task&& src) noexcept {
        this->~Task();
        handle = src.handle;
        src.handle = null;
        return *this;
    }

    [[nodiscard]] bool await_ready() noexcept {
        assert(handle);
        return handle.promise().state.load() == TASK_DONE;
    }
    [[nodiscard]] bool await_suspend(std::coroutine_handle<> continuation) noexcept {
        assert(handle);
        i64 cont = reinterpret_cast<i64>(continuation.address());
        return handle.promise().state.compare_and_swap(TASK_START, cont) == TASK_START;
    }
    [[nodiscard]] R await_resume() noexcept {
        assert(handle);
        if constexpr(Same<R, void>) {
            return;
        } else {
            return rpp::move(handle.promise().value);
        }
    }

    void resume() noexcept {
        assert(handle);
        handle.resume();
    }
    [[nodiscard]] bool done() noexcept {
        assert(handle);
        return await_ready();
    }
    [[nodiscard]] R block() noexcept {
        assert(handle);
        handle.promise().block();
        return await_resume();
    }

    [[nodiscard]] bool ok() const noexcept {
        return handle != null;
    }

    template<typename F>
        requires Invocable<F, R> && Is_Task<Invoke_Result<F, R>>
    [[nodiscard]] auto then(F&& f) noexcept -> Invoke_Result<F, R> {
        if constexpr(Same<R, void>) {
            co_await *this;
            co_return (co_await f());
        } else {
            co_return (co_await f(co_await *this));
        }
    }

private:
    std::coroutine_handle<Promise<R, A>> handle;
};

template<Allocator A>
struct Promise<void, A> : Promise_Base<void, A> {
    [[nodiscard]] Task<void, A> get_return_object() noexcept {
        return Task<void, A>{*this};
    }
    void return_void() noexcept {
#ifdef RPP_COMPILER_MSVC
        Libc::keep_alive();
#endif
        this->flag.signal();
    }
};

struct Event {

    Event() noexcept;
    ~Event() noexcept;

    Event(const Event&) noexcept = delete;
    Event& operator=(const Event&) noexcept = delete;

    Event(Event&& src) noexcept;
    Event& operator=(Event&& src) noexcept;

    [[nodiscard]] static u64 wait_any(Slice<Event> events) noexcept;

    void signal() const noexcept;
    void reset() const noexcept;
    [[nodiscard]] bool try_wait() const noexcept;

#ifdef RPP_OS_WINDOWS
    [[nodiscard]] static Event of_sys(void* event) noexcept;
#elif defined RPP_OS_LINUX
    [[nodiscard]] static Event of_sys(i32 fd, i32 mask) noexcept;
#elif defined RPP_OS_MACOS
    [[nodiscard]] static Event of_sys(i32 fd, i16 mask) noexcept;
#endif

private:
#ifdef RPP_OS_WINDOWS
    Event(void* event) noexcept : event_{event} {
    }
    void* event_ = null;
#elif defined RPP_OS_LINUX
    Event(i32 fd, i32 mask) noexcept : fd{fd}, mask{mask} {
    }
    i32 fd = -1;
    i32 mask = 0;
#elif defined RPP_OS_MACOS
    Event(i32 fd, i16 mask) noexcept : fd{fd}, mask{mask} {
    }
    i32 fd = -1;
    i32 signal_fd = -1;
    i16 mask = 0;
#endif
};

} // namespace rpp::Async
