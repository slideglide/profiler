#pragma once
#include <cstdint>
#include <string>

namespace fxprof {
    using StringIndex = uint32_t;

    class StringTable {
    public:
        StringTable() = default;

        StringIndex indexForString(std::string const& str) {
            auto it = m_index.find(str);
            if (it != m_index.end()) {
                return it->second;
            }

            auto stringIndex = static_cast<StringIndex>(m_strings.size());
            m_strings.push_back(str);
            m_index.insert({ str, stringIndex });
            return stringIndex;
        }

        std::string const& getString(StringIndex index) const {
            return m_strings.at(index);
        }

        std::vector<std::string> const& getAllStrings() const {
            return m_strings;
        }

    private:
        std::vector<std::string> m_strings;
        std::unordered_map<std::string, StringIndex> m_index;
    };

    using StringHandle = StringIndex;

    class ProfileStringTable {
    public:
        ProfileStringTable() = default;

        StringHandle indexForString(std::string const& str) {
            return m_table.indexForString(str);
        }

        StringHandle indexForHexAddress(uint64_t address) {
            auto it = m_hexAddressStrings.find(address);
            if (it != m_hexAddressStrings.end()) {
                return it->second;
            }

            auto str = fmt::format("0x{:x}", address);
            auto handle = m_table.indexForString(str);
            m_hexAddressStrings.insert({ address, handle });
            return handle;
        }

        std::string const& getString(StringHandle handle) const {
            return m_table.getString(handle);
        }

        StringTable const& getTable() const {
            return m_table;
        }

    private:
        StringTable m_table;
        std::unordered_map<uint64_t, StringHandle> m_hexAddressStrings;
    };
}

template <>
struct matjson::Serialize<fxprof::StringTable> {
    static Value toJson(fxprof::StringTable const& value) {
        return value.getAllStrings();
    }
};

template <>
struct matjson::Serialize<fxprof::ProfileStringTable> {
    static Value toJson(fxprof::ProfileStringTable const& value) {
        return value.getTable();
    }
};