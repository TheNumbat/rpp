
#include "test.h"

#include <rpp/async.h>
#include <rpp/heap.h>
#include <rpp/range_allocator.h>
#include <rpp/rc.h>
#include <rpp/variant.h>

struct Counting_Alloc {
    constexpr static Literal name = "Lifetime test";
    inline static i64 live = 0;

    static void* alloc(u64 size) noexcept {
        void* result = sys_alloc(size);
        if(result) live++;
        return result;
    }
    static void free(void* ptr) noexcept {
        if(!ptr) return;
        assert(live > 0);
        live--;
        sys_free(ptr);
    }
};

struct Tracked {
    Tracked(i32* drops, i32 value) noexcept : drops(drops), value(value) {
    }
    Tracked(Tracked&& src) noexcept : drops(src.drops), value(src.value) {
        src.drops = null;
    }
    ~Tracked() noexcept {
        if(drops) (*drops)++;
    }
    bool operator<(const Tracked& rhs) const noexcept {
        return value < rhs.value;
    }

    i32* drops;
    i32 value;
};

template<typename T>
void self_move(T& value) {
    T& alias = value;
    value = move(alias);
}

template<typename Container>
void container_assignment() {
    i32 drops = 0;
    {
        Container dst, src;
        dst.emplace(&drops, 1);
        src.emplace(&drops, 2);
        src.emplace(&drops, 3);
        dst = move(src);
        assert(drops == 1);
        assert(src.empty());
        assert(dst.length() == 2);
        for(const auto& value : dst) assert(value.value == 2 || value.value == 3);
        self_move(dst);
        assert(drops == 1);
        assert(dst.length() == 2);
        dst = {};
        assert(drops == 3);
        assert(dst.empty());
        dst = {};
        src.emplace(&drops, 4);
    }
    assert(drops == 4);
    assert(Counting_Alloc::live == 0);
}

Async::Task<i32, Counting_Alloc> completed_task(i32 value) {
    co_return value;
}

Async::Task<void, Counting_Alloc> suspended_task(i32* drops) {
    Tracked value{drops, 0};
    co_await Async::Suspend{};
}

i32 main() {
    Test test{"empty"_v};
    {
        i32 value = 7;
        Ref<i32> ref{value};
        self_move(ref);
        assert(ref.ok() && &*ref == &value);

        Rc<i32, Counting_Alloc> rc{7};
        self_move(rc);
        assert(rc.ok() && *rc == 7 && rc.references() == 1);
        Arc<i32, Counting_Alloc> arc{9};
        self_move(arc);
        assert(arc.ok() && *arc == 9 && arc.references() == 1);

        Opt<String<Counting_Alloc>> opt{"value"_v.string<Counting_Alloc>()};
        self_move(opt);
        assert(opt.ok() && opt->view() == "value"_v);
        opt = move(*opt);
        assert(opt.ok() && opt->view() == "value"_v);
        opt = "replacement"_v.string<Counting_Alloc>();
        assert(opt->view() == "replacement"_v);
        opt.clear();
        opt = "from empty"_v.string<Counting_Alloc>();
        assert(opt->view() == "from empty"_v);

        Variant<i32, f32> trivial{7};
        self_move(trivial);
        assert(trivial.index() == 0);
        assert(trivial.match([](auto v) { return static_cast<i32>(v); }) == 7);
        Variant<i32, String<Counting_Alloc>> variant{"variant"_v.string<Counting_Alloc>()};
        self_move(variant);
        assert(variant.index() == 1);
        assert(variant.match(
            Overload{[](i32) { return false; },
                     [](const String<Counting_Alloc>& v) { return v.view() == "variant"_v; }}));
    }
    assert(Counting_Alloc::live == 0);
    {
        Thread::Mutex first, second;
        {
            Thread::Lock dst{first}, src{second};
            dst = move(src);
            assert(first.try_lock());
            first.unlock();
            assert(!second.try_lock());
            self_move(dst);
            assert(!second.try_lock());
        }
        assert(first.try_lock());
        first.unlock();
        assert(second.try_lock());
        second.unlock();
    }
    {
        i32 drops = 0;
        {
            Box<Tracked, Counting_Alloc> dst{&drops, 1}, src{&drops, 2};
            dst = move(src);
            assert(drops == 1 && dst->value == 2 && !src.ok());
            self_move(dst);
            assert(drops == 1 && dst->value == 2);
            dst.emplace(&drops, 3);
            assert(drops == 2 && dst->value == 3);
            dst = {};
            assert(drops == 3 && !dst.ok());
            src.emplace(&drops, 4);
        }
        assert(drops == 4);
        assert(Counting_Alloc::live == 0);
    }
    {
        auto dst = "old"_v.string<Counting_Alloc>();
        auto src = "new"_v.string<Counting_Alloc>();
        dst = move(src);
        assert(dst.view() == "new"_v && src.empty());
        assert(Counting_Alloc::live == 1);
        self_move(dst);
        assert(dst.view() == "new"_v);
        dst = {};
        assert(dst.empty());
        assert(Counting_Alloc::live == 0);
        src = "reused"_v.string<Counting_Alloc>();
    }
    assert(Counting_Alloc::live == 0);

    container_assignment<Vec<Tracked, Counting_Alloc>>();
    container_assignment<Queue<Tracked, Counting_Alloc>>();
    container_assignment<Heap<Tracked, Counting_Alloc>>();

    {
        i32 drops = 0;
        Free_List<Tracked, Counting_Alloc> dst, src;
        dst.destroy(dst.make(&drops, 1));
        auto cached = src.make(&drops, 2);
        src.destroy(cached);
        assert(drops == 2 && Counting_Alloc::live == 2);
        dst = move(src);
        assert(Counting_Alloc::live == 1);
        self_move(dst);
        auto reused = dst.make(&drops, 3);
        assert(reused == cached);
        dst.destroy(reused);
        dst = {};
        assert(drops == 3 && Counting_Alloc::live == 0);
        src.destroy(src.make(&drops, 4));
        assert(drops == 4);
    }
    assert(Counting_Alloc::live == 0);
    {
        Range_Allocator<Counting_Alloc> dst(1024), src(4096);
        auto allocated = *src.allocate(64, 16);
        dst = move(src);
        assert(src.statistics().total_capacity == 0);
        assert(dst.statistics().total_capacity == 4096);
        dst.free(allocated);
        dst.statistics().assert_clear();
        self_move(dst);
        allocated = *dst.allocate(128, 32);
        dst.free(allocated);
        Range_Allocator<Counting_Alloc> moved{move(dst)};
        allocated = *moved.allocate(256, 64);
        moved.free(allocated);
        moved.statistics().assert_clear();
        moved = {};
        assert(Counting_Alloc::live == 0);
        src = Range_Allocator<Counting_Alloc>{2048};
        allocated = *src.allocate(32, 8);
        src.free(allocated);
        src.statistics().assert_clear();
    }
    assert(Counting_Alloc::live == 0);
    {
        bool first_finished = false, second_finished = false;
        Thread::Thread dst{[&]() { first_finished = true; }};
        Thread::Thread src{[&]() { second_finished = true; }};
        dst = move(src);
        assert(first_finished);
        self_move(dst);
        dst.join();
        assert(second_finished);
        dst = {};
        src = Thread::Thread{[]() {}};
        src.join();
    }
    {
        auto dst = completed_task(1), src = completed_task(2);
        dst = move(src);
        assert(!src.ok() && dst.block() == 2);
        self_move(dst);
        assert(dst.block() == 2);
        dst = {};
        assert(!dst.ok());
        src = completed_task(3);
        assert(src.block() == 3);
    }
    assert(Counting_Alloc::live == 0);
    {
        i32 drops = 0;
        auto src = suspended_task(&drops);
        Async::Task<void, Counting_Alloc> dst;
        dst = move(src);
        self_move(dst);
        assert(!src.ok() && !dst.done() && drops == 0);
        dst.resume();
        dst.block();
        assert(drops == 1);
        dst = {};
    }
    assert(Counting_Alloc::live == 0);
    {
        Async::Event dst, src;
        dst = move(src);
        self_move(dst);
        dst.signal();
        assert(Async::Event::wait_any(Slice<Async::Event>{&dst, 1}) == 0);
        dst.reset();
        src = Async::Event{};
        dst = {};
        dst.signal();
        assert(Async::Event::wait_any(Slice<Async::Event>{&dst, 1}) == 0);
        dst.reset();
    }
    return 0;
}
