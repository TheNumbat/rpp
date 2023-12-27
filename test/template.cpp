
#include "test.h"

i32 main() {

    Profile::start_thread();
    Profile::begin_frame();

    {
        Test test{"template"_v};
        //
    }

    Profile::end_frame();
    Profile::finalize();

    return 0;
}
