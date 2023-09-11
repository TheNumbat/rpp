
#include "base.h"
#include <atomic>

namespace rpp {

static std::atomic<i64> __allocs;

void* sys_alloc(u64 sz) {
    void* ret = calloc(sz, 1);
    assert(ret);
#ifndef RELEASE_BUILD
    __allocs++;
#endif
    return ret;
}

void sys_free(void* mem) {
    if(!mem) return;
#ifndef RELEASE_BUILD
    __allocs--;
#endif
    free(mem);
}

i64 sys_net_allocs() {
    return __allocs.load();
}

} // namespace rpp