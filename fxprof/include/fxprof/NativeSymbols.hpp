#pragma once
#include <cstdint>

namespace fxprof {
    using NativeSymbolIndex = uint32_t;
    using ThreadHandle = size_t;

    struct NativeSymbolHandle {
        ThreadHandle thread;
        NativeSymbolIndex index;
    };

    class NativeSymbols {
    public:
        std::pair<NativeSymbolIndex, StringHandle> symbolIndexAndStringIndexForSymbol(
            GlobalLibIndex libIndex,
            Symbol const& symbol,
            ProfileStringTable& stringTable
        ) {
            auto nameStringIndex = stringTable.indexForString(symbol.name);
            auto symbolIndex = this->symbolIndexForSymbol(
                libIndex,
                symbol.address,
                symbol.size,
                nameStringIndex
            );
            return { symbolIndex, nameStringIndex };
        }

        NativeSymbolIndex symbolIndexForSymbol(
            GlobalLibIndex libIndex,
            uint32_t address,
            std::optional<uint32_t> size,
            StringHandle nameHandle
        ) {
            Key key{ libIndex, address };
            auto it = m_symbolMap.find(key);
            if (it != m_symbolMap.end()) {
                return it->second;
            }

            NativeSymbolIndex symbolIndex = static_cast<NativeSymbolIndex>(m_addresses.size());
            m_addresses.push_back(address);
            m_functionSizes.push_back(size);
            m_libIndexes.push_back(libIndex);
            m_names.push_back(nameHandle);
            m_symbolMap.insert({ key, symbolIndex });
            return symbolIndex;
        }

    private:
        std::vector<uint32_t> m_addresses;
        std::vector<std::optional<uint32_t>> m_functionSizes;
        std::vector<GlobalLibIndex> m_libIndexes;
        std::vector<StringHandle> m_names;

        struct Key {
            GlobalLibIndex libIndex;
            uint32_t address;
            bool operator==(Key const& other) const noexcept {
                return libIndex == other.libIndex && address == other.address;
            }

            struct Hash {
                size_t operator()(Key const& key) const noexcept {
                    auto h1 = std::hash<GlobalLibIndex>()(key.libIndex);
                    auto h2 = std::hash<uint32_t>()(key.address);
                    return h1 ^ (h2 << 1);
                }
            };
        };

        std::unordered_map<Key, NativeSymbolIndex, Key::Hash> m_symbolMap;
    };
}