#pragma once
#include <cstdint>
#include "Process.hpp"

namespace fxprof {
    using NativeSymbolIndex = uint32_t;

    struct NativeSymbolHandle {
        ThreadHandle thread;
        NativeSymbolIndex index;
    };
}