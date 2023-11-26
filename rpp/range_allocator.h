
#pragma once

#include "base.h"
#include "rng.h"

namespace rpp {

template<Allocator A, u64 Buckets = 24, u64 Bias = 8>
struct Range_Allocator {

    Range_Allocator() = default;
    explicit Range_Allocator(u64 heap_size) {
        assert(heap_size);
        Block* primary = blocks.make(Block{0, 0, heap_size, null, null});
        insert_free_block(primary);
        stats.free_blocks = 1;
        stats.free_size = heap_size;
        stats.total_capacity = heap_size;
    }
    ~Range_Allocator() {
        Thread::Lock lock(mutex);
        reset();
    }

    Range_Allocator(const Range_Allocator&) = delete;
    Range_Allocator& operator=(const Range_Allocator&) = delete;

    Range_Allocator(Range_Allocator&& src) {
        *this = std::move(src);
    }
    Range_Allocator& operator=(Range_Allocator&& src) {
        this->~Range_Allocator();

        Thread::Lock lock(src.mutex);

        stats = src.stats;
        src.stats = {};
        free_blocks = src.free_blocks;
        for(u64 i = 0; i < Buckets; i++) src.free_blocks[i] = null;
        blocks = std::move(src.blocks);

        return *this;
    }

    struct Block {
        u64 offset;
        u64 length() const {
            return size - (offset - start);
        }
        u64 padding() const {
            return offset - start;
        }

    private:
        explicit Block(u64 start, u64 offset, u64 size, Block* next_block, Block* prev_block)
            : start(start), offset(offset), size(size), next_block(next_block),
              prev_block(prev_block){};

        u64 size;
        u64 start;
        Block* next_block;
        Block* prev_block;
        Block* next_free;
        Block* prev_free;
        bool free;

        friend struct Range_Allocator<A, Buckets, Bias>;
    };
    using Range = Block*;

    Opt<Range> allocate(u64 size, u64 alignment) {

        assert(size && alignment);

        Thread::Lock lock(mutex);

        // Find list that contains blocks of size 2^log2(size) to 2*2^log2(size)
        u8 idx = size_to_idx(size);
        while(idx < Buckets && !free_blocks[idx]) {
            idx++;
        }
        if(idx == Buckets) {
            return {};
        }

        // Find block of size that can fit this allocation
        Block* block = free_blocks[idx];
        u64 padding = 0;
        while(block) {
            padding = Math::align(block->start, alignment) - block->start;
            if(block->size >= size + padding) {
                break;
            }
            block = block->next_free;
            while(!block && idx < Buckets - 1) {
                block = free_blocks[++idx];
            }
        }

        if(!block) {
            return {};
        }

        // Remove block from free list
        remove_free_block(block);
        block->offset = block->start + padding;

        // Split block
        if(block->size > size + padding) {

            Block* new_block =
                blocks.make(Block{block->start + size + padding, 0, block->size - size - padding,
                                  block->next_block, block});
            block->size = size + padding;

            if(block->next_block) block->next_block->prev_block = new_block;
            block->next_block = new_block;

            // Add remaining space back to free list
            insert_free_block(new_block);

            stats.free_blocks += 1;
        }

        stats.free_size -= size + padding;
        stats.allocated_size += size + padding;
        stats.high_water = Math::max(stats.high_water, stats.allocated_size);
        stats.allocated_blocks += 1;
        stats.free_blocks -= 1;

        stats.total_allocs += 1;
        stats.total_alloc_size += size + padding;

        return Opt<Range>{block};
    }

    void free(Range allocation) {

        Thread::Lock lock(mutex);

        Block* block = allocation;
        Block* prev = block->prev_block;
        Block* next = block->next_block;
        u64 free_size = block->size;

        // Merge with prev
        if(prev && prev->free) {

            remove_free_block(prev);

            prev->size += block->size;
            prev->next_block = next;
            if(next) next->prev_block = prev;

            blocks.destroy(block);
            stats.free_blocks -= 1;

            block = prev;
        }

        // Merge with next
        if(next && next->free) {

            remove_free_block(next);

            block->size += next->size;
            block->next_block = next->next_block;
            if(next->next_block) next->next_block->prev_block = block;

            stats.free_blocks -= 1;

            blocks.destroy(next);
        }

        insert_free_block(block);

        stats.free_size += free_size;
        stats.allocated_size -= free_size;
        stats.allocated_blocks -= 1;
        stats.free_blocks += 1;

        stats.total_frees += 1;
        stats.total_free_size += free_size;
    }

    struct Stats {
        u64 free_size = 0;
        u64 allocated_size = 0;
        u64 free_blocks = 0;
        u64 allocated_blocks = 0;
        u64 bucket_sizes[Buckets] = {};
        u64 high_water = 0;

        u64 total_frees = 0;
        u64 total_allocs = 0;
        u64 total_free_size = 0;
        u64 total_alloc_size = 0;
        u64 total_capacity = 0;

        void assert_clear() {
            assert(total_allocs == total_frees);
            assert(total_alloc_size == total_free_size);
            assert(free_blocks == 1 || (free_blocks == 0 && total_capacity == 0));
            assert(free_size == total_capacity);
            assert(allocated_blocks == 0);
            assert(allocated_size == 0);
            u64 sum = 0;
            for(u64 i = 0; i < Buckets; i++) sum += bucket_sizes[i];
            assert(sum == free_blocks);
        }
    };

    Stats statistics() {
        Stats copy;
        {
            Thread::Lock lock(mutex);
            copy = stats;
        }
        return copy;
    }

    static void test() {

        Range_Allocator<A, Buckets, Bias> allocator(Math::GB(8));

        RNG::Stream rng(0);
        info("Testing allocator...");

        Region_Scope;
        for(u64 i = 0; i < 1000; i++) {
            Vec<Range> allocations, leaks;
            for(u64 j = 0; j < 1000; j++) {
                u64 align = rng.range(static_cast<u64>(1), Math::MB(1));
                u64 size = rng.range(static_cast<u64>(1), Math::MB(1));
                auto mem = *allocator.allocate(size, align);
                assert(mem->offset % align == 0);
                allocations.push(mem);
            }
            rng.shuffle(allocations);
            for(auto& mem : allocations) {
                allocator.free(mem);
            }
            allocator.statistics().assert_clear();
        }
    }

private:
    Thread::Mutex mutex;
    Free_List<Block, A> blocks;
    Array<Block*, Buckets> free_blocks;
    Stats stats;

    u8 size_to_idx(u64 size) {
        u64 idx = Math::log2(size);
        idx = idx >= Bias ? idx - Bias : 0;
        return static_cast<u8>(Math::min(idx, Buckets - 1ul));
    }

    void insert_free_block(Block* block) {
        block->free = true;
        u8 idx = size_to_idx(block->size);
        block->prev_free = null;
        block->next_free = free_blocks[idx];
        if(free_blocks[idx]) free_blocks[idx]->prev_free = block;
        free_blocks[idx] = block;
        stats.bucket_sizes[idx] += 1;
    }

    void remove_free_block(Block* block) {
        block->free = false;
        u8 idx = size_to_idx(block->size);
        if(block->prev_free) block->prev_free->next_free = block->next_free;
        if(block->next_free) block->next_free->prev_free = block->prev_free;
        if(free_blocks[idx] == block) free_blocks[idx] = block->next_free;
        block->next_free = null;
        block->prev_free = null;
        stats.bucket_sizes[idx] -= 1;
    }

    void reset() {
        for(u64 i = 0; i < Buckets; i++) {
            Block* block = free_blocks[i];
            while(block) {
                Block* next = block->next_free;
                blocks.destroy(block);
                block = next;
            }
            free_blocks[i] = null;
        }
        blocks.clear();
    }
};

} // namespace rpp
