# rpp

Minimal Rust-inspired C++20 standard library (mostly) replacement.

- Optimizes for fast compile times.
- Only includes code I'm actively using.

Headers
- base.h: always include
- files.h: file IO
- net.h: network IO
- thread.h: threading primitives
- async.h: thread-safe coroutine primitives
- pool.h: thread pool and coroutine scheduler
- range_allocator.h: generic general purpose allocator

Supported configurations
- Windows / x86_64 / MSVC
- Windows / x86_64 / Clang
- Linux / x86_64 / Clang

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
- Pair
- Storage: manual in-place RAII wrapper
- Opt: in-place optional

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
- Thread pool that can run blocking jobs and coroutines with two priority levels

To-Do:
- Tuple replacement
- Variant replacement
- Thread pool upgrades
    - Async IO events
    - Affinity per task
    - Lock-free queue
    - Thread local queues
    - Work stealing
- Range allocator upgrades
    - Second level of linear buckets
    - Reduce memory overhead
