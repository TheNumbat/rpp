
#pragma once

namespace rpp {

template<Allocator A, typename... Ts>
String<A> format(String_View fmt, Ts&&... args) {
    // TODO
    return String<A>{};
}

template<typename... Ts>
u64 format_size(String_View fmt, Ts&&... args) {
    // TODO
    return 0;
}

} // namespace rpp
