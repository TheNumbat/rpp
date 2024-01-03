
#include "../base.h"

#include <stdio.h>
#include <stdlib.h>

namespace rpp {

static Thread::Atomic g_net_allocs;

using Regions = Mallocator<"Regions", false>;

struct Chunk {
    Chunk* up = null;
    u64 size = 0;
    u64 used = 0;
};
static_assert(sizeof(Chunk) == 24);

constexpr u64 FIRST_CHUNK_SIZE = Math::KB(1);
constexpr u64 MAX_REGION_DEPTH = 128;

struct First_Chunk {
    Chunk chunk = {null, FIRST_CHUNK_SIZE - sizeof(Chunk), 0};
    u8 data[FIRST_CHUNK_SIZE - sizeof(Chunk)] = {};
};
static_assert(sizeof(First_Chunk) == FIRST_CHUNK_SIZE);

thread_local u64 current_region = 0;
thread_local u64 region_offsets[MAX_REGION_DEPTH] = {};
thread_local Region region_brands[MAX_REGION_DEPTH] = {};
thread_local First_Chunk first_chunk;
thread_local Chunk* chunks = &first_chunk.chunk;

static void new_chunk(u64 request) noexcept {
    u64 size = Math::max(request + sizeof(Chunk), 2 * (chunks->size + sizeof(Chunk)));
    Chunk* chunk = reinterpret_cast<Chunk*>(Regions::alloc(size));
    chunk->size = size - sizeof(Chunk);
    chunk->used = 0;
    chunk->up = chunks;
    chunks = chunk;
}

static void assert_brand(Region brand) noexcept {
    if(region_brands[current_region] != brand) {
        die("Region brand mismatch!");
    }
}

[[nodiscard]] void* Region_Allocator::alloc(Region brand, u64 size) noexcept {
    assert_brand(brand);
    if(chunks->size - chunks->used < size) {
        new_chunk(size);
    }
    u8* ret = reinterpret_cast<u8*>(chunks) + sizeof(Chunk) + chunks->used;
    chunks->used += size;
    region_offsets[current_region] += size;
    return ret;
}

void Region_Allocator::free(Region brand, void*) noexcept {
    assert_brand(brand);
}

void Region_Allocator::begin(Region brand) noexcept {
    current_region++;
    assert(current_region < MAX_REGION_DEPTH);
    region_offsets[current_region] = region_offsets[current_region - 1];
    region_brands[current_region] = brand;
}

void Region_Allocator::end(Region brand) noexcept {
    assert(current_region > 0);
    assert_brand(brand);
    u64 end_offset = region_offsets[current_region];
    current_region--;
    u64 start_offset = region_offsets[current_region];
    u64 free_size = end_offset - start_offset;
    while(free_size > chunks->used) {
        free_size -= chunks->used;
        Chunk* chunk = chunks;
        chunks = chunks->up;
        Regions::free(chunk);
    }
    chunks->used -= free_size;
    if(chunks->used == 0 && chunks->up != null) {
        Chunk* chunk = chunks;
        chunks = chunks->up;
        Regions::free(chunk);
    }
}

[[nodiscard]] u64 Region_Allocator::depth() noexcept {
    return current_region;
}

[[nodiscard]] u64 Region_Allocator::size() noexcept {
    return region_offsets[current_region];
}

[[nodiscard]] void* sys_alloc(u64 sz) noexcept {
    void* ret = malloc(sz);
    assert(ret);
#ifndef RPP_RELEASE_BUILD
    g_net_allocs.incr();
#endif
    return ret;
}

void sys_free(void* mem) noexcept {
    if(!mem) return;
#ifndef RPP_RELEASE_BUILD
    g_net_allocs.decr();
#endif
    free(mem);
}

[[nodiscard]] i64 sys_net_allocs() noexcept {
    return g_net_allocs.load();
}

} // namespace rpp