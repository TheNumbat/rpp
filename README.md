# rpp

[![Windows](https://github.com/TheNumbat/rpp/actions/workflows/windows.yml/badge.svg)](https://github.com/TheNumbat/rpp/actions/workflows/windows.yml)
[![Linux](https://github.com/TheNumbat/rpp/actions/workflows/linux.yml/badge.svg)](https://github.com/TheNumbat/rpp/actions/workflows/linux.yml)
[![Linux (aarch64)](https://github.com/TheNumbat/rpp/actions/workflows/linux-arm64.yml/badge.svg)](https://github.com/TheNumbat/rpp/actions/workflows/linux-arm64.yml)
[![macOS](https://github.com/TheNumbat/rpp/actions/workflows/macos.yml/badge.svg)](https://github.com/TheNumbat/rpp/actions/workflows/macos.yml)
[![macOS (aarch64)](https://github.com/TheNumbat/rpp/actions/workflows/macos-arm64.yml/badge.svg)](https://github.com/TheNumbat/rpp/actions/workflows/macos-arm64.yml)

Minimal Rust-inspired C++20 STL replacement.
Refer to the [blog post](https://thenumb.at/rpp/) for details.

Goals:
- Fast Compilation
- Debuggability
- High Performance
- Explicit Code
- Easy Metaprogramming

## Integration

To use rpp in your project, run the following command (or manually download the source):

```bash
git submodule add https://github.com/TheNumbat/rpp
```

Then add the following lines to your CMakeLists.txt:

```cmake
add_subdirectory(rpp)
target_link_libraries($your_target PRIVATE rpp)
target_include_directories($your_target PRIVATE ${RPP_INCLUDE_DIRS})
```

To use rpp with another build system, add `rpp` to your include path, add `rpp/rpp/impl/unify.cpp` to the build, and add either `rpp/rpp/pos/unify.cpp` or `rpp/rpp/w32/unify.cpp` based on your platform.

## Platform Support

The following configurations are supported:

| OS      | Compiler    | Arch         |
|---------|-------------|--------------|
| Windows | MSVC 19.39+ | x64          |
| Linux   | Clang 17+   | x64, aarch64 |
| macOS   | Clang 17+   | x64, aarch64 |

Except for MSVC on Windows, the [gcc vector extensions](https://gcc.gnu.org/onlinedocs/gcc/Vector-Extensions.html) (also [implemented by clang](https://clang.llvm.org/docs/LanguageExtensions.html#vectors-and-extended-vectors)) are used to emit SIMD operations.
On Linux, other architectures should therefore work, but they have not been tested.

Other configurations (GCC, etc.) may be added in the future.

## Examples

### Logging

```cpp
#include <rpp/base.h>

using namespace rpp;

i32 main() {
    assert(true);
    info("Information");
    warn("Warning");
    die("Fatal error (exits)");
}
```

### Data Structures

```cpp
#include <rpp/base.h>
#include <rpp/rc.h>
#include <rpp/stack.h>
#include <rpp/heap.h>
#include <rpp/tuple.h>
#include <rpp/variant.h>

using namespace rpp;

i32 main() {
    Ref<i32> ref;
    Box<i32> box;
    Rc<i32> rc;
    Arc<i32> arc;
    Opt<i32> optional;
    Storage<i32> storage;
    String<> string;
    String_View string_view;
    Array<i32, 1> array;
    Vec<i32> vec;
    Slice<i32> slice;
    Stack<i32> stack;
    Queue<i32> queue;
    Heap<i32> heap;
    Map<i32, i32> map;
    Pair<i32, i32> pair;
    Tuple<i32, i32, i32> tuple;
    Variant<i32, f32> variant{0};
    Function<i32()> function{[]() { return 0; }};
}
```

### Allocators

```cpp
#include <rpp/base.h>

using namespace rpp;

i32 main() {
    using A = Mallocator<"A">;
    using B = Mallocator<"B">;
    {
        Vec<i32, A> a;
        Vec<i32, B> b;
        info("A allocated: %", a);
        info("B allocated: %", b);

        Box<i32, Mpool> pool;
        info("Pool allocated: %", pool);

        Region(R) {
            Vec<i32, Mregion<R>> region{1, 2, 3};
            info("Region allocated: %", region);
        }
    }
    Profile::finalize(); // Print statistics and check for leaks
}
```

### Trace

```cpp
#include <rpp/base.h>

using namespace rpp;

i32 main() {
    Profile::begin_frame();
    Trace("Section") {
        // ...
    }
    Profile::end_frame();

    Profile::iterate_timings([](Thread::Id id, const Profile::Timing_Node& n) {
        // ...
    });
}
```

### Reflection

```cpp
#include <rpp/base.h>

using namespace rpp;

struct Foo {
    i32 x;
    Vec<i32> y;
};
RPP_RECORD(Foo, RPP_FIELD(x), RPP_FIELD(y));

template<Reflectable T>
struct Bar {
    T t;
};
template<Reflectable T>
RPP_TEMPLATE_RECORD(Bar, T, RPP_FIELD(t));

i32 main() {
    Bar<Foo> bar{Foo{42, Vec<i32>{1, 2, 3}}};
    info("bar: %", bar);
}
```

### Thread

```cpp
#include <rpp/base.h>
#include <rpp/thread.h>

using namespace rpp;

i32 main() {
    Thread::set_priority(Thread::Priority::high);

    auto future = Thread::spawn([]() {
        info("Hello from thread %!", Thread::this_id());
        return 0;
    });

    info("Thread returned: %", future->block());
}
```

### Async

```cpp
#include <rpp/base.h>
#include <rpp/pool.h>
#include <rpp/asyncio.h>

using namespace rpp;

i32 main() {
    Async::Pool<> pool;

    auto coro = [](Async::Pool<>& pool) -> Async::Task<i32> {
        co_await pool.suspend();
        info("Hello from thread %!", Thread::this_id());
        co_await Async::wait(pool, 100);
        co_return 0;
    };

    auto task = coro(pool);
    info("Task returned: %", task.block());
}
```

### Math

```cpp
#include <rpp/base.h>
#include <rpp/vmath.h>
#include <rpp/simd.h>

using namespace rpp;

i32 main() {
    Vec3 v{0.0f, 1.0f, 0.0f};
    Mat4 m = Mat4::translate(v);
    info("Translated: %", m * v);

    auto simd = SIMD::F32x4::set1(1.0f);
    info("Dot product: %", SIMD::F32x4::dp(simd, simd));
}
```

## Build and Run Tests

To build rpp and run the tests, run the following commands:

### Windows

Assure MSVC 19.39 and cmake 3.17 (or newer) are installed and in your PATH.

```bash
mkdir build
cd build
cmake ..
cmake --build . -DRPP_TEST=ON
ctest -C Debug
```

For faster parallel builds, you can instead generate [ninja](https://ninja-build.org/) build files with `cmake -G Ninja ..`.

### Linux

Assure clang-17 and cmake 3.17 (or newer) are installed.

```bash
mkdir build
cd build
CXX=clang++-17 cmake .. -DRPP_TEST=ON
make -j
ctest -C Debug
```

For faster parallel builds, you can instead generate [ninja](https://ninja-build.org/) build files with `cmake -G Ninja ..`.

## To-Dos

- Modules
- Async
    - [ ] scheduler priorities
    - [ ] scheduler affinity
    - [ ] scheduler work stealing
    - [ ] io_uring for Linux file IO
    - [ ] sockets
    - [ ] use relaxed atomics on aarch64
- Types
    - [ ] Result<T,E>
    - [ ] Map: don't store hashes of integer keys
    - [ ] Opt: specializations for null representations
- Allocators
    - [ ] Allow reallocating the topmost stack allocation
    - [ ] Per-thread pools
- Misc
    - [ ] Range_Allocator: add second level of linear buckets
    - [ ] Range_Allocator: reduce overhead
