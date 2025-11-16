#pragma once
#include "Category.hpp"
#include "FuncTable.hpp"
#include "IndexSet.hpp"
#include "NativeSymbols.hpp"
#include "ResourceTable.hpp"
#include "StringTable.hpp"

namespace fxprof {
    class InternalFrame {
    public:
        struct Hash {
            size_t operator()(InternalFrame const& frame) const noexcept {
                return std::hash<StringHandle>()(frame.m_name);
            }
        };

        bool operator==(InternalFrame const& other) const noexcept {
            return m_name == other.m_name;
        }

    private:
        StringHandle m_name = 0;
    };

    class FrameTable {
    public:
        std::vector<FuncIndex> const& getFuncColumns() const {
            return m_funcCol;
        }

        std::vector<CategoryHandle> const& getCategoryColumns() const {
            return m_categoryCol;
        }

        std::vector<SubcategoryIndex> const& getSubcategoryColumns() const {
            return m_subcategoryCol;
        }

        std::vector<std::optional<uint32_t>> const& getLineColumns() const {
            return m_lineCol;
        }

        std::vector<std::optional<uint32_t>> const& getColumnColumns() const {
            return m_columnCol;
        }

        std::vector<std::optional<uint32_t>> const& getAddressColumns() const {
            return m_addressCol;
        }

        std::vector<std::optional<NativeSymbolIndex>> const& getNativeSymbolColumns() const {
            return m_nativeSymbolCol;
        }

        std::vector<uint16_t> const& getInlineDepthColumns() const {
            return m_inlineDepthCol;
        }

    private:
        std::vector<FuncIndex> m_funcCol;
        std::vector<CategoryHandle> m_categoryCol;
        std::vector<SubcategoryIndex> m_subcategoryCol;
        std::vector<std::optional<uint32_t>> m_lineCol;
        std::vector<std::optional<uint32_t>> m_columnCol;
        std::vector<std::optional<uint32_t>> m_addressCol;
        std::vector<std::optional<NativeSymbolIndex>> m_nativeSymbolCol;
        std::vector<uint16_t> m_inlineDepthCol;
    };

    class FrameInterner {
    public:
        std::tuple<FrameTable, FuncTable, ResourceTable> createTables() const {
            return {};
        }

    private:
        IndexSet<InternalFrame, InternalFrame::Hash> m_frameKeySet;
        bool m_containsJsFrame = false;
    };
}

template <>
struct matjson::Serialize<fxprof::FrameTable> {
    static Value toJson(fxprof::FrameTable const& value) {
        return makeObject({
            {"length", value.getFuncColumns().size()},
            {"func", value.getFuncColumns()},
            {"category", value.getCategoryColumns()},
            {"subcategory", value.getSubcategoryColumns()},
            {"line", value.getLineColumns()},
            {"column", value.getColumnColumns()},
            {"address", value.getAddressColumns()},
            {"nativeSymbol", value.getNativeSymbolColumns()},
            {"inlineDepth", value.getInlineDepthColumns()},
            {"innerWindowID", std::vector<Value>(value.getFuncColumns().size(), 0)}
        });
    }
};