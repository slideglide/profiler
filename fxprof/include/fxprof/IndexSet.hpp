#pragma once
#include <unordered_map>
#include <vector>

#include <matjson.hpp>
#include <matjson/std.hpp>

namespace fxprof {
    /// @brief A set with stable indices
    /// @tparam T The type of the elements in the set
    /// @tparam Hash The hash function to use for the elements
    template <typename T, typename Hash = std::hash<T>>
    class IndexSet {
    public:
        std::pair<size_t, bool> insert(T const& value) {
            auto hash = Hash{}(value);
            auto it = m_index.find(hash);
            if (it != m_index.end()) {
                return { it->second, false };
            }

            size_t index = m_values.size();
            m_values.push_back(value);
            m_index.insert({ hash, index });
            return { index, true };
        }

        std::pair<size_t, bool> insert(T&& value) {
            auto hash = Hash{}(value);
            auto it = m_index.find(hash);
            if (it != m_index.end()) {
                return { it->second, false };
            }

            size_t index = m_values.size();
            m_values.push_back(std::move(value));
            m_index.insert({ hash, index });
            return { index, true };
        }

        bool contains(T const& value) const {
            return m_index.contains(Hash{}(value));
        }

        std::optional<size_t> indexOf(T const& value) const {
            auto hash = Hash{}(value);
            auto it = m_index.find(hash);
            if (it != m_index.end()) {
                return it->second;
            }
            return std::nullopt;
        }

        size_t size() const noexcept { return m_values.size(); }
        std::vector<T> const& vec() const noexcept { return m_values; }

        auto begin() const noexcept { return m_values.begin(); }
        auto end() const noexcept { return m_values.end(); }

    private:
        std::vector<T> m_values;
        std::unordered_map<size_t, size_t> m_index;
    };
}

template <typename T, typename Hash>
struct matjson::Serialize<fxprof::IndexSet<T, Hash>> {
    static Value toJson(fxprof::IndexSet<T, Hash> const& value) {
        return value.vec();
    }
};