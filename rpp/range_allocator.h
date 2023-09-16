
#pragma once

#include "base.h"

namespace rpp {

template<Allocator A, u64 Buckets = 24, u64 Bias = 8>
struct Range_Allocator {

    explicit Range_Allocator(u64 heap_size);
    ~Range_Allocator();

    Range_Allocator(const Range_Allocator&) = delete;
    Range_Allocator& operator=(const Range_Allocator&) = delete;
    Range_Allocator(Range_Allocator&&);
    Range_Allocator& operator=(Range_Allocator&&);

    struct Block {
        u64 offset;
        u64 len() const {
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

    Opt<Range> allocate(u64 size, u64 alignment);
    void free(Range range);

    struct Stats {
        u64 free_size = 0;
        u64 allocated_size = 0;
        u64 free_blocks = 0;
        u64 allocated_blocks = 0;
        u64 bucket_sizes[Buckets] = {};

        u64 total_frees = 0;
        u64 total_allocs = 0;
        u64 total_free_size = 0;
        u64 total_alloc_size = 0;
        u64 total_capacity = 0;

        void assert_clear();
    };

    Stats statistics();
    static void test();

private:
    Thread::Mutex mutex;
    Free_List<Block, A> blocks;
    Array<Block*, Buckets> free_blocks;
    Stats stats;

    u8 size_to_idx(u64 size);
    void insert_free_block(Block* block);
    void remove_free_block(Block* block);
    void reset();
};

} // namespace rpp

#include "range_allocator.inl"
