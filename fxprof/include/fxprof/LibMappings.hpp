#pragma once
#include <cstdint>
#include <unordered_map>

namespace fxprof {
    template <typename T>
    struct Mapping {
        uint64_t startAVMA;
        uint64_t endAVMA;
        uint32_t relativeAddressAtStart;
        T value;
    };

    template <typename T>
    class LibMappings {
    public:
        using MappingType = Mapping<T>;
        using Type = T;

    private:
        std::unordered_map<uint64_t, MappingType> m_map;
    };
}