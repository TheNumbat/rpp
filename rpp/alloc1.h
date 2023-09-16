
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

} // namespace rpp
