#pragma once

constexpr size_t MAX_STACK_FRAMES = 64;

namespace platform {
    void init();

    struct StackSample {
        uintptr_t frames[MAX_STACK_FRAMES];
        size_t frameCount;
    };

    void captureThread(
        std::atomic_flag& isRunning,
        std::vector<StackSample>& samples
    );
}