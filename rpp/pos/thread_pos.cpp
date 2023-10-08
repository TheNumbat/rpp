
#include "../thread.h"

#include <unistd.h>

namespace rpp::Thread {

static_assert(sizeof(Id) == sizeof(pthread_t), "Id != pthread_t");

void sleep(u64 ms) {
    usleep(ms * 1000);
}

u64 id_len() {
    return 15;
}

Id this_id() {
    return (Id)pthread_self();
}

u64 perf_counter() {
    u64 ticks;
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    ticks = now.tv_sec;
    ticks *= 1000000000;
    ticks += now.tv_nsec;
    return ticks;
}

u64 perf_frequency() {
    return 1000000000;
}

u64 hardware_threads() {
    long ret = sysconf(_SC_NPROCESSORS_ONLN);
    assert(ret > 0);
    return static_cast<u64>(ret);
}

void set_priority(Priority p) {

    pthread_t id = pthread_self();
    pthread_attr_t attr;
    int policy = 0, min_prio = 0, max_prio = 0, prio = 0;

    int ret = pthread_attr_init(&attr);
    if(ret) {
        die("Failed to init pthread attribute: %", ret);
    }
    ret = pthread_attr_getschedpolicy(&attr, &policy);
    if(ret) {
        die("Failed to get scheduler policy: %", ret);
    }

    min_prio = sched_get_priority_min(policy);
    max_prio = sched_get_priority_max(policy);
    if(min_prio == -1 || max_prio == -1) {
        die("Failed to get scheduler min/max priority: %", errno);
    }

    switch(p) {
    case Priority::low: prio = min_prio; break;
    case Priority::normal:
#ifdef __APPLE__
        if(min_prio == 15 && max_prio == 47) {
            prio = 37;
        }
#else
        prio = min_prio + (max_prio - min_prio) / 2;
#endif
        break;
    case Priority::high:
#ifdef __APPLE__
        if(min_prio == 15 && max_prio == 47) {
            prio = 45;
        }
#else
        prio = min_prio + (max_prio - min_prio) / 2 + (max_prio - min_prio) / 4;
#endif
        break;
    case Priority::critical: prio = max_prio; break;
    default: UNREACHABLE;
    }

    ret = pthread_setschedprio(id, prio);
    if(ret) {
        die("Failed set scheduler priority: %", ret);
    }
    ret = pthread_attr_destroy(&attr);
    if(ret) {
        die("Failed to destroy pthread attribute: %", ret);
    }
}

void set_affinity(u64 core) {
    assert(core < hardware_threads());
    cpu_set_t set = {};
    CPU_SET(static_cast<int>(core), &set);
    int ret = pthread_setaffinity_np(pthread_self(), sizeof(set), &set);
    if(ret) {
        die("Failed to set pthread affinity to %: %", core, ret);
    }
}

Mutex::Mutex() {
    int ret = pthread_mutex_init(&lock_, null);
    if(ret) {
        die("Failed to create mutex: %", ret);
    }
}

Mutex::~Mutex() {
    int ret = pthread_mutex_destroy(&lock_);
    if(ret) {
        die("Failed to destroy mutex: %", ret);
    }
}

void Mutex::lock() {
    int ret = pthread_mutex_lock(&lock_);
    if(ret) {
        die("Failed to lock mutex: %", ret);
    }
}

void Mutex::unlock() {
    int ret = pthread_mutex_unlock(&lock_);
    if(ret) {
        die("Failed to unlock mutex: %", ret);
    }
}

bool Mutex::try_lock() {
    int ret = pthread_mutex_trylock(&lock_);
    if(ret == EBUSY)
        return false;
    else if(ret)
        die("Failed to try lock mutex: %", ret);
    return true;
}

i64 Atomic::load() const {
    return __atomic_load_n(&value_, __ATOMIC_SEQ_CST);
}

i64 Atomic::incr() {
    return __atomic_fetch_add(&value_, 1, __ATOMIC_SEQ_CST) + 1;
}

i64 Atomic::decr() {
    return __atomic_fetch_sub(&value_, 1, __ATOMIC_SEQ_CST) - 1;
}

i64 Atomic::exchange(i64 value) {
    return __atomic_exchange_n(&value_, value, __ATOMIC_SEQ_CST);
}

i64 Atomic::compare_and_swap(i64 compare_with, i64 set_to) {
    return __atomic_compare_exchange(&value_, &compare_with, &set_to, false, __ATOMIC_SEQ_CST,
                                     __ATOMIC_SEQ_CST);
}

Cond::Cond() {
    int ret = pthread_cond_init(&cond_, null);
    if(ret) {
        die("Failed to create condvar: %", ret);
    }
}

Cond::~Cond() {
    int ret = pthread_cond_destroy(&cond_);
    if(ret) {
        die("Failed to destroy condvar: %", ret);
    }
}

void Cond::wait(Mutex& mut) {
    int ret = pthread_cond_wait(&cond_, &mut.lock_);
    if(ret) {
        die("Failed to wait on cond: %", ret);
    }
}

void Cond::signal() {
    int ret = pthread_cond_signal(&cond_);
    if(ret) {
        die("Failed to signal cond: %", ret);
    }
}

void Cond::broadcast() {
    int ret = pthread_cond_broadcast(&cond_);
    if(ret) {
        die("Failed to broadcast cond: %", ret);
    }
}

Id sys_id(OS_Thread thread) {
    return static_cast<Id>(thread);
}

void sys_join(OS_Thread thread) {
    if(!thread) return;
    assert(!pthread_equal(pthread_self(), thread));
    int ret = pthread_join(thread, null);
    if(ret) {
        die("Failed to join thread: %", ret);
    }
}

void sys_detach(OS_Thread thread) {
    if(!thread) return;
    int ret = pthread_detach(thread);
    if(ret) {
        die("Failed to detatch thread: %", ret);
    }
}

OS_Thread sys_start(OS_Thread_Ret (*f)(void*), void* data) {
    OS_Thread thread = OS_Thread_Null;
    int ret = pthread_create(&thread, null, f, data);
    if(ret) {
        die("Failed to create thread: %", ret);
    }
    assert(thread);
    return thread;
}

} // namespace rpp::Thread
