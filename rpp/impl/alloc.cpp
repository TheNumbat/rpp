
#include "../base.h"

#include <stdio.h>

#ifdef COMPILER_MSVC
void* operator new(std::size_t, std::align_val_t, void* ptr) noexcept {
    return ptr;
}
void* operator new[](std::size_t, std::align_val_t, void* ptr) noexcept {
    return ptr;
}
void operator delete(void*, std::align_val_t, void*) noexcept {
}
void operator delete[](void*, std::align_val_t, void*) noexcept {
}
#endif

namespace rpp {

static Thread::Atomic g_net_allocs;

thread_local u64 Mregion::current_region = 0;
thread_local u64 Mregion::current_offset = 0;
thread_local u64 Mregion::region_offsets[REGION_COUNT] = {};
thread_local u8* Mregion::stack_memory = null;

void Mregion::create() {
    stack_memory = reinterpret_cast<u8*>(calloc(REGION_STACK_SIZE, 1));
}

void Mregion::destroy() {
    ::free(stack_memory);
    stack_memory = null;
}

void* Mregion::alloc(u64 size) {
    if(!stack_memory) {
        ::printf("\033[0;31mAllocation in Mregion after shutdown!\033[0m\n");
        ::exit(1);
    }
    assert(current_offset + size < REGION_STACK_SIZE);
    void* ret = stack_memory + current_offset;
    current_offset += size;
    Std::memset(ret, 0, size);
    return ret;
}

void Mregion::free(void* mem) {
}

void Mregion::begin() {
    assert(current_region + 1 < REGION_COUNT);
    current_region++;
    region_offsets[current_region] = current_offset;
}

void Mregion::end() {
    assert(current_region > 0);
    current_offset = region_offsets[current_region];
    current_region--;
}

Mregion::Scope::Scope() {
    Mregion::begin();
}

Mregion::Scope::~Scope() {
    Mregion::end();
}

u64 Mregion::depth() {
    return current_region;
}

u64 Mregion::size() {
    return current_offset;
}

void* sys_alloc(u64 sz) {
    void* ret = calloc(sz, 1);
    assert(ret);
#ifndef RELEASE_BUILD
    g_net_allocs.incr();
#endif
    return ret;
}

void sys_free(void* mem) {
    if(!mem) return;
#ifndef RELEASE_BUILD
    g_net_allocs.decr();
#endif
    free(mem);
}

i64 sys_net_allocs() {
    return g_net_allocs.load();
}

} // namespace rpp