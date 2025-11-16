#pragma once
#include <cstdint>

namespace fxprof {
    using NativeSymbolIndex = uint32_t;
    using ThreadHandle = size_t;

    struct NativeSymbolHandle {
        ThreadHandle thread;
        NativeSymbolIndex index;
    };
}