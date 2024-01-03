
#pragma once

#include "base.h"
#include "function.h"

namespace rpp {
namespace Log {

using Callback = void(Level, Thread::Id, Time, Location, String_View);
using Token = u64;

[[nodiscard]] Token subscribe(Function<Callback> f) noexcept;
void unsubscribe(Token token) noexcept;

template<Invocable<Level, Thread::Id, Time, Location, String_View> F>
[[nodiscard]] Token subscribe(F&& f) noexcept {
    return subscribe(Function<Callback>{forward<F>(f)});
}

} // namespace Log
} // namespace rpp
