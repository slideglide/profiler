#pragma once
#include <cstdint>

namespace fxprof {
    struct CpuDelta {
        uint64_t micros = 0;
        constexpr CpuDelta() {}
        constexpr explicit CpuDelta(uint64_t microseconds) : micros(microseconds) {}

        constexpr bool operator==(CpuDelta const& other) const {
            return micros == other.micros;
        }
    };

    constexpr CpuDelta ZERO_DELTA{0};
}

template <>
struct matjson::Serialize<fxprof::CpuDelta> {
    static Value toJson(fxprof::CpuDelta const& value) {
        return value.micros;
    }
};