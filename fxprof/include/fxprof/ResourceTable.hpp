#pragma once
#include <cstdint>
#include <unordered_map>
#include <matjson.hpp>
#include <matjson/std.hpp>
#include <matjson/stl_serialize.hpp>

#include "GlobalLibTable.hpp"

namespace fxprof {
    using ResourceIndex = uint32_t;

    class ResourceTable {
    public:
        ResourceIndex resourceForLib(GlobalLibIndex libIndex) {
            auto it = m_libToResource.find(libIndex);
            if (it != m_libToResource.end()) {
                return it->second;
            }

            ResourceIndex resourceIndex = static_cast<ResourceIndex>(m_resourceLibs.size());
            m_resourceLibs.push_back(libIndex);
            m_resourceNames.push_back(libIndex.name);
            m_libToResource.insert({ libIndex, resourceIndex });
            return resourceIndex;
        }

        std::vector<GlobalLibIndex> const& getResourceLibs() const {
            return m_resourceLibs;
        }

        std::vector<StringIndex> const& getResourceNames() const {
            return m_resourceNames;
        }

    private:
        std::vector<GlobalLibIndex> m_resourceLibs;
        std::vector<StringIndex> m_resourceNames;
        std::unordered_map<GlobalLibIndex, ResourceIndex> m_libToResource;
    };
};

template <>
struct matjson::Serialize<fxprof::ResourceTable> {
    static Value toJson(fxprof::ResourceTable const& value) {
        constexpr uint32_t RESOURCE_TYPE_LIB = 1;
        return makeObject({
            {"length", value.getResourceLibs().size()},
            {"lib", value.getResourceLibs()},
            {"name", value.getResourceNames()},
            {"host", std::vector<Value>(value.getResourceLibs().size(), nullptr)},
            {"type", std::vector<Value>(value.getResourceLibs().size(), RESOURCE_TYPE_LIB)},
        });
    }
};