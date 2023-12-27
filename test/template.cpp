
#include "test.h"

i32 main() {

    Profile::start_thread();
    Profile::begin_frame();

    {
        Test test{"empty"_v};
        //
    }

    Profile::end_frame();
    Profile::finalize();

    return 0;
}
