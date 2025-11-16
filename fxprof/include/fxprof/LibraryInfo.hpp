#pragma once
#include <algorithm>
#include <optional>
#include <vector>
#include <matjson.hpp>
#include <matjson/std.hpp>

namespace fxprof {
    struct LibraryInfo {
        std::string name;
        std::string path;
        std::string debugName;
        std::string debugPath;
        std::string breakpadId;
        std::optional<std::string> codeId;
        std::optional<std::string> arch;

        struct Hash {
            size_t operator()(LibraryInfo const& lib) const noexcept {
                auto h1 = std::hash<std::string>()(lib.name);
                auto h2 = std::hash<std::string>()(lib.path);
                auto h3 = std::hash<std::string>()(lib.debugName);
                auto h4 = std::hash<std::string>()(lib.debugPath);
                auto h5 = std::hash<std::string>()(lib.breakpadId);
                auto h6 = lib.codeId.has_value()
                    ? std::hash<std::string>()(*lib.codeId)
                    : 0;
                auto h7 = lib.arch.has_value()
                    ? std::hash<std::string>()(*lib.arch)
                    : 0;
                return h1 ^ (h2 << 1) ^ (h3 << 2) ^ (h4 << 3) ^ (h5 << 4)
                    ^ (h6 << 5) ^ (h7 << 6);
            }
        };
    };

    struct Symbol {
        std::string name;
        std::optional<uint32_t> size;
        uint32_t address;
    };

    class SymbolTable {
    public:
        SymbolTable(std::vector<Symbol> symbols) {
            std::ranges::sort(symbols, {}, &Symbol::address);
            auto last = std::ranges::unique(symbols, {}, &Symbol::address);
            symbols.erase(last.begin(), last.end());
            m_symbols = std::move(symbols);
        }

        std::optional<std::reference_wrapper<Symbol const>> lookup(uint32_t address) const {
            auto it = std::ranges::lower_bound(
                m_symbols, address, {}, &Symbol::address
            );
            if (it == m_symbols.begin()) {
                return std::nullopt;
            }
            --it;
            auto& symbol = *it;
            if (symbol.size.has_value()) {
                uint32_t endAddress = symbol.address + *symbol.size;
                if (address >= endAddress) {
                    return std::nullopt;
                }
            }
            return symbol;
        }

    private:
        std::vector<Symbol> m_symbols;
    };
}

template <>
struct matjson::Serialize<fxprof::LibraryInfo> {
    static Value toJson(fxprof::LibraryInfo const& value) {
        return makeObject({
            {"name", value.name},
            {"path", value.path},
            {"debugName", value.debugName},
            {"debugPath", value.debugPath},
            {"breakpadId", value.breakpadId},
            {"codeId", value.codeId},
            {"arch", value.arch}
        });
    }
};