
#include "test.h"

#include <rpp/rc.h>

static void region_alignment() {
    struct alignas(16) Aligned_Value {
        u64 words[2];
    };
    u64 initial_size = Region_Allocator::size();
    Region(R) {
        for(u64 size : {1u, 3u, 4u, 15u, 17u, 1001u, 4097u}) {
            auto bytes = Vec<u8, Mregion<R>>::make(size);
            auto values = Vec<Aligned_Value, Mregion<R>>::make(2);
            assert(reinterpret_cast<uptr>(bytes.data()) % alignof(Aligned_Value) == 0);
            assert(reinterpret_cast<uptr>(values.data()) % alignof(Aligned_Value) == 0);
            values[0] = {};
            u64 outer_size = Region_Allocator::size();
            Region(Nested) {
                auto nested = Vec<u8, Mregion<Nested>>::make(8193);
                auto aligned = Vec<Aligned_Value, Mregion<Nested>>::make(1);
                assert(reinterpret_cast<uptr>(aligned.data()) % alignof(Aligned_Value) == 0);
                aligned[0] = {};
            }
            assert(Region_Allocator::size() == outer_size);
        }
    }
    assert(Region_Allocator::size() == initial_size);
}

i32 main() {
    Profile::begin_frame();
    {
        Test test{"empty"_v};
        region_alignment();
        Region(R0) {
            auto v0 = Vec<u8, Mregion<R0>>::make(Math::MB(2));
            Region(R1) {
                auto v1 = Vec<u8, Mregion<R1>>::make(Math::MB(4));
                Region(R2) {
                    auto v2 = Vec<u8, Mregion<R2>>::make(256);
                    auto v3 = Vec<u8, Mregion<R2>>::make(256);
                    auto v4 = Vec<u8, Mregion<R2>>::make(256);
                    Region(R3) {
                        auto v5 = Vec<u8, Mregion<R3>>::make(256);
                        auto v6 = Vec<u8, Mregion<R3>>::make(256);
                        auto v7 = Vec<u8, Mregion<R3>>::make(Math::MB(2));
                    }
                }
            }
        }
        Trace("Alloc0") {
            using A = Mallocator<"Test">;
            void* ptr = A::alloc(100);
            A::free(ptr);

            {
                for(u64 i = 0; i < 100; i++) {
                    i32* n = Mpool::make<i32>();
                    Mpool::destroy(n);
                }

                Box<i32, Mpool> pool_box1{1};
                Box<i32, Mpool> pool_box2{2};
                Rc<i32, Mpool> pool_rc1{1};
                Rc<i32, Mpool> pool_rc2{2};
                Arc<i32, Mpool> pool_arc1{1};
                Arc<i32, Mpool> pool_arc2{2};
            }
            {
                for(u64 i = 0; i < 100; i++) {
                    auto* n = Mpool::make<Box<i32, A>>();
                    Mpool::destroy(n);
                }

                Box<Box<i32, Mpool>, Mpool> pool_box1{1};
                Box<Box<i32, Mpool>, Mpool> pool_box2{2};
                Rc<Box<i32, Mpool>, Mpool> pool_rc1{1};
                Rc<Box<i32, Mpool>, Mpool> pool_rc2{2};
                Arc<Box<i32, Mpool>, Mpool> pool_arc1{1};
                Arc<Box<i32, Mpool>, Mpool> pool_arc2{2};
            }
        }
    }
    Profile::end_frame();
    Profile::finalize();
    return 0;
}
