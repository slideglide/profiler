#pragma once
#include <cstddef>

#include "Thread.hpp"

namespace fxprof {
    using CounterHandle = size_t;

    class Counter {
    public:

    private:
        std::string name;
        std::string category;
        std::string description;
        ProcessHandle m_process = 0;
        std::string m_pid;
        // CounterSamples m_samples;
        // std::optional<GraphColor> m_color;
    };
}
