
#include "base.h"

namespace rpp {

static Thread::Atomic __allocs;

void* sys_alloc(u64 sz) {
    void* ret = calloc(sz, 1);
    assert(ret);
#ifndef RELEASE_BUILD
    __allocs.incr();
#endif
    return ret;
}

void sys_free(void* mem) {
    if(!mem) return;
#ifndef RELEASE_BUILD
    __allocs.decr();
#endif
    free(mem);
}

i64 sys_net_allocs() {
    return __allocs.load();
}

} // namespace rpp