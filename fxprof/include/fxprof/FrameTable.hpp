#pragma once
#include "Category.hpp"
#include "FuncTable.hpp"
#include "IndexSet.hpp"
#include "NativeSymbols.hpp"
#include "ResourceTable.hpp"
#include "StringTable.hpp"

namespace fxprof {
    struct SourceLocation {
        std::optional<StringHandle> filePath;
        std::optional<uint32_t> line;
        std::optional<uint32_t> column;
    };

    struct FrameSymbolInfo {
        std::optional<StringHandle> name;
        NativeSymbolHandle nativeSymbol;
        SourceLocation sourceLocation;
    };

    struct NativeFrameData {
        GlobalLibIndex lib;
        std::optional<NativeSymbolIndex> nativeSymbol;
        uint32_t relativeAddress = 0;
        uint16_t inlineDepth = 0;

        bool operator==(NativeFrameData const& other) const noexcept {
            return lib == other.lib && nativeSymbol == other.nativeSymbol
                && relativeAddress == other.relativeAddress
                && inlineDepth == other.inlineDepth;
        }
    };

    struct LabelFrame {
        bool operator==(LabelFrame const&) const noexcept { return true; }
    };

    using InternalFrameVariant = std::variant<LabelFrame, NativeFrameData>;

    class InternalFrame {
    public:
        InternalFrame(
            StringHandle name,
            InternalFrameVariant variant,
            SubcategoryHandle subcategory,
            SourceLocation sourceLocation,
            FrameFlags flags
        ) : m_name(name),
            m_variant(std::move(variant)),
            m_subcategory(subcategory),
            m_sourceLocation(std::move(sourceLocation)),
            m_flags(flags) {}

        struct Hash {
            size_t operator()(InternalFrame const& frame) const noexcept {
                return std::hash<StringHandle>()(frame.m_name);
            }
        };

        bool operator==(InternalFrame const& other) const noexcept {
            return m_name == other.m_name && m_variant == other.m_variant
                && m_subcategory == other.m_subcategory
                && m_sourceLocation.filePath == other.m_sourceLocation.filePath
                && m_sourceLocation.line == other.m_sourceLocation.line
                && m_sourceLocation.column == other.m_sourceLocation.column
                && m_flags == other.m_flags;
        }

        bool isJsFrame() const {
            return (m_flags & (FrameFlags::IsJS | FrameFlags::IsRelevantForJs)) != FrameFlags::None;
        }

        FuncKey funcKey() const {
            std::optional<GlobalLibIndex> globalLibIndex;
            if (m_variant.index() == 1) {
                globalLibIndex = std::get<NativeFrameData>(m_variant).lib;
            }

            return FuncKey{
                m_name,
                m_sourceLocation.filePath,
                globalLibIndex,
                m_flags
            };
        }

        StringHandle getName() const {
            return m_name;
        }

        InternalFrameVariant const& getVariant() const {
            return m_variant;
        }

        SubcategoryHandle getSubcategory() const {
            return m_subcategory;
        }

        SourceLocation const& getSourceLocation() const {
            return m_sourceLocation;
        }

        FrameFlags getFlags() const {
            return m_flags;
        }

    private:
        StringHandle m_name = 0;
        InternalFrameVariant m_variant;
        SubcategoryHandle m_subcategory{};
        SourceLocation m_sourceLocation;
        FrameFlags m_flags = FrameFlags::None;
    };

    struct InternalFrameAddress {
        union {
            uint64_t address;
            struct {
                GlobalLibIndex library;
                uint32_t offset;
            };
        };
        enum class Type : uint8_t {
            Unknown,
            InLib,
        } type = Type::Unknown;
    };

    class FrameTable {
    public:
        FrameTable(
            std::vector<FuncIndex> funcCol,
            std::vector<CategoryHandle> categoryCol,
            std::vector<SubcategoryIndex> subcategoryCol,
            std::vector<std::optional<uint32_t>> lineCol,
            std::vector<std::optional<uint32_t>> columnCol,
            std::vector<std::optional<uint32_t>> addressCol,
            std::vector<std::optional<NativeSymbolIndex>> nativeSymbolCol,
            std::vector<uint16_t> inlineDepthCol
        ) : m_funcCol(std::move(funcCol)),
            m_categoryCol(std::move(categoryCol)),
            m_subcategoryCol(std::move(subcategoryCol)),
            m_lineCol(std::move(lineCol)),
            m_columnCol(std::move(columnCol)),
            m_addressCol(std::move(addressCol)),
            m_nativeSymbolCol(std::move(nativeSymbolCol)),
            m_inlineDepthCol(std::move(inlineDepthCol)) {}

        std::vector<FuncIndex> const& getFuncColumns() const { return m_funcCol; }
        std::vector<CategoryHandle> const& getCategoryColumns() const { return m_categoryCol; }
        std::vector<SubcategoryIndex> const& getSubcategoryColumns() const { return m_subcategoryCol; }
        std::vector<std::optional<uint32_t>> const& getLineColumns() const { return m_lineCol; }
        std::vector<std::optional<uint32_t>> const& getColumnColumns() const { return m_columnCol; }
        std::vector<std::optional<uint32_t>> const& getAddressColumns() const { return m_addressCol; }
        std::vector<std::optional<NativeSymbolIndex>> const& getNativeSymbolColumns() const { return m_nativeSymbolCol; }
        std::vector<uint16_t> const& getInlineDepthColumns() const { return m_inlineDepthCol; }

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
            auto len = m_frameKeySet.size();

            std::vector<FuncIndex> funcCol;
            std::vector<CategoryHandle> categoryCol;
            std::vector<SubcategoryIndex> subcategoryCol;
            std::vector<std::optional<uint32_t>> lineCol;
            std::vector<std::optional<uint32_t>> columnCol;
            std::vector<std::optional<uint32_t>> addressCol;
            std::vector<std::optional<NativeSymbolIndex>> nativeSymbolCol;
            std::vector<uint16_t> inlineDepthCol;

            funcCol.reserve(len);
            categoryCol.reserve(len);
            subcategoryCol.reserve(len);
            lineCol.reserve(len);
            columnCol.reserve(len);
            addressCol.reserve(len);
            nativeSymbolCol.reserve(len);
            inlineDepthCol.reserve(len);

            FuncTable funcTable;
            ResourceTable resourceTable;

            for (auto& frame : m_frameKeySet) {
                auto funcKey = frame.funcKey();
                auto func = funcTable.indexForFunc(funcKey, resourceTable);

                funcCol.push_back(func);
                categoryCol.push_back(frame.getSubcategory().category);
                subcategoryCol.push_back(frame.getSubcategory().subcategory);
                auto& sourceLocation = frame.getSourceLocation();
                lineCol.push_back(sourceLocation.line);
                columnCol.push_back(sourceLocation.column);

                std::visit([&](auto&& variant) {
                    using T = std::decay_t<decltype(variant)>;
                    if constexpr (std::is_same_v<T, LabelFrame>) {
                        addressCol.push_back(std::nullopt);
                        nativeSymbolCol.push_back(std::nullopt);
                        inlineDepthCol.push_back(0);
                    } else if constexpr (std::is_same_v<T, NativeFrameData>) {
                        addressCol.push_back(variant.relativeAddress);
                        nativeSymbolCol.push_back(variant.nativeSymbol);
                        inlineDepthCol.push_back(variant.inlineDepth);
                    }
                }, frame.getVariant());
            }

            return {
                FrameTable{
                    std::move(funcCol),
                    std::move(categoryCol),
                    std::move(subcategoryCol),
                    std::move(lineCol),
                    std::move(columnCol),
                    std::move(addressCol),
                    std::move(nativeSymbolCol),
                    std::move(inlineDepthCol)
                },
                std::move(funcTable),
                std::move(resourceTable)
            };
        }

        size_t indexForFrame(InternalFrame frame) {
            auto [index, inserted] = m_frameKeySet.insert(std::move(frame));
            if (inserted && frame.isJsFrame()) {
                m_containsJsFrame = true;
            }
            return index;
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