#pragma once
#include <atomic>
#include <thread>
#include <vector>
#include <unordered_set>

#include <fxprof/Profile.hpp>
#include "platform/IPlatform.hpp"

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
            m_startTime = std::chrono::system_clock::now();
            platform::captureThread(m_running, m_samples);
            m_endTime = std::chrono::system_clock::now();
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
        using namespace fxprof;

        Profile profile(
            "Geometry Dash",
            ReferenceTimestamp::fromChrono(m_startTime),
            SamplingInterval::fromMillis(1)
        );

        profile.setOSName(GEODE_PLATFORM_NAME);

        auto process = profile.addProcess(
            "GeometryDash.exe",
            GetCurrentProcessId(),
            Timestamp::fromChrono(std::chrono::duration_cast<std::chrono::nanoseconds>(
                m_startTime.time_since_epoch()
            ))
        );

        auto thread = profile.addThread(
            process,
            GetCurrentThreadId(),
            Timestamp::fromChrono(std::chrono::duration_cast<std::chrono::nanoseconds>(
                m_startTime.time_since_epoch()
            )),
            true
        );

        profile.setThreadName(thread, geode::utils::thread::getName());

        auto category = profile.handleForCategory(
            Category{ "Other", CategoryColor::Grey }
        );

        // Collect all unique addresses from samples
        std::unordered_set<uintptr_t> uniqueAddresses;
        for (auto const& sample : m_samples) {
            for (size_t i = 0; i < sample.frameCount; ++i) {
                uniqueAddresses.insert(sample.frames[i]);
            }
        }
        std::vector<uintptr_t> addressList(uniqueAddresses.begin(), uniqueAddresses.end());

        // enumerate all loaded modules
        platform::enumerateModules([&](platform::ModuleInfo const& modInfo) {
            auto libHandle = profile.addLibrary(LibraryInfo{
                .name = modInfo.name,
                .path = modInfo.path,
                .debugName = modInfo.debugName,
                .debugPath = modInfo.debugPath,
                .breakpadId = modInfo.breakpadId,
                .codeId = modInfo.codeId,
                .arch = modInfo.arch
            });

            // Load symbols only for addresses we actually captured in this module
            auto symbols = platform::getModuleSymbols(modInfo.baseAddress, addressList);
            if (!symbols.empty()) {
                std::vector<Symbol> fxprofSymbols;
                fxprofSymbols.reserve(symbols.size());
                for (auto const& sym : symbols) {
                    fxprofSymbols.push_back(Symbol{
                        .name = sym.name,
                        .size = sym.size,
                        .address = sym.address
                    });
                }
                profile.setLibSymbolTable(libHandle, SymbolTable(std::move(fxprofSymbols)));
            }

            profile.addLibMapping(
                process,
                libHandle,
                modInfo.baseAddress,
                modInfo.baseAddress + modInfo.size,
                modInfo.relativeAddressAtStart
            );
        });

        // Build samples
        for (auto const& sample : m_samples) {
            auto stack = profile.handleForStackFrames(
                thread,
                [&](Profile& p, size_t i) -> std::optional<FrameHandle> {
                    if (i >= sample.frameCount) return std::nullopt;
                    size_t frameIndex = sample.frameCount - 1 - i;
                    uintptr_t addr = sample.frames[frameIndex];
                    FrameAddress frameAddr;
                    if (frameIndex == 0) {
                        frameAddr = FrameAddress::InstructionPointer{ addr };
                    } else {
                        frameAddr = FrameAddress::ReturnAddress{ addr };
                    }
                    return p.handleForFrameWithAddress(
                        thread,
                        frameAddr,
                        category,
                        FrameFlags::None
                    );
                }
            );

            profile.addSample(
                thread,
                Timestamp::fromMillis(1),
                stack,
                ZERO_DELTA,
                1
            );
        }

        // export to json
        (void) geode::utils::file::writeToJson("profiler.json", profile);
    }

    [[nodiscard]] bool isRunning() const {
        return m_isRunning;
    }

private:
    std::thread m_thread;
    std::atomic_flag m_running = ATOMIC_FLAG_INIT;
    std::chrono::system_clock::time_point m_startTime{}, m_endTime{};
    std::vector<platform::StackSample> m_samples;
    bool m_isRunning = false;
};