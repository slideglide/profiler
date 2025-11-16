#pragma once
#include <optional>
#include <string>

#include "FrameTable.hpp"
#include "MarkerTable.hpp"
#include "SampleTable.hpp"
#include "StackTable.hpp"
#include "Timestamp.hpp"

namespace fxprof {
    using ProcessHandle = size_t;

    struct FrameHandle {
        ThreadHandle thread;
        size_t index;
    };

    class Thread {
    public:
        Thread(
            ProcessHandle process,
            std::string tid,
            Timestamp startTime,
            bool isMain = false
        ) : m_process(process),
            m_tid(std::move(tid)),
            m_startTime(startTime),
            m_isMain(isMain) {}

        ProcessHandle getProcess() const {
            return m_process;
        }

        std::string const& getTid() const {
            return m_tid;
        }

        std::optional<std::string> const& getName() const {
            return m_name;
        }

        Timestamp getStartTime() const {
            return m_startTime;
        }

        std::optional<Timestamp> const& getEndTime() const {
            return m_endTime;
        }

        StackTable const& getStackTable() const {
            return m_stackTable;
        }

        FrameInterner const& getFrameInterner() const {
            return m_frameInterner;
        }

        SampleTable const& getSamples() const {
            return m_samples;
        }

        MarkerTable const& getMarkers() const {
            return m_markers;
        }

        bool isMain() const {
            return m_isMain;
        }

        bool isShowMarkersInTimeline() const {
            return m_showMarkersInTimeline;
        }

        void setName(std::string name) {
            m_name = std::move(name);
        }

        void addSample(
            Timestamp timestamp,
            std::optional<size_t> stackIndex,
            CpuDelta cpuDelta,
            int32_t weight
        ) {
            m_samples.addSample(timestamp, stackIndex, cpuDelta, weight);
            m_lastSampleStack = stackIndex;
            m_lastSampleWasZeroCpu = cpuDelta == ZERO_DELTA;
        }

        size_t stackIndexForStack(std::optional<size_t> prefix, size_t frame) {
            return m_stackTable.indexForStack(prefix, frame);
        }

        size_t frameIndexForFrame(InternalFrame frame) {
            return m_frameInterner.indexForFrame(std::move(frame));
        }

        std::pair<NativeSymbolIndex, StringHandle> nativeSymbolIndexAndStringIndexForSymbol(
            GlobalLibIndex libIndex,
            Symbol const& symbol,
            ProfileStringTable& stringTable
        ) {
            return m_nativeSymbols.symbolIndexAndStringIndexForSymbol(
                libIndex,
                symbol,
                stringTable
            );
        }

    private:
        ProcessHandle m_process = 0;
        std::string m_tid;
        std::optional<std::string> m_name;
        Timestamp m_startTime;
        std::optional<Timestamp> m_endTime;
        StackTable m_stackTable;
        FrameInterner m_frameInterner;
        SampleTable m_samples;
        // std::optional<NativeAllocationsTable> m_nativeAllocations;
        MarkerTable m_markers;
        NativeSymbols m_nativeSymbols;
        std::optional<size_t> m_lastSampleStack;
        bool m_lastSampleWasZeroCpu = false;
        bool m_showMarkersInTimeline = false;
        bool m_isMain = false;
    };
}

template <>
struct matjson::Serialize<fxprof::Thread> {
    static Value toJson(fxprof::Thread const& value) {
        auto [frameTable, funcTable, resourceTable] = value.getFrameInterner().createTables();
        auto threadName = value.getName().has_value() ? *value.getName() : fmt::format("Thread <{}>", value.getTid());

        return makeObject({
            {"frameTable", frameTable},
            {"funcTable", funcTable},
            {"markers", value.getMarkers()},
            {"name", std::move(threadName)},
            {"isMainThread", value.isMain()},
            // {"nativeSymbols", value.getNativeSymbols()},
            {"pausedRanges", Value::array()},
            // {"pid", ""},
            // {"processName", ""},
            // {"processShutdownTime", nullptr},
            // {"processStartupTime", nullptr},
            {"registerTime", value.getStartTime()},
            {"resourceTable", resourceTable},
            {"samples", value.getSamples()},
            // {"nativeAllocations", value.getNativeAllocations()},
            {"stackTable", value.getStackTable()},
            {"tid", value.getTid()},
            {"unregisterTime", value.getEndTime()},
            {"showMarkersInTimeline", value.isShowMarkersInTimeline()},
        });
    }
};