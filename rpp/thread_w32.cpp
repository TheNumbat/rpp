
#include "base.h"
#include "thread.h"

#include <windows.h>

namespace rpp::Thread {

u64 id_len() {
    return 5;
}

void sleep(u64 ms) {
    Sleep(static_cast<DWORD>(ms));
}

Id this_id() {
    static thread_local Id cache = (Id)GetCurrentThreadId();
    return cache;
}

u64 perf_counter() {
    LARGE_INTEGER li;
    QueryPerformanceCounter(&li);
    return li.QuadPart;
}

static u64 get_perf_frequency() {
    LARGE_INTEGER li;
    bool ret = QueryPerformanceFrequency(&li);
    assert(ret);
    return li.QuadPart;
}

u64 perf_frequency() {
    static u64 cache = get_perf_frequency();
    return cache;
}

u64 hardware_threads() {
    SYSTEM_INFO info;
    GetNativeSystemInfo(&info);
    return static_cast<u64>(info.dwNumberOfProcessors);
}

void set_priority(Priority p) {
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

void set_affinity(u64 core) {
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

Mutex::Mutex() {
    InitializeSRWLock(reinterpret_cast<PSRWLOCK>(&lock_));
}

Mutex::~Mutex() {
    lock_ = null;
}

void Mutex::lock() {
    AcquireSRWLockExclusive(reinterpret_cast<PSRWLOCK>(&lock_));
}

void Mutex::unlock() {
    ReleaseSRWLockExclusive(reinterpret_cast<PSRWLOCK>(&lock_));
}

bool Mutex::try_lock() {
    return TryAcquireSRWLockExclusive(reinterpret_cast<PSRWLOCK>(&lock_));
}

i64 Atomic::load() const {
    return value_;
}

i64 Atomic::incr() {
    return InterlockedIncrement64(&value_);
}

i64 Atomic::decr() {
    return InterlockedDecrement64(&value_);
}

void Atomic::store(i64 value) {
    InterlockedExchange64(&value_, value);
}

i64 Atomic::compare_and_swap(i64 compare_with, i64 set_to) {
    return InterlockedCompareExchange64(&value_, set_to, compare_with);
}

Cond::Cond() {
    InitializeConditionVariable(reinterpret_cast<PCONDITION_VARIABLE>(&cond_));
}

Cond::~Cond() {
    cond_ = CONDITION_VARIABLE_INIT;
}

void Cond::wait(Mutex& mut) {
    bool ret = SleepConditionVariableSRW(reinterpret_cast<PCONDITION_VARIABLE>(&cond_),
                                         reinterpret_cast<PSRWLOCK>(&mut.lock_), INFINITE, 0);
    if(!ret) {
        die("Failed to wait on cond: %", Log::sys_error());
    }
}

void Cond::signal() {
    WakeConditionVariable(reinterpret_cast<PCONDITION_VARIABLE>(&cond_));
}

void Cond::broadcast() {
    WakeAllConditionVariable(reinterpret_cast<PCONDITION_VARIABLE>(&cond_));
}

Id sys_id(OS_Thread thread_) {
    HANDLE thread = reinterpret_cast<HANDLE>(thread_);
    assert(thread != INVALID_HANDLE_VALUE);
    return GetThreadId(thread);
}

void sys_join(OS_Thread thread_) {
    Id id = sys_id(thread_);
    assert(id != this_id());

    HANDLE thread = reinterpret_cast<HANDLE>(thread_);
    if(thread == INVALID_HANDLE_VALUE) return;

    DWORD ret = WaitForSingleObjectEx(thread, INFINITE, false);
    if(ret != WAIT_OBJECT_0) {
        die("Unexpected wait return when joining thread %: %", id, Log::sys_error());
    }

    bool ok = CloseHandle(thread);
    assert(ok);
}

void sys_detach(OS_Thread thread_) {
    HANDLE thread = reinterpret_cast<HANDLE>(thread_);
    if(thread == INVALID_HANDLE_VALUE) return;
    bool ret = CloseHandle(thread);
    assert(ret);
    thread = INVALID_HANDLE_VALUE;
}

OS_Thread sys_start(OS_Thread_Ret (*f)(void*), void* data) {
    HANDLE thread = INVALID_HANDLE_VALUE;
    DWORD id = 0;
    thread = CreateThread(null, 0, reinterpret_cast<LPTHREAD_START_ROUTINE>(f), data, 0, &id);
    if(!thread) {
        die("Failed to create thread: %", Log::sys_error());
    }
    return reinterpret_cast<void*>(thread);
}

} // namespace rpp::Thread
