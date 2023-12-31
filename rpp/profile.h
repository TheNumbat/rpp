
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

#define PROF_SCOPE2(name, line) Profile::Scope prof_scope__##line(name##_v)
#define PROF_SCOPE1(name, line) PROF_SCOPE2(name, line)
#define Prof_Scope(name) PROF_SCOPE1(name, __COUNTER__)

struct Profile {

    static void start_thread();
    static void end_thread();

    static float begin_frame();
    static void end_frame();

    using Time_Point = u64;

    static void enter(Log::Location l);
    static void enter(String_View l);
    static void exit();

    struct Alloc {
        String_View name;
        void* address = null;
        u64 size = 0; // 0 means free
    };
    static void alloc(Alloc a);

    static Time_Point timestamp();
    static f32 ms(Time_Point duration);
    static f32 s(Time_Point duration);

    struct Scope {
        Scope(Log::Location loc) {
            Profile::enter(move(loc));
        }
        Scope(String_View name) {
            Profile::enter(move(name));
        }
        ~Scope() {
            Profile::exit();
        }
    };

    struct Timing_Node {
        Log::Location loc;
        Time_Point begin = 0, end = 0;
        Time_Point self_time = 0, heir_time = 0;
        u64 calls = 0;
        u64 parent = 0;
        Vec<u64, Mhidden> children;

        static Timing_Node make(Log::Location loc, u64 parent) {
            Timing_Node ret;
            ret.loc = move(loc);
            ret.parent = parent;
            ret.begin = timestamp();
            ret.calls = 1;
            return ret;
        }
    };

    template<typename F>
    static void iterate_timings(F&& f) {
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

    static void finalizer(Function<void()> f);
    static void finalize();

private:
    struct Alloc_Profile {
        Alloc_Profile() {
        }

        i64 allocates = 0, frees = 0;
        i64 allocate_size = 0, free_size = 0;
        i64 high_water = 0;
        i64 current_set_size = 0;
        Map<void*, u64, Mhidden> current_set;
    };

    struct Frame_Profile {
        Time_Point begin();
        void end();
        void enter(Log::Location loc);
        void exit();
        void compute_self_times(u64 idx);

        u64 current_node = 0;
        Vec<Timing_Node, Mhidden> nodes;
        Vec<Alloc, Mhidden> allocations;
    };

    struct Thread_Profile {
        Thread_Profile() : during_frame(false) {
        }

        bool during_frame;
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

namespace Reflect {

template<>
struct Refl<Profile::Alloc> {
    using T = Profile::Alloc;
    static constexpr Literal name = "Alloc";
    static constexpr Kind kind = Kind::record_;
    using members = List<FIELD(name), FIELD(address), FIELD(size)>;
};

} // namespace Reflect

} // namespace rpp
