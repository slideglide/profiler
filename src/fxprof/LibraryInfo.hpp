#pragma once

namespace fxprof {
    struct LibraryInfo {
        std::string name;
        std::string path;
        std::string debugName;
        std::string debugPath;
        std::string breakpadId;
        std::optional<std::string> codeId;
        std::optional<std::string> arch;
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

        std::optional<std::reference_wrapper<Symbol>> lookup(uint32_t address) {
            auto it = std::ranges::lower_bound(
                m_symbols, address, {}, &Symbol::address
            );
            if (it == m_symbols.begin()) {
                return std::nullopt;
            }
            --it;
            Symbol& symbol = *it;
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