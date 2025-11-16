#pragma once
#include <vector>
#include <unordered_map>

namespace fxprof {
    template <typename T, typename Hash = std::hash<T>>
    class IndexSet {
    public:
        std::pair<size_t, bool> insert(T const& value) {
            auto it = m_index.find(value);
            if (it != m_index.end()) {
                return { it->second, false };
            }

            size_t index = m_values.size();
            m_values.push_back(value);
            m_index.insert({ value, index });
            return { index, true };
        }

        size_t size() const noexcept { return m_values.size(); }
        std::vector<T> const& vec() const noexcept { return m_values; }

        auto begin() const noexcept { return m_values.begin(); }
        auto end() const noexcept { return m_values.end(); }

    private:
        std::vector<T> m_values;
        std::unordered_map<T, size_t, Hash> m_index;
    };
}