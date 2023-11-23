
#pragma once

#include "base.h"
#include "function.h"

namespace rpp {
namespace Log {

using Callback = void(Level, Thread::Id, Time, Location, String_View);
using Token = u64;

Token subscribe(Function<Callback> f);
void unsubscribe(Token token);

template<Invocable<Level, Thread::Id, Time, Location, String_View> F>
Token subscribe(F&& f) {
    return subscribe(Function<Callback>{std::forward<F>(f)});
}

} // namespace Log
} // namespace rpp
