
#pragma once

#include "base.h"
#include <windows.h>

namespace rpp::Thread {

u64 id_len() {
    return 5;
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

Mutex::Mutex() {
    InitializeSRWLock((PSRWLOCK)&lock_);
}

Mutex::~Mutex() {
    lock_ = null;
}

void Mutex::lock() {
    AcquireSRWLockExclusive((PSRWLOCK)&lock_);
}

void Mutex::unlock() {
    ReleaseSRWLockExclusive((PSRWLOCK)&lock_);
}

bool Mutex::try_lock() {
    return TryAcquireSRWLockExclusive((PSRWLOCK)&lock_);
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

} // namespace rpp::Thread
