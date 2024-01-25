
#include "../thread.h"

#ifdef RPP_ARCH_X64
#include <immintrin.h>
#endif

#include <errno.h>
#include <linux/futex.h>
#include <string.h>
#include <sys/syscall.h>
#include <unistd.h>

namespace rpp::Thread {

[[nodiscard]] static String_View error(int code) noexcept {
    constexpr int buffer_size = 256;
    static thread_local char buffer[buffer_size];
    return String_View{strerror_r(code, buffer, buffer_size)};
}

static_assert(sizeof(Id) == sizeof(pthread_t), "Id != pthread_t");

void sleep(u64 ms) noexcept {
    usleep(ms * 1000);
}

[[nodiscard]] u64 id_len() noexcept {
    return 15;
}

[[nodiscard]] Id this_id() noexcept {
    return (Id)pthread_self();
}

void pause() noexcept {
#ifdef RPP_ARCH_X64
    _mm_pause();
#elif defined RPP_ARCH_ARM64
    asm volatile("yield");
#endif
}

[[nodiscard]] u64 perf_counter() noexcept {
    u64 ticks;
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    ticks = now.tv_sec;
    ticks *= 1000000000;
    ticks += now.tv_nsec;
    return ticks;
}

[[nodiscard]] u64 perf_frequency() noexcept {
    return 1000000000;
}

[[nodiscard]] u64 hardware_threads() noexcept {
    long ret = sysconf(_SC_NPROCESSORS_ONLN);
    assert(ret > 0);
    return static_cast<u64>(ret);
}

void set_priority(Priority p) noexcept {

    pthread_t id = pthread_self();
    pthread_attr_t attr;
    int policy = 0, min_prio = 0, max_prio = 0, prio = 0;

    int ret = pthread_attr_init(&attr);
    if(ret) {
        die("Failed to init pthread attribute: %", error(ret));
    }
    ret = pthread_attr_getschedpolicy(&attr, &policy);
    if(ret) {
        die("Failed to get scheduler policy: %", error(ret));
    }

    min_prio = sched_get_priority_min(policy);
    max_prio = sched_get_priority_max(policy);
    if(min_prio == -1 || max_prio == -1) {
        die("Failed to get scheduler min/max priority: %", error(errno));
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
    default: RPP_UNREACHABLE;
    }

    ret = pthread_setschedprio(id, prio);
    if(ret) {
        die("Failed set scheduler priority: %", error(ret));
    }
    ret = pthread_attr_destroy(&attr);
    if(ret) {
        die("Failed to destroy pthread attribute: %", error(ret));
    }
}

void set_affinity(u64 core) noexcept {
    assert(core < hardware_threads());
    cpu_set_t set = {};
    CPU_SET(static_cast<int>(core), &set);
    int ret = pthread_setaffinity_np(pthread_self(), sizeof(set), &set);
    if(ret) {
        die("Failed to set pthread affinity to %: %", core, error(ret));
    }
}

void Flag::block() noexcept {
    while(__atomic_load_n(&value_, __ATOMIC_SEQ_CST) == 0) {
        int ret = syscall(SYS_futex, &value_, FUTEX_WAIT, 0, NULL, NULL, 0);
        if(ret == -1 && errno != EAGAIN) {
            die("Failed to wait on futex: %", error(errno));
        }
    }
}

void Flag::signal() noexcept {
    __atomic_exchange_n(&value_, 1, __ATOMIC_SEQ_CST);
    int ret = syscall(SYS_futex, &value_, FUTEX_WAKE, RPP_INT32_MAX, NULL, NULL, 0);
    if(ret == -1) {
        die("Failed to wake futex: %", error(errno));
    }
}

[[nodiscard]] bool Flag::ready() noexcept {
    return __atomic_load_n(&value_, __ATOMIC_SEQ_CST) != 0;
}

Mutex::Mutex() noexcept {
    int ret = pthread_mutex_init(&lock_, null);
    if(ret) {
        die("Failed to create mutex: %", error(ret));
    }
}

Mutex::~Mutex() noexcept {
    int ret = pthread_mutex_destroy(&lock_);
    if(ret) {
        die("Failed to destroy mutex: %", error(ret));
    }
}

void Mutex::lock() noexcept {
    int ret = pthread_mutex_lock(&lock_);
    if(ret) {
        die("Failed to lock mutex: %", error(ret));
    }
}

void Mutex::unlock() noexcept {
    int ret = pthread_mutex_unlock(&lock_);
    if(ret) {
        die("Failed to unlock mutex: %", error(ret));
    }
}

[[nodiscard]] bool Mutex::try_lock() noexcept {
    int ret = pthread_mutex_trylock(&lock_);
    if(ret == EBUSY)
        return false;
    else if(ret)
        die("Failed to try lock mutex: %", error(ret));
    return true;
}

[[nodiscard]] i64 Atomic::load() const noexcept {
    return __atomic_load_n(&value_, __ATOMIC_SEQ_CST);
}

i64 Atomic::incr() noexcept {
    return __atomic_fetch_add(&value_, 1, __ATOMIC_SEQ_CST) + 1;
}

i64 Atomic::decr() noexcept {
    return __atomic_fetch_sub(&value_, 1, __ATOMIC_SEQ_CST) - 1;
}

i64 Atomic::exchange(i64 value) noexcept {
    return __atomic_exchange_n(&value_, value, __ATOMIC_SEQ_CST);
}

[[nodiscard]] i64 Atomic::compare_and_swap(i64 compare_with, i64 set_to) noexcept {
    __atomic_compare_exchange_n(&value_, &compare_with, set_to, false, __ATOMIC_SEQ_CST,
                                __ATOMIC_SEQ_CST);
    return compare_with;
}

Cond::Cond() noexcept {
    int ret = pthread_cond_init(&cond_, null);
    if(ret) {
        die("Failed to create condvar: %", error(ret));
    }
}

Cond::~Cond() noexcept {
    int ret = pthread_cond_destroy(&cond_);
    if(ret) {
        die("Failed to destroy condvar: %", error(ret));
    }
}

void Cond::wait(Mutex& mut) noexcept {
    int ret = pthread_cond_wait(&cond_, &mut.lock_);
    if(ret) {
        die("Failed to wait on cond: %", error(ret));
    }
}

void Cond::signal() noexcept {
    int ret = pthread_cond_signal(&cond_);
    if(ret) {
        die("Failed to signal cond: %", error(ret));
    }
}

void Cond::broadcast() noexcept {
    int ret = pthread_cond_broadcast(&cond_);
    if(ret) {
        die("Failed to broadcast cond: %", error(ret));
    }
}

[[nodiscard]] Id sys_id(OS_Thread thread) noexcept {
    return static_cast<Id>(thread);
}

void sys_join(OS_Thread thread) noexcept {
    if(!thread) return;
    assert(!pthread_equal(pthread_self(), thread));
    int ret = pthread_join(thread, null);
    if(ret) {
        die("Failed to join thread: %", error(ret));
    }
}

void sys_detach(OS_Thread thread) noexcept {
    if(!thread) return;
    int ret = pthread_detach(thread);
    if(ret) {
        die("Failed to detatch thread: %", error(ret));
    }
}

[[nodiscard]] OS_Thread sys_start(OS_Thread_Ret (*f)(void*), void* data) noexcept {
    OS_Thread thread = OS_Thread_Null;
    int ret = pthread_create(&thread, null, f, data);
    if(ret) {
        die("Failed to create thread: %", error(ret));
    }
    assert(thread);
    return thread;
}

} // namespace rpp::Thread
