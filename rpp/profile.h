
#pragma once

#ifndef RPP_BASE
#error "Include base.h instead."
#endif

namespace rpp {

// NOTE(max): if DO_PROFILE is false, enter, exit, and alloc will no-op,
// but begin_frame and end_frame will still keep track of frame timing, and
// data structures will be allocated.

#ifdef RPP_RELEASE_BUILD
constexpr bool DO_PROFILE = false;
#else
constexpr bool DO_PROFILE = true;
#endif

#define RPP_TRACE2(COUNTER) __profile_##COUNTER
#define RPP_TRACE1(NAME, COUNTER) if(::rpp::Profile::Scope RPP_TRACE2(COUNTER){String_View{NAME}})

#define Trace(NAME) RPP_TRACE1(NAME, __COUNTER__)

struct Profile {

    static float begin_frame() noexcept;
    static void end_frame() noexcept;

    using Time_Point = u64;

    static void enter(Log::Location l) noexcept;
    static void enter(String_View l) noexcept;
    static void exit() noexcept;

    struct Alloc {
        String_View name;
        void* address = null;
        u64 size = 0; // 0 means free
    };
    static void alloc(Alloc a) noexcept;

    [[nodiscard]] static Time_Point timestamp() noexcept;
    [[nodiscard]] static f32 ms(Time_Point duration) noexcept;
    [[nodiscard]] static f32 s(Time_Point duration) noexcept;

    struct Scope {
        Scope(Log::Location loc) noexcept {
            Profile::enter(rpp::move(loc));
        }
        Scope(String_View name) noexcept {
            Profile::enter(rpp::move(name));
        }
        ~Scope() noexcept {
            Profile::exit();
        }

        [[nodiscard]] consteval operator bool() noexcept {
            return true;
        }
    };

    struct Timing_Node {
        Log::Location loc;
        Time_Point begin = 0, end = 0;
        Time_Point self_time = 0, heir_time = 0;
        u64 calls = 0;
        u64 parent = 0;
        Vec<u64, Mhidden> children;

        [[nodiscard]] static Timing_Node make(Log::Location loc, u64 parent) noexcept {
            Timing_Node ret;
            ret.loc = rpp::move(loc);
            ret.parent = parent;
            ret.begin = timestamp();
            ret.calls = 1;
            return ret;
        }
    };

    template<typename F>
    static void iterate_timings(F&& f) noexcept {
        Thread::Lock lock(threads_lock);

        for(auto& entry : threads) {

            Thread::Id id = entry.first;
            Thread_Profile& thread = *entry.second;
            Thread::Lock frames_lock(thread.frames_lock);

            Frame_Profile* frame = null;
            if(thread.during_frame && thread.frames.length() > 1u) {
                frame = &thread.frames.penultimate();
            } else if(!thread.during_frame && !thread.frames.empty()) {
                frame = &thread.frames.back();
            } else {
                continue;
            }

            for(auto& node : frame->nodes) {
                f(id, node);
            }
        }
    }

    static void finalizer(Function<void()> f) noexcept;
    static void finalize() noexcept;

private:
    static void register_thread() noexcept;
    static void unregister_thread() noexcept;

    struct Alloc_Profile {
        Alloc_Profile() noexcept {
        }

        i64 allocates = 0, frees = 0;
        i64 allocate_size = 0, free_size = 0;
        i64 high_water = 0;
        i64 current_set_size = 0;
        Map<void*, u64, Mhidden> current_set;
    };

    struct Frame_Profile {
        [[nodiscard]] Time_Point begin() noexcept;
        void end() noexcept;
        void enter(Log::Location loc) noexcept;
        void exit() noexcept;
        void compute_self_times(u64 idx) noexcept;

        u64 current_node = 0;
        Vec<Timing_Node, Mhidden> nodes;
        Vec<Alloc, Mhidden> allocations;
    };

    struct Thread_Profile {
        Thread_Profile() noexcept : during_frame(false) {
        }
        ~Thread_Profile() noexcept {
            finalize();
        }

        void finalize() noexcept {
            frames = {};
            if(during_frame) Profile::end_frame();
            if(registered) Profile::unregister_thread();
        }

        bool ready() noexcept {
            if(!registered) Profile::register_thread();
            return during_frame;
        }

        bool registered = false;
        bool during_frame = false;
        Thread::Mutex frames_lock;
        Queue<Frame_Profile, Mhidden> frames;
    };

    static inline Thread::Mutex threads_lock;
    static inline Thread::Mutex allocs_lock;
    static inline Thread::Mutex finalizers_lock;
    static inline thread_local Thread_Profile this_thread;
    static inline Map<Thread::Id, Ref<Thread_Profile>, Mhidden> threads;
    static inline Map<String_View, Alloc_Profile, Mhidden> allocs;
    static inline Vec<Function<void()>, Mhidden> finalizers;
};

RPP_RECORD(Profile::Alloc, RPP_FIELD(name), RPP_FIELD(address), RPP_FIELD(size));

} // namespace rpp
