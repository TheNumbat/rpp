
#include "test.h"

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
    return 0;
}
