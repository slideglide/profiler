#pragma once
#include <cstdint>
#include <optional>
#include <vector>

#include "CpuDelta.hpp"
#include "Timestamp.hpp"

namespace fxprof {
    enum class WeightType : uint8_t {
        Samples,
        TracingMs,
        Bytes,
    };

    inline std::string_view format_as(WeightType type) {
        switch (type) {
            case WeightType::Samples: return "samples";
            case WeightType::TracingMs: return "tracing-ms";
            case WeightType::Bytes: return "bytes";
        }
        return "unknown";
    }

    class SampleTable {
    public:
        WeightType getType() const { return m_sampleWeightType; }
        std::vector<int32_t> const& getWeights() const { return m_sampleWeights; }
        std::vector<Timestamp> const& getTimestamps() const { return m_sampleTimestamps; }
        std::vector<std::optional<size_t>> const& getStackIndexes() const { return m_sampleStackIndexes; }
        std::vector<CpuDelta> const& getCpuDeltas() const { return m_sampleCpuDeltas; }
        Timestamp getLastSampleTimestamp() const { return m_lastSampleTimestamp; }
        bool isSortedByTime() const { return m_isSortedByTime; }

        void addSample(
            Timestamp timestamp,
            std::optional<size_t> stackIndex,
            CpuDelta cpuDelta,
            int32_t weight
        ) {
            m_sampleWeights.push_back(weight);
            m_sampleTimestamps.push_back(timestamp);
            m_sampleStackIndexes.push_back(stackIndex);
            m_sampleCpuDeltas.push_back(cpuDelta);
            if (timestamp < m_lastSampleTimestamp) {
                m_isSortedByTime = false;
            }
            m_lastSampleTimestamp = timestamp;
        }

    private:
        WeightType m_sampleWeightType = WeightType::Samples;
        std::vector<int32_t> m_sampleWeights;
        std::vector<Timestamp> m_sampleTimestamps;
        std::vector<std::optional<size_t>> m_sampleStackIndexes;
        std::vector<CpuDelta> m_sampleCpuDeltas;
        Timestamp m_lastSampleTimestamp = Timestamp::fromNanos(0);
        bool m_isSortedByTime = true;
    };
}

template <>
struct matjson::Serialize<fxprof::SampleTable> {
    static Value toJson(fxprof::SampleTable const& value) {
        auto obj = makeObject({
            {"length", value.getTimestamps().size()},
            {"weightType", format_as(value.getType())},
        });

        if (value.isSortedByTime()) {
            obj.set("stack", value.getStackIndexes());
            obj.set("timeDeltas", value.getTimestamps());
            obj.set("weights", value.getWeights());
            obj.set("threadCPUDelta", value.getCpuDeltas());
        } else {
            // sort by time
            std::vector<size_t> indices(value.getTimestamps().size());
            for (size_t i = 0; i < indices.size(); ++i) {
                indices[i] = i;
            }
            std::ranges::sort(indices, [&](size_t a, size_t b) {
                return value.getTimestamps()[a].nanos < value.getTimestamps()[b].nanos;
            });

            // sorted arrays
            std::vector<std::optional<size_t>> sortedStacks;
            std::vector<fxprof::Timestamp> sortedTimestamps;
            std::vector<fxprof::CpuDelta> sortedCpuDeltas;
            std::vector<int32_t> sortedWeights;
            sortedStacks.reserve(indices.size());
            sortedTimestamps.reserve(indices.size());
            sortedCpuDeltas.reserve(indices.size());
            sortedWeights.reserve(indices.size());
            for (auto index : indices) {
                sortedStacks.push_back(value.getStackIndexes()[index]);
                sortedTimestamps.push_back(value.getTimestamps()[index]);
                sortedCpuDeltas.push_back(value.getCpuDeltas()[index]);
                sortedWeights.push_back(value.getWeights()[index]);
            }

            // set sorted arrays
            obj.set("stack", sortedStacks);
            obj.set("timeDeltas", sortedTimestamps);
            obj.set("weights", sortedWeights);
            obj.set("threadCPUDelta", sortedCpuDeltas);
        }

        return obj;
    }
};