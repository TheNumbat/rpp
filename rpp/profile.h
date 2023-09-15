
#pragma once

namespace rpp {

// NOTE(max): if DO_PROFILE is false, enter, exit, and alloc will no-op,
// but begin_frame and end_frame will still keep track of frame timing, and
// data structures will be allocated.

#ifdef RELEASE_BUILD
constexpr bool DO_PROFILE = false;
#else
constexpr bool DO_PROFILE = true;
#endif

#define Prof_Func Profile::Scope prof_scope__##__LINE__(Here)
#define Prof_Scope(name) Profile::Scope prof_scope__##__LINE__(name##_v)

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
    static void track_alloc_stats();

    static Time_Point timestamp();
    static f32 ms(Time_Point duration);
    static f32 s(Time_Point duration);

    struct Scope {
        Scope(Log::Location loc) {
            Profile::enter(std::move(loc));
        }
        Scope(String_View name) {
            Profile::enter(std::move(name));
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
            ret.loc = std::move(loc);
            ret.parent = parent;
            ret.begin = timestamp();
            return ret;
        }
    };

    template<typename F>
    static void iterate_timings(F&& f) {
        Thread::Lock lock(threads_lock);

        for(auto& entry : threads) {

            Thread::Id id = entry.first;
            Thread_Profile& tp = entry.second;

            Thread::Lock flock(tp->frames_lock);

            Frame_Profile& fp = null;
            if(tp->during_frame && tp->frames.size() > 1u) {
                fp = tp->frames.penultimate();
            } else if(!tp->during_frame && !tp->frames.empty()) {
                fp = tp->frames.back();
            } else {
                continue;
            }

            for(auto& node : fp.nodes) {
                f(node);
            }
        }
    }

    static void print_alloc_stats();

private:
    struct Alloc_Profile {
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
        bool during_frame = false;
        Thread::Mutex frames_lock;
        Queue<Frame_Profile, Mhidden> frames;
    };

    static void shutdown_stats();

    static inline Thread::Mutex threads_lock;
    static inline Thread::Mutex allocs_lock;
    static inline thread_local Thread_Profile this_thread;
    static inline Map<Thread::Id, Thread_Profile*, Mhidden> threads;
    static inline Map<String_View, Alloc_Profile, Mhidden> allocs;
};

template<>
struct Reflect<Profile::Alloc> {
    using T = Profile::Alloc;
    static constexpr Literal name = "Alloc";
    static constexpr Kind kind = Kind::record_;
    using members = List<FIELD(name), FIELD(address), FIELD(size)>;
    static_assert(Record<T>);
};

} // namespace rpp
