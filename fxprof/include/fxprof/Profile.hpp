#pragma once
#include "Category.hpp"
#include "Counter.hpp"
#include "GlobalLibTable.hpp"
#include "LibMappings.hpp"
#include "NativeSymbols.hpp"
#include "Process.hpp"
#include "ReferenceTimestamp.hpp"
#include "StackTable.hpp"
#include "StringTable.hpp"
#include "Thread.hpp"
#include "markers/Internal.hpp"
#include "markers/Types.hpp"

namespace fxprof {
    struct SamplingInterval {
        uint64_t nanos = 0;
        SamplingInterval() = default;
        SamplingInterval(uint64_t n) : nanos(n) {}

        static SamplingInterval fromNanos(uint64_t nanos) {
            return {nanos};
        }

        static SamplingInterval fromHz(float hz) {
            return {static_cast<uint64_t>(1'000'000'000.0f / hz)};
        }

        static SamplingInterval fromMillis(uint64_t millis) {
            return {millis * 1'000'000};
        }

        static SamplingInterval from(std::chrono::nanoseconds duration) {
            return {static_cast<uint64_t>(duration.count())};
        }

        double toSeconds() const {
            return static_cast<double>(nanos) / 1'000'000'000.0;
        }
    };

    struct SourceLocation {
        std::optional<StringHandle> filePath;
        std::optional<uint32_t> line;
        std::optional<uint32_t> column;
    };

    struct FrameSymbolInfo {
        std::optional<StringHandle> name;
        NativeSymbolHandle nativeSymbol;
        SourceLocation sourceLocation;
    };

    struct FrameHandle {
        ThreadHandle thread;
        size_t index;
    };

    struct StackHandle {
        ThreadHandle thread;
        size_t index;
    };

    enum class TimelineUnit {
        Milliseconds,
        Bytes
    };

    class Profile {
    public:
        Profile(
            std::string product,
            ReferenceTimestamp referenceTimestamp,
            SamplingInterval interval
        ) : m_product(std::move(product)), m_interval(interval), m_referenceTimestamp(referenceTimestamp) {
            m_categories.insert(InternalCategory("Other", CategoryColor::Gray));
        }

        void setInterval(SamplingInterval interval) { m_interval = interval; }
        void setReferenceTimestamp(ReferenceTimestamp timestamp) { m_referenceTimestamp = timestamp; }
        void setProduct(std::string product) { m_product = std::move(product); }
        void setOSName(std::string osName) { m_osName = std::move(osName); }
        void setTimelineUnit(TimelineUnit unit) { m_timelineUnit = unit; }

        void setPlatformSpecificReferenceTimestamp(PlatformSpecificReferenceTimestamp timestamp) {
            m_platformSpecificReferenceTimestamp = timestamp;
        }

        ProcessHandle addProcess(std::string name, uint32_t pid, Timestamp startTime) {
            auto uPid = this->makeUniquePid(pid);
            auto handle = m_processes.size();
            m_processes.emplace_back(std::move(name), std::move(uPid), startTime);
            return handle;
        }

        ThreadHandle addThread(ProcessHandle process, uint32_t tid, Timestamp startTime, bool isMain) {
            auto uTid = this->makeUniqueTid(tid);
            auto handle = m_threads.size();
            m_threads.emplace_back(process, std::move(uTid), startTime, isMain);
            m_processes[process].addThread(handle, isMain);
            return handle;
        }

        void addSample(
            ThreadHandle thread,
            Timestamp timestamp,
            std::optional<StackHandle> stack,
            CpuDelta cpuDelta,
            int32_t weight
        ) {
            // find stack index
            std::optional<size_t> stackIndex = stack.has_value() ? std::optional(stack->index) : std::nullopt;
            m_threads[thread].addSample(timestamp, stackIndex, cpuDelta, weight);
        }


        void setThreadName(ThreadHandle thread, std::string name) {
            m_threads[thread].setName(std::move(name));
        }

        matjson::Value serializeMeta() const {
            return matjson::makeObject({
                {"categories", m_categories},
                {"debug", false},
                {"extensions", matjson::makeObject({
                    {"length", 0},
                    {"baseURL", matjson::Value::array()},
                    {"id", matjson::Value::array()},
                    {"name", matjson::Value::array()}
                })},
                {"interval", m_interval.toSeconds() * 1000.0},
                {"preprocessedProfileVersion", 57},
                {"processType", 0},
                {"product", m_product},
                {"oscpu", m_osName},
                {"sampleUnits", matjson::makeObject({
                    {"time", m_timelineUnit == TimelineUnit::Milliseconds ? "ms" : "bytes"},
                    {"eventDelay", "ms"},
                    {"threadCPUDelta", "µs"}
                })},
                {"startTime", m_referenceTimestamp.msSinceEpoch},
                // {"symbolicated", m_symbolicated},
                {"version", 24},
                {"usesOnlyOneStackType", true},
                {"sourceCodeIsNotOnSearchfox", true},
                {"markerSchema", matjson::Value::array()},
                {"initialVisibleThreads", m_initialVisibleThreads},
                {"initialSelectedThreads", m_initialSelectedThreads},
            });
        }

        GlobalLibTable const& getGlobalLibs() const {
            return m_globalLibs;
        }

        matjson::Value serializeShared() const {
            return matjson::makeObject({
                {"stringArray", m_stringTable}
            });
        }

        matjson::Value serializeThreads() const {
            return m_threads;
        }

        matjson::Value serializeCounters() const {
            return matjson::Value::array();
        }

    private:
        std::string makeUniquePid(uint32_t pid) {
            return this->makeUniquePidOrTid(m_usedPids, pid);
        }

        std::string makeUniqueTid(uint32_t tid) {
            return this->makeUniquePidOrTid(m_usedTids, tid);
        }

        static std::string makeUniquePidOrTid(std::unordered_map<uint32_t, uint32_t>& usedIds, uint32_t id) {
            auto it = usedIds.find(id);
            if (it == usedIds.end()) {
                usedIds.insert({id, 1});
                return fmt::format("{}", id);
            }

            uint32_t suffix = it->second;
            it->second += 1;
            return fmt::format("{}.{}", id, suffix);
        }

    private:
        std::string m_product;
        std::optional<std::string> m_osName;
        SamplingInterval m_interval;
        TimelineUnit m_timelineUnit = TimelineUnit::Milliseconds;
        GlobalLibTable m_globalLibs;
        LibMappings<LibraryHandle> m_kernelLibs;
        std::unordered_set<InternalCategory> m_categories;
        std::vector<Process> m_processes;
        std::vector<Counter> m_counters;
        std::vector<Thread> m_threads;
        std::vector<ThreadHandle> m_initialVisibleThreads;
        std::vector<ThreadHandle> m_initialSelectedThreads;
        ReferenceTimestamp m_referenceTimestamp;
        std::optional<PlatformSpecificReferenceTimestamp> m_platformSpecificReferenceTimestamp;
        ProfileStringTable m_stringTable;
        std::vector<InternalMarkerSchema> m_markerSchemas;
        std::unordered_map<std::string, MarkerTypeHandle> m_markerTypes;
        std::unordered_map<uint32_t, uint32_t> m_usedPids;
        std::unordered_map<uint32_t, uint32_t> m_usedTids;
        bool symbolicated = false;
    };
}

template <>
struct matjson::Serialize<fxprof::Profile> {
    static Value toJson(fxprof::Profile const& value) {
        return makeObject({
            {"meta", value.serializeMeta()},
            // {"libs", value.getGlobalLibs()},
            {"shared", value.serializeShared()},
            {"threads", value.serializeThreads()},
            {"pages", Value::array()},
            {"profilerOverhead", Value::array()},
            {"counters", value.serializeCounters()},
        });
    }
};