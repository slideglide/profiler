#pragma once
#include <cstdint>

#include "Frame.hpp"
#include "ResourceTable.hpp"

namespace fxprof {
    using FuncIndex = uint32_t;

    struct FuncKey {
        StringHandle name;
        std::optional<StringHandle> filePath;
        std::optional<GlobalLibIndex> lib;
        FrameFlags flags;

        struct Hash {
            size_t operator()(FuncKey const& funcKey) const noexcept {
                auto h1 = std::hash<StringHandle>()(funcKey.name);
                auto h2 = funcKey.filePath.has_value()
                    ? std::hash<StringHandle>()(*funcKey.filePath)
                    : 0;
                auto h3 = funcKey.lib.has_value()
                    ? std::hash<GlobalLibIndex>()(*funcKey.lib)
                    : 0;
                auto h4 = std::hash<uint8_t>()(static_cast<uint8_t>(funcKey.flags));
                return h1 ^ (h2 << 1) ^ (h3 << 2) ^ (h4 << 3);
            }
        };
    };

    class FuncTable {
    public:
        std::vector<StringHandle> const& getNameColumns() const { return m_names; }
        std::vector<std::optional<StringHandle>> const& getFileColumns() const { return m_files; }
        std::vector<std::optional<ResourceIndex>> const& getResourceColumns() const { return m_resources; }
        std::vector<FrameFlags> const& getFlagsColumns() const { return m_flags; }
        IndexSet<FuncKey, FuncKey::Hash> const& getFuncKeySet() const { return m_funcKeySet; }

        FuncIndex indexForFunc(FuncKey const& funcKey, ResourceTable& resourceTable) {
            auto [index, inserted] = m_funcKeySet.insert(funcKey);

            if (!inserted) {
                return static_cast<FuncIndex>(index);
            }

            auto resource = funcKey.lib.has_value()
                ? std::optional(resourceTable.resourceForLib(*funcKey.lib))
                : std::nullopt;

            m_names.push_back(funcKey.name);
            m_files.push_back(funcKey.filePath);
            m_resources.push_back(resource);
            m_flags.push_back(funcKey.flags);

            return static_cast<FuncIndex>(index);
        }

    private:
        std::vector<StringHandle> m_names;
        std::vector<std::optional<StringHandle>> m_files;
        std::vector<std::optional<ResourceIndex>> m_resources;
        std::vector<FrameFlags> m_flags;
        IndexSet<FuncKey, FuncKey::Hash> m_funcKeySet;
    };
};

template <>
struct matjson::Serialize<fxprof::FuncTable> {
    static Value toJson(fxprof::FuncTable const& value) {
        std::vector<Value> isJS;
        std::vector<Value> relevantForJS;
        std::vector<Value> resources;
        isJS.reserve(value.getFlagsColumns().size());
        relevantForJS.reserve(value.getFlagsColumns().size());
        resources.reserve(value.getFlagsColumns().size());

        for (size_t i = 0; i < value.getFlagsColumns().size(); ++i) {
            fxprof::FrameFlags flags = value.getFlagsColumns()[i];
            isJS.push_back((flags & fxprof::FrameFlags::IsJS) != fxprof::FrameFlags::None);
            relevantForJS.push_back((flags & fxprof::FrameFlags::IsRelevantForJs) != fxprof::FrameFlags::None);
            resources.push_back(value.getResourceColumns()[i].has_value()
                ? Value(*value.getResourceColumns()[i])
                : -1);
        }

        return makeObject({
            {"length", value.getNameColumns().size()},
            {"name", value.getNameColumns()},
            {"isJS", std::move(isJS)},
            {"relevantForJS", std::move(relevantForJS)},
            {"resource", std::move(resources)},
            {"fileName", value.getFileColumns()},
            {"lineNumber", std::vector<Value>(value.getNameColumns().size(), nullptr)},
            {"columnNumber", std::vector<Value>(value.getNameColumns().size(), nullptr)},
        });
    }
};