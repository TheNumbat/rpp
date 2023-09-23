
#pragma once

namespace rpp {

template<Literal N, bool log>
void* Mallocator<N, log>::alloc(u64 size) {
    if(!size) return null;
    void* ret = sys_alloc(size);
    if constexpr(log) {
        Profile::alloc({String_View{N}, ret, size});
    }
    return ret;
}

template<Literal N, bool log>
void Mallocator<N, log>::free(void* mem) {
    if(!mem) return;
    if constexpr(log) {
        Profile::alloc({String_View{N}, mem, 0});
    }
    sys_free(mem);
}

inline void* Mregion::alloc(u64 size) {
    assert(current_offset + size < REGION_STACK_SIZE);
    void* ret = stack_memory + current_offset;
    current_offset += size;
    std::memset(ret, 0, size);
    return ret;
}

inline void Mregion::free(void* mem) {
}

inline void Mregion::begin() {
    assert(current_region + 1 < REGION_COUNT);
    current_region++;
    region_offsets[current_region] = current_offset;
}

inline void Mregion::end() {
    assert(current_region > 0);
    current_offset = region_offsets[current_region];
    current_region--;
}

inline Mregion::Scope::Scope() {
    Mregion::begin();
}

inline Mregion::Scope::~Scope() {
    Mregion::end();
}

inline u64 Mregion::depth() {
    return current_region;
}

inline u64 Mregion::size() {
    return current_offset;
}

} // namespace rpp
