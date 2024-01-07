
#include "test.h"
#include <rc.h>

i32 main() {
    Test test{"empty"_v};
    Trace("Ref0/1") {
        i32 i = 5;
        Ref<i32> r{i};

        assert(*r == 5);
        i = 10;

        Ref<Ref<i32>> rr{r};
        assert(**rr == 10);
    }
    Trace("Box") {
        Box<i32> b{5};
        assert(b && *b == 5);

        Box<i32> b2;
        assert(!b2);

        b2 = move(b);
        assert(!b && b2 && *b2 == 5);

        Box<i32> b3 = b2.clone();
        assert(b2 && b3 && *b3 == 5);

        Box<String_View> sv{"Hello"_v};
        Box<String_View> sv2 = sv.clone();
        assert(sv && sv2 && *sv == *sv2 && *sv2 == "Hello"_v);

        Box<Thread::Mutex> m;
        m.emplace();
    }
    Trace("Rc") {
        Rc<i32> r0;
        assert(!r0);

        Rc<i32> r1{5};
        assert(r1 && *r1 == 5);
        assert(r1.references() == 1);

        Rc<i32> r2 = r1.dup();
        assert(r1 && r2 && *r1 == 5 && *r2 == 5);
        assert(r1.references() == 2 && r2.references() == 2);

        Rc<i32> r3 = move(r2);
        assert(r1 && r3 && *r1 == 5 && *r3 == 5);
        assert(r1.references() == 2 && r3.references() == 2);
        assert(!r2 && r2.references() == 0);
    }
    Trace("Arc") {
        Arc<i32> r0;
        assert(!r0);

        Arc<i32> r1{5};
        assert(r1 && *r1 == 5);
        assert(r1.references() == 1);

        Arc<i32> r2 = r1.dup();
        assert(r1 && r2 && *r1 == 5 && *r2 == 5);
        assert(r1.references() == 2 && r2.references() == 2);

        Arc<i32> r3 = move(r2);
        assert(r1 && r3 && *r1 == 5 && *r3 == 5);
        assert(r1.references() == 2 && r3.references() == 2);
        assert(!r2 && r2.references() == 0);
    }
    return 0;
}
