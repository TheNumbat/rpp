
#include "test.h"

#include <rpp/base.h>
#include <rpp/thread.h>

using Alloc = Mallocator<"Alloc">;

struct Destroy {
    ~Destroy() {
        info("Log at static destruction.");
        Profile::finalize();
    }
};
Destroy at_exit;

Test test{"static"_v};

Vec<i32, Alloc> g_vec0;
Vec<i32, Alloc> g_vec1{1, 2, 3};

i32 main() {
    Profile::begin_frame();

    static Vec<i32, Alloc> vec{1, 2, 3};
    static Box<i32, Alloc> box{5};

    static thread_local Vec<i32, Alloc> tls_vec{1, 2, 3};
    static thread_local Box<i32, Alloc> tls_box{5};

    info("g_vec0: %", g_vec0);
    info("g_vec1: %", g_vec1);
    info("vec: %", vec);
    info("box: %", box);
    info("tls_vec: %", tls_vec);
    info("tls_box: %", tls_box);

    auto v = Thread::spawn([]() {
        tls_vec = Vec<i32, Alloc>{1, 2, 3, 4, 5};
        info("tls_vec in thread: %", tls_vec);
        info("tls_box in thread: %", tls_box);
        return move(tls_vec);
    });
    info("Thread returned: %", v->block());

    Profile::end_frame();
    return 0;
}
