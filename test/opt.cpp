
#include "test.h"

i32 main() {

    Profile::start_thread();
    Profile::begin_frame();

    {
        Test test{"empty"_v};
        {
            Prof_Scope("Storage");

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
        {
            Prof_Scope("Opt");

            Opt<i32> o;
            assert(!o);
            o = 5;
            assert(o);

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
    }

    Profile::end_frame();
    Profile::finalize();

    return 0;
}
