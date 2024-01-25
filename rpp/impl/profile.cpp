
#include "../base.h"

namespace rpp {

[[nodiscard]] Profile::Time_Point Profile::timestamp() noexcept {
    return Thread::perf_counter();
}

[[nodiscard]] f32 Profile::ms(Profile::Time_Point duration) noexcept {
    return static_cast<f32>(duration / (Thread::perf_frequency() / 1000.0));
}

[[nodiscard]] f32 Profile::s(Profile::Time_Point duration) noexcept {
    return static_cast<f32>(duration / static_cast<f64>(Thread::perf_frequency()));
}

void Profile::register_thread() noexcept {
    Thread::Lock lock(threads_lock);

    Thread::Id id = Thread::this_id();
    assert(!threads.contains(id));

    this_thread.registered = true;
    this_thread.during_frame = false;
    threads.insert(id, Ref{this_thread});
}

void Profile::unregister_thread() noexcept {
    Thread::Lock lock(threads_lock);

    Thread::Id id = Thread::this_id();
    static_cast<void>(threads.try_erase(id));

    this_thread.registered = false;
}

[[nodiscard]] Profile::Time_Point Profile::Frame_Profile::begin() noexcept {
    assert(current_node == 0 && nodes.empty());
    Timing_Node& node = nodes.push(Timing_Node::make(Log::Location{"Frame"_v, {}, 0}, 0));
    return node.begin;
}

void Profile::Frame_Profile::end() noexcept {
    assert(current_node == 0);
    Timing_Node& root = nodes.front();
    root.end = timestamp();
    root.heir_time = root.end - root.begin;
    compute_self_times(0);
}

void Profile::Frame_Profile::compute_self_times(u64 idx) noexcept {
    Timing_Node& node = nodes[idx];
    u64 child_time = 0;
    for(u64 child : node.children) {
        compute_self_times(child);
        child_time = child_time + nodes[child].heir_time;
    }
    node.self_time = node.heir_time - child_time;
}

f32 Profile::begin_frame() noexcept {

    assert(!this_thread.ready());

    Thread_Profile& prof = this_thread;
    Thread::Lock lock(prof.frames_lock);

    if(!prof.frames.empty() && prof.frames.full()) {
        prof.frames.pop();
    }

    Frame_Profile& new_frame = prof.frames.emplace();

    Time_Point t = new_frame.begin();

    f32 ret = 0.0f;
    if(prof.frames.length() > 1) {
        Frame_Profile& prev_frame = prof.frames.penultimate();
        assert(!prev_frame.nodes.empty());
        ret = ms(t - prev_frame.nodes[0].begin) / 1000.0f;
    }

    prof.during_frame = true;
    return ret;
}

void Profile::end_frame() noexcept {

    assert(this_thread.registered && this_thread.during_frame);

    Thread_Profile& prof = this_thread;
    Thread::Lock lock(prof.frames_lock);

    assert(!prof.frames.empty());

    Frame_Profile& this_frame = prof.frames.back();
    this_frame.end();

    prof.during_frame = false;
}

void Profile::enter(String_View name) noexcept {
    if constexpr(DO_PROFILE) {
        if(!this_thread.ready()) return;
        enter(Log::Location{move(name), ""_v, 0});
    }
}

void Profile::enter(Log::Location loc) noexcept {
    if constexpr(DO_PROFILE) {
        if(!this_thread.ready()) return;
        Thread::Lock lock(this_thread.frames_lock);
        this_thread.frames.back().enter(move(loc));
    }
}

void Profile::Frame_Profile::enter(Log::Location loc) noexcept {

    bool repeat = false;
    Timing_Node& node = nodes[current_node];

    for(u64 child_idx : node.children) {
        Timing_Node& child = nodes[child_idx];
        if(child.loc == loc) {
            current_node = child_idx;
            repeat = true;
        }
    }

    if(!repeat) {
        u64 child_idx = nodes.length();
        node.children.push(child_idx);
        nodes.push(Timing_Node::make(move(loc), current_node));
        current_node = child_idx;
    } else {
        node.begin = timestamp();
        node.calls++;
    }
}

void Profile::exit() noexcept {
    if constexpr(DO_PROFILE) {
        if(!this_thread.ready()) return;
        Thread::Lock lock(this_thread.frames_lock);
        this_thread.frames.back().exit();
    }
}

void Profile::Frame_Profile::exit() noexcept {

    Frame_Profile& frame = this_thread.frames.back();
    Timing_Node& node = frame.nodes[frame.current_node];

    node.end = timestamp();
    node.heir_time += node.end - node.begin;
    frame.current_node = node.parent;
}

void Profile::alloc(Alloc a) noexcept {
    if constexpr(DO_PROFILE) {
        {
            Thread::Lock lock(allocs_lock);
            Alloc_Profile& prof = allocs.get_or_insert(a.name);

            if(a.size) {

                if(prof.current_set.contains(a.address)) {
                    warn("Profile: % reallocated %!", a.name, a.address);
                }
                prof.current_set.insert(a.address, a.size);

                prof.allocate_size += a.size;
                prof.allocates++;
                prof.current_set_size += a.size;
                prof.high_water = Math::max(prof.high_water, prof.current_set_size);

            } else {

                Opt<Ref<u64>> sz = prof.current_set.try_get(a.address);
                if(!sz.ok()) {
                    warn("Profile: % freed % with no entry!", a.name, a.address);
                } else {
                    i64 size = **sz;
                    prof.current_set.erase(a.address);
                    prof.free_size += size;
                    prof.frees++;
                    prof.current_set_size -= size;
                }
            }
        }
        {
            Thread::Lock lock(this_thread.frames_lock);
            if(this_thread.during_frame) {
                this_thread.frames.back().allocations.push(move(a));
            }
        }
    }
}

void Profile::finalizer(Function<void()> f) noexcept {
    Thread::Lock lock(finalizers_lock);
    finalizers.push(move(f));
}

void Profile::finalize() noexcept {
    // All threads must have exited before we can finalize.
    {
        Thread::Lock lock(finalizers_lock);
        for(auto& f : finalizers) {
            f();
        }
        finalizers.~Vec();
    }
    this_thread.finalize();
    {
        Thread::Lock lock(allocs_lock);
        for(auto& prof : allocs) {
            info("Allocation stats for [%]:", prof.first);
            info("\tAllocs: %", prof.second.allocates);
            info("\tFrees: %", prof.second.frees);
            info("\tHigh water: %", prof.second.high_water);
            info("\tAlloc size: %", prof.second.allocate_size);
            info("\tFree size: %", prof.second.free_size);
            if(prof.second.current_set_size != 0) {
                warn("\tUnbalanced size: %", prof.second.current_set_size);
            }
        }
        allocs.~Map();
    }
    {
        Thread::Lock lock(threads_lock);
        threads.~Map();
    }
    i64 net = sys_net_allocs();
    if(net != 0) {
        warn("Unbalanced allocations: %", net);
    } else {
        info("No memory leaked.");
    }
    if(Region_Allocator::size() != 0) {
        warn("Unbalanced region size: %", Region_Allocator::size());
    } else {
        info("No region memory leaked.");
    }
    if(Region_Allocator::depth() != 0) {
        warn("Unbalanced regions: %", Region_Allocator::depth());
    } else {
        info("No regions leaked.");
    }
    if(net != 0) {
        warn("Memory leaked, shutting down now...");
        Libc::exit(1);
    }
}

} // namespace rpp
