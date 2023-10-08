
#include "base.h"

#include <cstdarg>
#include <cstdio>
#include <cstring>

namespace rpp::Std {

i32 strncmp(const char* a, const char* b, u64 bytes) {
    return std::strncmp(a, b, bytes);
}

u64 strlen(const char* str) {
    return std::strlen(str);
}

void* memset(void* dest, i32 value, u64 bytes) {
    return std::memset(dest, value, bytes);
}

void* memcpy(void* dest, const void* src, u64 bytes) {
    return std::memcpy(dest, src, bytes);
}

i32 snprintf(u8* buffer, u64 buffer_size, const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    i32 written = std::vsnprintf(reinterpret_cast<char* const>(buffer), buffer_size, fmt, args);
    va_end(args);
    return written;
}

} // namespace rpp::Std
