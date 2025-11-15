#pragma once
#include <atomic>
#include <thread>
#include <vector>
#include <unordered_map>
#include <matjson/reflect.hpp>

#include "platform/IPlatform.hpp"
#include "FirefoxProfile.hpp"

class Profiler {
private:
    Profiler() {
        m_samples.reserve(1000 * 60);
    }

    ~Profiler() { this->stop(); }

public:
    static Profiler& get() {
        static Profiler instance;
        return instance;
    }

    Profiler(Profiler const&) = delete;
    Profiler& operator=(Profiler const&) = delete;
    Profiler(Profiler&&) = delete;
    Profiler& operator=(Profiler&&) = delete;

    void start() {
        if (m_isRunning) {
            return;
        }
        m_running.test_and_set();
        m_samples.clear();
        m_thread = std::thread([this]() {
            m_startTime = std::chrono::steady_clock::now();
            platform::captureThread(m_running, m_samples);
            m_endTime = std::chrono::steady_clock::now();
        });
        m_isRunning = true;
    }

    void stop() {
        m_running.clear();
        if (m_thread.joinable()) {
            m_thread.join();
        }
        m_isRunning = false;
        this->save();
    }

    void save() {
        Profile profile;
        profile.meta.interval = 10; // Sample interval in milliseconds
        profile.meta.startTime = std::chrono::duration_cast<std::chrono::milliseconds>(
            m_startTime.time_since_epoch()
        ).count();
        profile.meta.endTime = std::chrono::duration_cast<std::chrono::milliseconds>(
            m_endTime.time_since_epoch()
        ).count();

        // export to json
        (void) geode::utils::file::writeToJson(
            "profiler.json",
            profile
        );
    }

    [[nodiscard]] bool isRunning() const {
        return m_isRunning;
    }

private:
    std::thread m_thread;
    std::atomic_flag m_running = ATOMIC_FLAG_INIT;
    std::chrono::steady_clock::time_point m_startTime{}, m_endTime{};
    std::vector<platform::StackSample> m_samples;
    bool m_isRunning = false;
};