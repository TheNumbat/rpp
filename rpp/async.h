
#pragma once

#include "base.h"
#include "thread.h"

#include <coroutine>

namespace rpp::Async {

using Alloc = Mallocator<"Async">;

template<typename T>
struct Promise;

template<typename T>
struct Coroutine;

template<typename T>
struct Coroutine {

    using promise_type = Promise<T>;

    explicit Coroutine(std::coroutine_handle<Promise<T>> handle_) {
        handle = handle_;
        handle.promise().reference();
    }
    explicit Coroutine(Promise<T>& promise) {
        handle = std::coroutine_handle<Promise<T>>::from_promise(promise);
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

    void resume() {
        assert(handle);
        handle.resume();
    }
    void await_suspend(std::coroutine_handle<> self) {
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
    std::coroutine_handle<Promise<T>> handle;

    template<typename U>
    friend struct Promise;
};

template<typename T>
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

    std::suspend_always initial_suspend() noexcept {
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
        return Alloc::alloc(size);
    }

    void operator delete(void* ptr, std::size_t size) noexcept {
        Alloc::free(ptr);
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

    template<typename U>
    friend struct Coroutine;
};

template<>
struct Coroutine<void> {

    using promise_type = Promise<void>;

    explicit Coroutine(std::coroutine_handle<Promise<void>> promise);
    explicit Coroutine(Promise<void>& promise);
    ~Coroutine();

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

    void await_suspend(std::coroutine_handle<> self) {
    }
    void resume();
    void await_resume();
    bool await_ready();

private:
    std::coroutine_handle<Promise<void>> handle;

    template<typename U>
    friend struct Promise;
};

template<>
struct Promise<void> {
    Promise() = default;
    ~Promise() = default;

    Promise(const Promise& src) = delete;
    Promise& operator=(const Promise& src) = delete;

    Promise(Promise&& src) = delete;
    Promise& operator=(Promise&& src) = delete;

    Coroutine<void> get_return_object() {
        return Coroutine{*this};
    }

    std::suspend_always initial_suspend() noexcept {
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
        return Alloc::alloc(size);
    }

    void operator delete(void* ptr, std::size_t size) noexcept {
        Alloc::free(ptr);
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

    template<typename U>
    friend struct Coroutine;
};

inline Coroutine<void>::Coroutine(std::coroutine_handle<Promise<void>> handle_) {
    handle = handle_;
    handle.promise().reference();
}
inline Coroutine<void>::Coroutine(Promise<void>& promise) {
    handle = std::coroutine_handle<Promise<void>>::from_promise(promise);
    handle.promise().reference();
}
inline Coroutine<void>::~Coroutine() {
    if(handle) {
        if(handle.promise().unreference()) {
            handle.destroy();
        }
    }
}
inline void Coroutine<void>::await_resume() {
    assert(handle);
    handle.promise().await_resume();
}
inline bool Coroutine<void>::await_ready() {
    assert(handle);
    return handle.promise().await_ready();
}
inline void Coroutine<void>::resume() {
    assert(handle);
    handle.resume();
}

} // namespace rpp::Async
