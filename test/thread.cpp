
#include "test.h"

#include <thread.h>

i32 main() {

    Profile::start_thread();
    Profile::begin_frame();

    {
        Test test{"thread"_v};

        {
            Prof_Scope("Thread0");

            Thread::Mutex mut;
            mut.lock();
            mut.unlock();
            { Thread::Lock lock{mut}; }
        }
        {
            auto value = Thread::spawn([]() { return Vec<i32>{2, 3}; });

            auto v = Thread::spawn([]() { info("Hello from thread2"); });

            auto gone0 = Thread::spawn([]() {});
            (void)gone0;

            auto gone = Thread::Thread{[]() {}};
            (void)gone;

            v->block();
            info("Thread 1 returned %", value->block());
        }
        {
            Vec<Thread::Future<void>> tasks;
            for(u64 i = 0; i < Thread::hardware_threads(); i++) {
                tasks.push(Thread::spawn([]() { info("Hello from thread"); }));
            }

            for(auto& task : tasks) {
                task->block();
            }
        }
    }

    Profile::end_frame();
    Profile::finalize();

    return 0;
}
