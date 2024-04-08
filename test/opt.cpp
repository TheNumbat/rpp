
#include "test.h"

struct InstCnt {
    InstCnt() {
        ++cnt;
    }
    ~InstCnt() {
        --cnt;
    }
    InstCnt(const InstCnt&) {
        ++cnt;
    }
    InstCnt(InstCnt&&) {
        ++cnt;
    }
    InstCnt& operator=(const InstCnt&) = delete;
    InstCnt& operator=(InstCnt&&) = delete;

    inline static i32 cnt = 0;
};

i32 main() {
    Test test{"empty"_v};
    Trace("Storage") {
        Storage<String<>> s;
        s.construct("Hello"_v.string());
        s.destruct();

        Storage<String<>> s2{"World"_v.string()};

        Storage<String<>> s3{s2->clone()};
        Storage<String<>> s4{move(*s3)};

        s2.destruct();
        s4.destruct();

        Storage<Thread::Mutex> m;
        m.construct();
        m->lock();
        m->unlock();
        m.destruct();
    }
    Trace("Opt") {
        Opt<i32> o;
        assert(!o.ok());
        o = 5;
        assert(o.ok());

        Opt<i32> o2 = o;
        Opt<i32> o3 = move(o2);

        Opt<String_View> os{"Hello"_v};
        Opt<String_View> os2 = os.clone();
        Opt<String_View> os3 = move(os2);

        String_View vv = move(*os3);
        assert(vv == "Hello"_v);

        assert(os->length() == 5);

        Opt<Thread::Mutex> m;
    }
    Trace("OptInstCnt") {
        Opt<InstCnt> c1;
        assert(InstCnt::cnt == 0);

        // Move assign value type.
        c1 = InstCnt{};
        assert(InstCnt::cnt == 1);
        c1 = InstCnt{};
        assert(InstCnt::cnt == 1);
        c1.clear();
        assert(InstCnt::cnt == 0);

        // Emplace value type.
        c1.emplace();
        assert(InstCnt::cnt == 1);
        c1.emplace();
        assert(InstCnt::cnt == 1);
        c1.clear();
        assert(InstCnt::cnt == 0);

        // Move construct from value type.
        Opt<InstCnt> c2(InstCnt{});
        assert(InstCnt::cnt == 1);

        // Move construct from Opt type, moves inner value.
        c1 = move(c2);
        assert(InstCnt::cnt == 2);
        c2.clear();
        assert(InstCnt::cnt == 1);
        c1.clear();
        assert(InstCnt::cnt == 0);

        // Move construct from Opt type.
        c1.emplace();
        Opt<InstCnt> c3(move(c1));
        assert(InstCnt::cnt == 2);

        c1.clear();
        assert(InstCnt::cnt == 1);
        c3.clear();
        assert(InstCnt::cnt == 0);

        // Clone and move assign Opt type.
        c1.emplace();
        c2 = c1.clone();
        assert(InstCnt::cnt == 2);
    }
    assert(InstCnt::cnt == 0);
    return 0;
}
