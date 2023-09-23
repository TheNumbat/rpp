# rpp

Minimal Rust-inspired C++20 standard library replacement.

- Optimizes for fast compile times.
- Only includes code I'm actively using.

Headers
- base.h: always include
- files.h: file IO
- net.h: network IO
- thread.h: threading and coroutine pool
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

To-Do:
- Finish coroutine scheduler
- Tuple replacement
- Variant replacement 
