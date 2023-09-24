
#pragma once

#include "base.h"
#include "thread.h"

#include <coroutine>

namespace rpp::Async {

using Suspend = std::suspend_always;

using Alloc = Mallocator<"Async">;

template<typename T, Allocator A>
struct Promise;

template<typename T, Allocator A>
struct Coroutine;

template<typename T, Allocator A = Alloc>
struct Coroutine {

    using return_type = T;
    using promise_type = Promise<T, A>;

    explicit Coroutine(std::coroutine_handle<Promise<T, A>> handle_) {
        handle = handle_;
        handle.promise().reference();
    }
    explicit Coroutine(Promise<T, A>& promise) {
        handle = std::coroutine_handle<Promise<T, A>>::from_promise(promise);
        handle.promise().reference();
    }
    ~Coroutine() {
        if(handle) {
            if(handle.promise().unreference()) {
                handle.destroy();
            }
        }
    }

    Coroutine(const Coroutine& src) = delete;
    Coroutine& operator=(const Coroutine& src) = delete;

    Coroutine dup() const {
        return Coroutine{handle};
    }

    Coroutine(Coroutine&& src) {
        handle = src.handle;
        src.handle = null;
    }
    Coroutine& operator=(Coroutine&& src) {
        this->~Coroutine();
        handle = src.handle;
        src.handle = null;
        return *this;
    }

    T& wait() {
        return await_resume();
    }
    bool done() {
        return await_ready();
    }

    std::coroutine_handle<> await_suspend(std::coroutine_handle<> cont) {
        // this is what we're waiting on, next is the suspension of the current job
        return handle;
    }
    void resume() {
        assert(handle);
        handle.resume();
    }
    T& await_resume() {
        assert(handle);
        return handle.promise().await_resume();
    }
    bool await_ready() {
        assert(handle);
        return handle.promise().await_ready();
    }

private:
    std::coroutine_handle<Promise<T, A>> handle;

    template<typename U, Allocator B>
    friend struct Promise;
};

template<typename T, Allocator A = Alloc>
struct Promise {
    Promise() = default;
    ~Promise() = default;

    Promise(const Promise& src) = delete;
    Promise& operator=(const Promise& src) = delete;

    Promise(Promise&& src) = delete;
    Promise& operator=(Promise&& src) = delete;

    Coroutine<T> get_return_object() {
        return Coroutine{*this};
    }

    std::suspend_never initial_suspend() noexcept {
        return {};
    }
    std::suspend_always final_suspend() noexcept {
        return {};
    }

    void return_value(T&& value) noexcept {
        promise.fill(std::move(value));
    }

    void unhandled_exception() noexcept {
        die("Unhandled exception in coroutine.");
    }

    void* operator new(std::size_t size) noexcept {
        return A::alloc(size);
    }

    void operator delete(void* ptr, std::size_t size) noexcept {
        A::free(ptr);
    }

    T& await_resume() noexcept {
        return promise.wait();
    }
    bool await_ready() noexcept {
        return promise.ready();
    }

private:
    void reference() {
        references.incr();
    }
    bool unreference() {
        return references.decr() == 0;
    }

    Thread::Promise<T> promise;
    Thread::Atomic references;

    template<typename U, Allocator B>
    friend struct Coroutine;
};

template<Allocator A>
struct Coroutine<void, A> {

    using return_type = void;
    using promise_type = Promise<void, A>;

    explicit Coroutine(std::coroutine_handle<Promise<void, A>> promise) {
        handle = promise;
        handle.promise().reference();
    }
    explicit Coroutine(Promise<void, A>& promise) {
        handle = std::coroutine_handle<Promise<void, A>>::from_promise(promise);
        handle.promise().reference();
    }
    ~Coroutine() {
        if(handle) {
            if(handle.promise().unreference()) {
                handle.destroy();
            }
        }
    }

    Coroutine(const Coroutine& src) = delete;
    Coroutine& operator=(const Coroutine& src) = delete;

    Coroutine dup() const {
        return Coroutine{handle};
    }

    Coroutine(Coroutine&& src) {
        handle = src.handle;
        src.handle = null;
    }
    Coroutine& operator=(Coroutine&& src) {
        this->~Coroutine();
        handle = src.handle;
        src.handle = null;
        return *this;
    }

    void wait() {
        await_resume();
    }
    bool done() {
        return await_ready();
    }

    std::coroutine_handle<> await_suspend(std::coroutine_handle<> next) {
        // suspend this with dependency on next
        return handle;
    }
    void resume() {
        assert(handle);
        handle.resume();
    }
    void await_resume() {
        assert(handle);
        handle.promise().await_resume();
    }
    bool await_ready() {
        assert(handle);
        return handle.promise().await_ready();
    }

private:
    std::coroutine_handle<Promise<void, A>> handle;

    template<typename U, Allocator B>
    friend struct Promise;
};

template<Allocator A>
struct Promise<void, A> {
    Promise() = default;
    ~Promise() = default;

    Promise(const Promise& src) = delete;
    Promise& operator=(const Promise& src) = delete;

    Promise(Promise&& src) = delete;
    Promise& operator=(Promise&& src) = delete;

    Coroutine<void> get_return_object() {
        return Coroutine{*this};
    }

    std::suspend_never initial_suspend() noexcept {
        return {};
    }
    std::suspend_always final_suspend() noexcept {
        return {};
    }

    void return_void() noexcept {
        promise.fill();
    }

    void unhandled_exception() noexcept {
        die("Unhandled exception in coroutine.");
    }

    void* operator new(std::size_t size) noexcept {
        return A::alloc(size);
    }

    void operator delete(void* ptr, std::size_t size) noexcept {
        A::free(ptr);
    }

    void await_resume() noexcept {
        promise.wait();
    }
    bool await_ready() noexcept {
        return promise.ready();
    }

private:
    void reference() {
        references.incr();
    }
    bool unreference() {
        return references.decr() == 0;
    }

    Thread::Promise<void> promise;
    Thread::Atomic references;

    template<typename U, Allocator B>
    friend struct Coroutine;
};

} // namespace rpp::Async
