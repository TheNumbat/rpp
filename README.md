# rpp

Minimal Rust-inspired C++20 standard library (mostly) replacement.

- Optimizes for fast compile times.
- Only includes code I'm actively using.

Headers
- base.h: reflection, allocators, profiling, strings, basic containers, basic math
- files.h: sync file IO
- net.h: sync network IO
- thread.h: threading primitives
- async.h: thread-safe coroutine primitives
- asyncio.h: IO coroutines
- pool.h: coroutine scheduler thread pool
- range_allocator.h: generic general purpose allocator
- simd.h: basic SSE vectors
- vmath.h: 3D vector and matrix math
- rng.h: hash-based PRNG
- function.h, heap.h, rc.h, stack.h, tuple.h, variant.h: see containers

Supported configurations
- Windows / x86_64 / MSVC 19.38+
- Linux / x86_64 / Clang 17+

Pointers
- Raw pointer: non owning
- Ref: non owning
- Box: owning unique
- Rc: refcount
- Arc: atomic refcount

Containers
- String: pascal style utf8 string
- String_View: non owning const string reference
- Array: fixed size array
- Vec: resizable array
- Slice: non owning const range reference
- Stack
- Queue: ring buffer
- Heap: linear priority queue
- Map: robin hood hash map
- Opt: in-place optional
- Storage: manual in-place RAII wrapper
- Pair: two values with heterogeneous types
- Tuple: N values with heterogeneous types
- Variant: sum type (only unique types) with scuffed pattern matching
- Function: fixed-size type-erased closure

Utilities
- 3D vector and matrix math (SSE)
- Type based allocator tracking
- Thread local region based stack allocator
- Timing and allocation profile tracking
- Various concepts
- Only trivial types are copyable
- Nontrivial types provide a clone method
- Generic reflection system
- Generic sprintf
- Hashing and PRNG
- Logging macros
- Custom coroutine wrappers
- Thread pool that schedules coroutines

To-Do:
- Release
    - Refactor tests
    - Update readme
    - CI
    - Pool Allocator
- Result<T,E> type
- Thread pool
    - Priorities & CPU affinity
    - Work stealing
- IO
    - Proper async file IO on linux
- Map
    - Don't store hashes given integer keys
- Range allocator
    - Second level of linear buckets
    - Reduce memory overhead
