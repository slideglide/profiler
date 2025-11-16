#pragma once
#include <chrono>
#include <cstdint>

namespace fxprof {
    struct Timestamp {
        uint64_t nanos = 0;

        static Timestamp now() {
            auto now = std::chrono::steady_clock::now();
            auto duration = now.time_since_epoch();
            return fromChrono(std::chrono::duration_cast<std::chrono::nanoseconds>(duration));
        }

        static Timestamp fromNanos(uint64_t nanos) {
            return { nanos };
        }

        static Timestamp fromChrono(std::chrono::nanoseconds duration) {
            return { static_cast<uint64_t>(duration.count()) };
        }

        static Timestamp fromChrono(std::chrono::milliseconds duration) {
            return { static_cast<uint64_t>(duration.count()) * 1'000'000 };
        }

        static Timestamp fromMillis(double millis) {
            return { static_cast<uint64_t>(millis * 1'000'000.0) };
        }

        bool operator<(Timestamp const& other) const {
            return nanos < other.nanos;
        }

        bool operator<=(Timestamp const& other) const {
            return nanos <= other.nanos;
        }

        bool operator==(Timestamp const& other) const {
            return nanos == other.nanos;
        }
    };
}

template <>
struct matjson::Serialize<fxprof::Timestamp> {
    static Value toJson(fxprof::Timestamp const& value) {
        return static_cast<double>(value.nanos) / 1'000'000.0;
    }
};