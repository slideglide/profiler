#pragma once
#include "LibraryInfo.hpp"
#include "StringTable.hpp"

namespace fxprof {
    struct GlobalLibTable {

    };

    using LibraryHandle = size_t;

    struct GlobalLibIndex {
        size_t index;
        StringHandle name;

        bool operator==(GlobalLibIndex const& other) const noexcept {
            return index == other.index && name == other.name;
        }
    };
}

template <>
struct std::hash<fxprof::GlobalLibIndex> {
    size_t operator()(fxprof::GlobalLibIndex const& libIndex) const noexcept {
        auto h1 = std::hash<size_t>()(libIndex.index);
        auto h2 = std::hash<fxprof::StringHandle>()(libIndex.name);
        return h1 ^ (h2 << 1);
    }
};

template <>
struct matjson::Serialize<fxprof::GlobalLibIndex> {
    static Value toJson(fxprof::GlobalLibIndex const& libIndex) {
        return libIndex.index;
    }
};