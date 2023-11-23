
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

using Regions = Mallocator<"Regions", false>;

struct Chunk {
    Chunk* up = null;
    u64 size = 0;
    u64 used = 0;
};
static_assert(sizeof(Chunk) == 24);

constexpr u64 MIN_CHUNK_SIZE = Math::MB(2);
constexpr u64 REGION_COUNT = 256;

thread_local u64 current_region = 0;
thread_local u64 region_offsets[REGION_COUNT] = {};
thread_local Chunk* chunks = null;

static void new_chunk(u64 request) {
    u64 size = Math::max(request + sizeof(Chunk), MIN_CHUNK_SIZE);
    Chunk* chunk = reinterpret_cast<Chunk*>(Regions::alloc(size));
    chunk->size = size - sizeof(Chunk);
    chunk->used = 0;
    chunk->up = chunks;
    chunks = chunk;
}

void* Mregion::alloc(u64 size) {
    if(!chunks || chunks->size - chunks->used < size) {
        new_chunk(size);
    }
    u8* ret = reinterpret_cast<u8*>(chunks) + sizeof(Chunk) + chunks->used;
    chunks->used += size;
    region_offsets[current_region] += size;
    Std::memset(ret, 0, size);
    return ret;
}

void Mregion::free(void* mem) {
}

void Mregion::begin() {
    current_region++;
    assert(current_region < REGION_COUNT);
    region_offsets[current_region] = region_offsets[current_region - 1];
}

void Mregion::end() {
    assert(current_region > 0);
    u64 end_offset = region_offsets[current_region];
    current_region--;
    u64 start_offset = region_offsets[current_region];
    u64 free_size = end_offset - start_offset;
    while(free_size && free_size >= chunks->used) {
        free_size -= chunks->used;
        Chunk* chunk = chunks;
        chunks = chunks->up;
        Regions::free(chunk);
    }
    if(free_size) {
        chunks->used -= free_size;
    }
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
    return region_offsets[current_region];
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