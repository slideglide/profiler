#pragma once
#include <optional>
#include <unordered_map>
#include <vector>

namespace fxprof {
    struct Stack {
        std::optional<size_t> parent;
        size_t frame;

        bool operator==(Stack const& other) const noexcept {
            return frame == other.frame && parent == other.parent;
        }
    };

    class StackTable {
    public:
        StackTable() = default;

        size_t indexForStack(std::optional<size_t> prefix, size_t frame) {
            Stack key{ prefix, frame };
            auto it = m_index.find(key);
            if (it != m_index.end()) {
                return it->second;
            }

            auto stackIndex = m_values.size();
            m_values.emplace_back(key);
            m_index.emplace(key, stackIndex);
            return stackIndex;
        }

        size_t len() const {
            return m_values.size();
        }

        std::vector<Stack> const& getAllStacks() const noexcept {
            return m_values;
        }

    private:
        struct StackHash {
            size_t operator()(Stack const& stack) const noexcept {
                size_t hash = std::hash<size_t>()(stack.frame);
                size_t hash2 = std::hash<std::optional<size_t>>()(stack.parent);
                return hash ^ (hash2 << 1);
            }
        };

        std::vector<Stack> m_values;
        std::unordered_map<Stack, size_t, StackHash> m_index;
    };
}

template <>
struct matjson::Serialize<fxprof::StackTable> {
    static Value toJson(fxprof::StackTable const& table) {
        std::vector<Value> prefixes;
        prefixes.reserve(table.len());

        std::vector<Value> frames;
        frames.reserve(table.len());

        for (auto const& [parent, frame] : table.getAllStacks()) {
            prefixes.push_back(parent);
            frames.push_back(frame);
        }

        return makeObject({
            {"length", table.len()},
            {"prefix", std::move(prefixes)},
            {"frame", std::move(frames)}
        });
    }
};