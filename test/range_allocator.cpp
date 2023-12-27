
#include "test.h"

#include <range_allocator.h>

i32 main() {

    Profile::start_thread();
    Profile::begin_frame();

    {
        Test test{"empty"_v};

        Range_Allocator allocator(Math::GB(8));

        RNG::Stream rng(0);

        for(u64 i = 0; i < 100; i++) {
            Region_Scope(R);
            Vec<Range_Allocator<>::Range, Mregion<R>> allocations(1000), leaks(1000);
            for(u64 j = 0; j < 1000; j++) {
                u64 align = rng.range(static_cast<u64>(1), Math::MB(8));
                u64 size = rng.range(static_cast<u64>(1), Math::MB(8));
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

    Profile::end_frame();
    Profile::finalize();

    return 0;
}
