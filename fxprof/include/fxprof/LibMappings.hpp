#pragma once
#include <cstdint>
#include <map>

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

        void addMapping(
            uint64_t startAVMA,
            uint64_t endAVMA,
            uint32_t relativeAddressAtStart,
            T value
        ) {
            uint64_t removalStartAVMA = startAVMA;

            if (auto m = lookupImpl(startAVMA)) {
                removalStartAVMA = m->get().startAVMA;
            }

            auto it = m_map.lower_bound(removalStartAVMA);
            auto end = m_map.lower_bound(endAVMA);

            m_map.erase(it, end);

            m_map.insert({
                startAVMA,
                MappingType{
                    startAVMA,
                    endAVMA,
                    relativeAddressAtStart,
                    std::move(value)
                }
            });
        }

        std::optional<std::reference_wrapper<MappingType>> lookupImpl(uint64_t avma) {
            if (m_map.empty())
                return std::nullopt;

            auto it = m_map.upper_bound(avma);

            if (it == m_map.begin())
                return std::nullopt;

            --it;

            auto& m = it->second;
            if (avma < m.endAVMA) {
                return m;
            }

            return std::nullopt;
        }

        std::optional<std::reference_wrapper<MappingType const>> lookupImpl(uint64_t avma) const {
            if (m_map.empty())
                return std::nullopt;

            auto it = m_map.upper_bound(avma);

            if (it == m_map.begin())
                return std::nullopt;

            --it;

            auto& m = it->second;
            if (avma < m.endAVMA) {
                return m;
            }

            return std::nullopt;
        }

        void clear() {
            m_map.clear();
        }

        std::optional<std::pair<uint32_t, Type>> removeMapping(uint64_t startAVMA) {
            auto it = m_map.find(startAVMA);
            if (it == m_map.end()) {
                return std::nullopt;
            }

            MappingType mapping = it->second;
            m_map.erase(it);
            return std::make_pair(mapping.relativeAddressAtStart, std::move(mapping.value));
        }

        std::optional<std::reference_wrapper<Type>> lookup(uint64_t avma) const {
            if (auto m = lookupImpl(avma)) {
                return std::ref(m->get().value);
            }
            return std::nullopt;
        }

        std::optional<std::pair<uint32_t, std::reference_wrapper<Type const>>> convertAddress(uint64_t avma) const {
            auto mappingOpt = lookupImpl(avma);
            if (!mappingOpt)
                return std::nullopt;

            auto& m = mappingOpt->get();
            uint32_t offsetFromStart = static_cast<uint32_t>(avma - m.startAVMA);
            uint32_t relativeAddress = m.relativeAddressAtStart + offsetFromStart;

            return std::make_pair(relativeAddress, std::ref(m.value));
        }

    private:
        std::map<uint64_t, MappingType> m_map;
    };
}