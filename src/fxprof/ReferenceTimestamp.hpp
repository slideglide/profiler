#pragma once
#include <chrono>

namespace fxprof {
    struct ReferenceTimestamp {
        double msSinceEpoch = 0.0;

        static ReferenceTimestamp now() {
            auto now = std::chrono::system_clock::now();
            auto duration = now.time_since_epoch();
            auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
            return { static_cast<double>(millis) };
        }
    };

    struct PlatformSpecificReferenceTimestamp {
        uint64_t value = 0;
        enum class Type : uint8_t {
            ClockMonotonicNanosecondsSinceBoot,
            MachAbsoluteTimeNanoseconds,
            QueryPerformanceCounterValue
        } type;
    };
}