#pragma once
#include <cstdint>
#include <filesystem>
#include <optional>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include <matjson.hpp>
#include <fmt/format.h>

#define BUILDER_HELPER(ClassName, Method, ...) \
    ClassName& Method & noexcept { __VA_ARGS__; return *this; } \
    [[nodiscard]] ClassName&& Method && noexcept { __VA_ARGS__; return std::move(*this); }

namespace gecko {
    using StringIndex = uint32_t;

    struct Frame {
        union {
            uintptr_t address;
            StringIndex label;
        };
        bool isLabel = false;

        static Frame fromAddress(uintptr_t address) {
            Frame ret;
            ret.address = address;
            ret.isLabel = false;
            return ret;
        }

        static Frame fromLabel(StringIndex label) {
            Frame ret;
            ret.label = label;
            ret.isLabel = true;
            return ret;
        }

        bool operator==(Frame const& other) const {
            return isLabel == other.isLabel && (isLabel ? label == other.label : address == other.address);
        }

    private:
        Frame() = default;
    };

    class Lib {
    public:
        Lib(
            std::filesystem::path path,
            std::filesystem::path debugPath,
            std::string arch,
            std::string breakpadId,
            std::optional<std::string> codeId
        ) : m_path(std::move(path)), m_debugPath(std::move(debugPath)), m_arch(std::move(arch)),
            m_breakpadId(std::move(breakpadId)), m_codeId(std::move(codeId)) {}

        [[nodiscard]] matjson::Value toJson() const {
            matjson::Value obj = matjson::Value::object();
            obj["name"] = m_path.filename();
            obj["path"] = m_path;
            obj["debugName"] = m_debugPath.filename();
            obj["debugPath"] = m_debugPath;
            obj["arch"] = m_arch;
            obj["breakpadId"] = m_breakpadId;
            obj["codeId"] = m_codeId;
            obj["start"] = m_baseAddress;
            obj["end"] = m_endAddress;
            return obj;
        }

    private:
        std::filesystem::path m_path;
        std::filesystem::path m_debugPath;
        std::string m_arch;
        std::string m_breakpadId;
        std::optional<std::string> m_codeId;
        uintptr_t m_baseAddress = 0;
        uintptr_t m_endAddress = 0;
    };

    class StackTable {
    public:
        StackTable() = default;

        std::optional<size_t> indexForFrames(std::span<size_t const> frameIndexes) {
            std::optional<size_t> parent = std::nullopt;
            for (size_t frameIndex : frameIndexes) {
                Stack stack{ parent, frameIndex };
                auto it = m_stackMap.find(stack);
                if (it != m_stackMap.end()) {
                    parent = it->second;
                } else {
                    size_t newIndex = m_stacks.size();
                    m_stacks.push_back(stack);
                    m_stackMap.insert({ stack, newIndex });
                    parent = newIndex;
                }
            }
            return parent;
        }

    private:
        struct Stack {
            std::optional<size_t> parent;
            size_t frame;

            bool operator==(Stack const& other) const {
                return frame == other.frame && parent == other.parent;
            }
        };

        struct StackHash {
            size_t operator()(Stack const& stack) const {
                size_t hash = std::hash<size_t>()(stack.frame);
                size_t hash2 = std::hash<std::optional<size_t>>()(stack.parent);
                return hash ^ (hash2 << 1);
            }
        };

        std::vector<Stack> m_stacks;
        std::unordered_map<Stack, size_t, StackHash> m_stackMap;
    };

    class StringTable {
    public:
        StringTable() = default;

        StringIndex indexForString(std::string const& str) {
            auto it = m_stringMap.find(str);
            if (it != m_stringMap.end()) {
                return it->second;
            }

            auto stringIndex = static_cast<StringIndex>(m_strings.size());
            m_strings.push_back(str);
            m_stringMap.insert({ str, stringIndex });
            return stringIndex;
        }

    private:
        std::vector<std::string> m_strings;
        std::unordered_map<std::string, StringIndex> m_stringMap;
    };

    class FrameTable {
    public:
        FrameTable() = default;

        size_t indexForFrame(StringTable& stringTable, Frame frame) {
            auto it = m_frameMap.find(frame);
            if (it != m_frameMap.end()) {
                return it->second;
            }

            auto frameIndex = m_frames.size();
            size_t locationStringIndex;
            if (frame.isLabel) {
                locationStringIndex = stringTable.indexForString(fmt::format("0x{:x}", frame.address));
            } else {
                locationStringIndex = frame.label;
            }
            m_frames.push_back(locationStringIndex);
            m_frameMap.insert({ frame, frameIndex });
            return frameIndex;
        }

    private:
        struct FrameHash {
            size_t operator()(Frame const& frame) const {
                size_t hash = std::hash<bool>()(frame.isLabel);
                size_t hash2 = frame.isLabel ? std::hash<StringIndex>()(frame.label) : std::hash<uintptr_t>()(frame.address);
                return hash ^ (hash2 << 1);
            }
        };

        std::vector<StringIndex> m_frames;
        std::unordered_map<Frame, size_t, FrameHash> m_frameMap;
    };

    struct Sample {
        uint64_t timestamp;
        std::optional<size_t> stackIndex;
        uint64_t cpuDeltaNanos = 0;
    };

    using SampleTable = std::vector<Sample>;

    struct Marker {
        StringIndex nameStringIndex;
        // MarkerTiming timing;
        // Value data;
    };

    using MarkerTable = std::vector<Marker>;

    class MarkerSchema {};

    class ThreadBuilder {
    public:
        ThreadBuilder(uint32_t pid, uint32_t index, uint64_t startTime, bool isMain, bool isLibDispatchThread)
            : m_pid(pid), m_index(index), m_startTime(startTime),
              m_isMain(isMain), m_isLibDispatchThread(isLibDispatchThread) {}

        [[nodiscard]] uint32_t getIndex() const { return m_index; }
    private:
        uint32_t m_pid = 0;
        uint32_t m_index = 0;
        std::optional<std::string> m_name;
        uint64_t m_startTime = 0;
        std::optional<uint64_t> m_endTime = std::nullopt;
        StackTable m_stackTable;
        FrameTable m_frameTable;
        SampleTable m_sampleTable;
        MarkerTable m_markerTable;
        std::unordered_map<std::string_view, MarkerSchema> m_markerSchemas;
        StringTable m_stringTable;
        bool m_isMain = false;
        bool m_isLibDispatchThread = false;
    };

    class ProfileBuilder {
    public:
        ProfileBuilder(
            uint64_t startTime,
            uint64_t startTimeSystem,
            std::string commandName,
            uint32_t pid,
            uint32_t interval
        ) : m_pid(pid), m_interval(interval), m_startTime(startTime),
            m_startTimeSystem(startTimeSystem), m_commandName(std::move(commandName)) {}

        BUILDER_HELPER(ProfileBuilder, setStartTime(uint64_t startTime), m_startTime = startTime)
        BUILDER_HELPER(ProfileBuilder, setStartTimeSystem(uint64_t startTimeSystem), m_startTimeSystem = startTimeSystem)
        BUILDER_HELPER(ProfileBuilder, setEndTime(uint64_t endTime), m_endTime = endTime)
        BUILDER_HELPER(ProfileBuilder, setCommandName(std::string commandName), m_commandName = std::move(commandName))
        BUILDER_HELPER(ProfileBuilder, setPid(uint32_t pid), m_pid = pid)
        BUILDER_HELPER(ProfileBuilder, setInterval(uint32_t interval), m_interval = interval)
        BUILDER_HELPER(ProfileBuilder, addLib(Lib lib), m_libs.push_back(std::move(lib)))
        BUILDER_HELPER(ProfileBuilder, addThread(ThreadBuilder thread), m_threads.emplace(thread.getIndex(), std::move(thread)))
        BUILDER_HELPER(ProfileBuilder, addSubprocess(ProfileBuilder subprocess), m_subprocesses.push_back(std::move(subprocess)))

        [[nodiscard]] matjson::Value toJson() const {
            auto meta = matjson::makeObject({
                {"version", 24},
                {"startTime", m_startTime},
                {"shutdownTime", m_endTime},
                {"pausedRanges", matjson::Value::array()},
                {"product", m_commandName},
                {"interval", m_interval},
                {"pid", m_pid},
                {"processType", 0},
                {"categories", std::vector{
                    matjson::makeObject({
                        {"name", "Other"},
                        {"color", "grey"},
                        {"subcategories", std::vector<matjson::Value>{"Other"}}
                    })
                }},
                {"sampleUnits", matjson::makeObject({
                    {"time", "ms"},
                    {"eventDelay", "ms"},
                    {"threadCPUDelta", "µs"}
                })},
                {"markerSchema", matjson::Value::array()}
            });

            auto subprocesses = std::vector<matjson::Value>();
            subprocesses.reserve(m_subprocesses.size());
            for (auto const& subprocess : m_subprocesses) {
                subprocesses.push_back(subprocess.toJson());
            }

            return matjson::makeObject({
                {"meta", std::move(meta)},
                {"libs", matjson::Value::array()},
                {"threads", matjson::Value::array()},
                {"processes", std::move(subprocesses)}
            });
        }

    private:
        uint32_t m_pid = 0;
        uint32_t m_interval = 1;
        std::vector<Lib> m_libs;
        std::unordered_map<uint32_t, ThreadBuilder> m_threads;
        uint64_t m_startTime = 0;
        uint64_t m_startTimeSystem = 0;
        std::optional<uint64_t> m_endTime = std::nullopt;
        std::string m_commandName;
        std::vector<ProfileBuilder> m_subprocesses;
    };
}

#undef BUILDER_HELPER

template<>
struct matjson::Serialize<gecko::ProfileBuilder> {
    static geode::Result<gecko::ProfileBuilder> fromJson(Value const& value) {
        return geode::Err("Deserialization of ProfileBuilder is not supported");
    }

    static Value toJson(gecko::ProfileBuilder const& value) {
        return value.toJson();
    }
};