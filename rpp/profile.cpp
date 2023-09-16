
#include "base.h"

namespace rpp {

Profile::Time_Point Profile::timestamp() {
    return Thread::perf_counter();
}

f32 Profile::ms(Profile::Time_Point duration) {
    return static_cast<f32>(duration / (Thread::perf_frequency() / 1000.0));
}

f32 Profile::s(Profile::Time_Point duration) {
    return static_cast<f32>(duration / static_cast<f64>(Thread::perf_frequency()));
}

void Profile::start_thread() {
    Thread::Lock lock(threads_lock);

    Thread::Id id = Thread::this_id();
    assert(!threads.contains(id));

    this_thread.during_frame = false;
    threads.insert(id, Ref{this_thread});
}

void Profile::end_thread() {
    Thread::Lock lock(threads_lock);

    Thread::Id id = Thread::this_id();
    threads.erase(id);

    this_thread.~Thread_Profile();
}

Profile::Time_Point Profile::Frame_Profile::begin() {
    assert(current_node == 0 && nodes.empty());
    Timing_Node& node = nodes.push(Timing_Node::make(Here, 0));
    return node.begin;
}

void Profile::Frame_Profile::end() {
    assert(current_node == 0);
    Timing_Node& root = nodes.front();
    root.end = timestamp();
    root.heir_time = root.end - root.begin;
    compute_self_times(0);
}

void Profile::Frame_Profile::compute_self_times(u64 idx) {
    Timing_Node& node = nodes[idx];
    u64 child_time = 0;
    for(u64 child : node.children) {
        compute_self_times(child);
        child_time = child_time + nodes[child].heir_time;
    }
    node.self_time = node.heir_time - child_time;
}

float Profile::begin_frame() {

    Thread_Profile& prof = this_thread;
    Thread::Lock lock(prof.frames_lock);

    if(!prof.frames.empty() && prof.frames.full()) {
        prof.frames.pop();
    }

    Frame_Profile& new_frame = prof.frames.emplace();

    Time_Point t = new_frame.begin();

    float ret = 0.0f;
    if(prof.frames.length() > 1) {
        Frame_Profile& prev_frame = prof.frames.penultimate();
        assert(!prev_frame.nodes.empty());
        ret = ms(t - prev_frame.nodes[0].begin) / 1000.0f;
    }

    prof.during_frame = true;
    return ret;
}

void Profile::end_frame() {

    Thread_Profile& prof = this_thread;
    Thread::Lock lock(prof.frames_lock);

    assert(!prof.frames.empty());

    Frame_Profile& this_frame = prof.frames.back();
    this_frame.end();

    prof.during_frame = false;
}

void Profile::enter(String_View name) {
    if constexpr(DO_PROFILE) {
        enter(Log::Location{std::move(name), ""_v, 0, 0});
    }
}

void Profile::enter(Log::Location loc) {
    if constexpr(DO_PROFILE) {
        Thread::Lock lock(this_thread.frames_lock);
        this_thread.frames.back().enter(std::move(loc));
    }
}

void Profile::Frame_Profile::enter(Log::Location loc) {

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
        nodes.push(Timing_Node::make(std::move(loc), current_node));
        current_node = child_idx;
    } else {
        node.begin = timestamp();
        node.calls++;
    }
}

void Profile::exit() {
    if constexpr(DO_PROFILE) {
        Thread::Lock lock(this_thread.frames_lock);
        this_thread.frames.back().exit();
    }
}

void Profile::Frame_Profile::exit() {

    Frame_Profile& frame = this_thread.frames.back();
    Timing_Node& node = frame.nodes[frame.current_node];

    node.end = timestamp();
    node.heir_time += node.end - node.begin;
    frame.current_node = node.parent;
}

void Profile::alloc(Alloc a) {
    if constexpr(DO_PROFILE) {
        {
            Thread::Lock lock(allocs_lock);
            Alloc_Profile& prof = allocs.get_or_insert(a.name.clone());

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
                if(!sz) {
                    warn("Profile: % freed % with no entry!", a.name, a.address);
                } else {
                    i64 size = *sz;
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
                this_thread.frames.back().allocations.push(std::move(a));
            }
        }
    }
}

void Profile::track_alloc_stats() {
#ifndef RELEASE_BUILD
    atexit(shutdown_stats);
#endif
}

void Profile::shutdown_stats() {
    print_alloc_stats();
    threads.~Map();
    allocs.~Map();
    i64 net = sys_net_allocs();
    if(net != 0) {
        warn("Unbalanced allocations: %", net);
    } else {
        info("No memory leaked.");
    }
}

void Profile::print_alloc_stats() {
    Thread::Lock alock(allocs_lock);
    Thread::Lock tlock(threads_lock);
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
}

} // namespace rpp
