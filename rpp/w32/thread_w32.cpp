
#include "../thread.h"

#include <windows.h>

namespace rpp::Thread {

[[nodiscard]] u64 id_len() noexcept {
    return 5;
}

void sleep(u64 ms) noexcept {
    Sleep(static_cast<DWORD>(ms));
}

[[nodiscard]] Id this_id() noexcept {
    static thread_local Id cache = (Id)GetCurrentThreadId();
    return cache;
}

void pause() noexcept {
    YieldProcessor();
}

[[nodiscard]] u64 perf_counter() noexcept {
    LARGE_INTEGER li;
    QueryPerformanceCounter(&li);
    return li.QuadPart;
}

[[nodiscard]] static u64 get_perf_frequency() noexcept {
    LARGE_INTEGER li;
    bool ret = QueryPerformanceFrequency(&li);
    assert(ret);
    return li.QuadPart;
}

[[nodiscard]] u64 perf_frequency() noexcept {
    static u64 cache = get_perf_frequency();
    return cache;
}

[[nodiscard]] u64 hardware_threads() noexcept {
    SYSTEM_INFO info;
    GetNativeSystemInfo(&info);
    return static_cast<u64>(info.dwNumberOfProcessors);
}

void set_priority(Priority p) noexcept {
    int value;
    switch(p) {
    case Priority::low: value = THREAD_PRIORITY_LOWEST; break;
    case Priority::normal: value = THREAD_PRIORITY_NORMAL; break;
    case Priority::high: value = THREAD_PRIORITY_HIGHEST; break;
    case Priority::critical: value = THREAD_PRIORITY_TIME_CRITICAL; break;
    default: UNREACHABLE;
    }
    bool ret = SetThreadPriority(GetCurrentThread(), value);
    if(!ret) {
        warn("Failed to set thread % to priority %", this_id(), p);
    }
}

void set_affinity(u64 core) noexcept {
    assert(core < hardware_threads());
    if(core >= 64) {
        warn("Can't set affinity for more than 64 cores!");
        return;
    }
    DWORD_PTR mask = 1ll << core;
    DWORD_PTR ret = SetThreadAffinityMask(GetCurrentThread(), mask);
    if(!ret) {
        warn("Failed to set thread affinity to %: %", core, Log::sys_error());
    }
}

static_assert(sizeof(SRWLOCK) == sizeof(void*));
static_assert(sizeof(CONDITION_VARIABLE) == sizeof(void*));
static_assert(sizeof(HANDLE) == sizeof(OS_Thread));

void Flag::block() noexcept {
    u16 zero = 0;
    while(value_ == 0) {
        bool ret = WaitOnAddress(&value_, &zero, sizeof(value_), INFINITE);
        if(!ret) {
            die("Failed to wait on address: %", Log::sys_error());
        }
    }
}

void Flag::signal() noexcept {
    InterlockedIncrement16(&value_);
    WakeByAddressAll(&value_);
}

[[nodiscard]] bool Flag::ready() noexcept {
    return value_ != 0;
}

Mutex::Mutex() noexcept {
    InitializeSRWLock(reinterpret_cast<PSRWLOCK>(&lock_));
}

Mutex::~Mutex() noexcept {
    lock_ = null;
}

void Mutex::lock() noexcept {
    AcquireSRWLockExclusive(reinterpret_cast<PSRWLOCK>(&lock_));
}

void Mutex::unlock() noexcept {
    ReleaseSRWLockExclusive(reinterpret_cast<PSRWLOCK>(&lock_));
}

[[nodiscard]] bool Mutex::try_lock() noexcept {
    return TryAcquireSRWLockExclusive(reinterpret_cast<PSRWLOCK>(&lock_));
}

[[nodiscard]] i64 Atomic::load() const noexcept {
    return value_;
}

i64 Atomic::incr() noexcept {
    return InterlockedIncrement64(&value_);
}

i64 Atomic::decr() noexcept {
    return InterlockedDecrement64(&value_);
}

i64 Atomic::exchange(i64 value) noexcept {
    return InterlockedExchange64(&value_, value);
}

[[nodiscard]] i64 Atomic::compare_and_swap(i64 compare_with, i64 set_to) noexcept {
    return InterlockedCompareExchange64(&value_, set_to, compare_with);
}

Cond::Cond() noexcept {
    InitializeConditionVariable(reinterpret_cast<PCONDITION_VARIABLE>(&cond_));
}

Cond::~Cond() noexcept {
    cond_ = CONDITION_VARIABLE_INIT;
}

void Cond::wait(Mutex& mut) noexcept {
    bool ret = SleepConditionVariableSRW(reinterpret_cast<PCONDITION_VARIABLE>(&cond_),
                                         reinterpret_cast<PSRWLOCK>(&mut.lock_), INFINITE, 0);
    if(!ret) {
        die("Failed to wait on cond: %", Log::sys_error());
    }
}

void Cond::signal() noexcept {
    WakeConditionVariable(reinterpret_cast<PCONDITION_VARIABLE>(&cond_));
}

void Cond::broadcast() noexcept {
    WakeAllConditionVariable(reinterpret_cast<PCONDITION_VARIABLE>(&cond_));
}

[[nodiscard]] Id sys_id(OS_Thread thread_) noexcept {
    HANDLE thread = reinterpret_cast<HANDLE>(thread_);
    assert(thread != INVALID_HANDLE_VALUE);
    return GetThreadId(thread);
}

void sys_join(OS_Thread thread_) noexcept {
    Id id = sys_id(thread_);
    assert(id != this_id());

    HANDLE thread = reinterpret_cast<HANDLE>(thread_);
    if(thread == INVALID_HANDLE_VALUE) return;

    DWORD ret = WaitForSingleObjectEx(thread, INFINITE, false);
    if(ret != WAIT_OBJECT_0) {
        die("Unexpected wait return when joining thread %: % (%)", id, static_cast<u32>(ret),
            Log::sys_error());
    }

    bool ok = CloseHandle(thread);
    assert(ok);
}

void sys_detach(OS_Thread thread_) noexcept {
    HANDLE thread = reinterpret_cast<HANDLE>(thread_);
    if(thread == INVALID_HANDLE_VALUE) return;
    bool ret = CloseHandle(thread);
    assert(ret);
    thread = INVALID_HANDLE_VALUE;
}

[[nodiscard]] OS_Thread sys_start(OS_Thread_Ret (*f)(void*), void* data) noexcept {
    HANDLE thread = INVALID_HANDLE_VALUE;
    DWORD id = 0;
    thread = CreateThread(null, 0, reinterpret_cast<LPTHREAD_START_ROUTINE>(f), data, 0, &id);
    if(!thread) {
        die("Failed to create thread: %", Log::sys_error());
    }
    return reinterpret_cast<void*>(thread);
}

} // namespace rpp::Thread
