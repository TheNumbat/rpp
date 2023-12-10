
#include "../base.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

namespace rpp::Libc {

void exit(i32 code) {
    ::exit(code);
}

i32 strncmp(const char* a, const char* b, u64 bytes) {
    return ::strncmp(a, b, bytes);
}

u64 strlen(const char* str) {
    return ::strlen(str);
}

i32 memcmp(const void* a, const void* b, u64 bytes) {
    return ::memcmp(a, b, bytes);
}

void* memset(void* dest, i32 value, u64 bytes) {
    return ::memset(dest, value, bytes);
}

void* memcpy(void* dest, const void* src, u64 bytes) {
    return ::memcpy(dest, src, bytes);
}

i32 snprintf(u8* buffer, u64 buffer_size, const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    i32 written = ::vsnprintf(reinterpret_cast<char*>(buffer), buffer_size, fmt, args);
    va_end(args);
    return written;
}

i64 strtoll(const char* str, char** endptr, i32 base) {
    return ::strtoll(str, endptr, base);
}

f32 strtof(const char* str, char** endptr) {
    return ::strtof(str, endptr);
}

} // namespace rpp::Libc
