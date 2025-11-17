#pragma once
#include "IndexSet.hpp"
#include "LibraryInfo.hpp"
#include "StringTable.hpp"

namespace fxprof {
    using LibraryHandle = size_t;

    struct GlobalLibIndex {
        size_t index;
        StringHandle name;

        bool operator==(GlobalLibIndex const& other) const noexcept {
            return index == other.index && name == other.name;
        }
    };

    class GlobalLibTable {
    public:
        LibraryHandle handleForLib(LibraryInfo library) {
            return m_allLibs.insert(std::move(library)).first;
        }

        IndexSet<LibraryInfo, LibraryInfo::Hash> const& getAllLibs() const {
            return m_allLibs;
        }

        std::optional<std::reference_wrapper<SymbolTable const>> getLibSymbolTable(GlobalLibIndex libIndex) const {
            auto handle = m_usedLibs[libIndex.index];
            auto it = m_symbolTables.find(handle);
            if (it != m_symbolTables.end()) {
                return std::ref(it->second);
            }
            return std::nullopt;
        }

        void setLibSymbolTable(LibraryHandle libHandle, SymbolTable symbolTable) {
            m_symbolTables.insert({ libHandle, std::move(symbolTable) });
        }

        GlobalLibIndex indexForUsedLib(LibraryHandle libHandle, ProfileStringTable& stringTable) {
            auto it = m_usedLibMap.find(libHandle);
            if (it != m_usedLibMap.end()) {
                return it->second;
            }

            LibraryInfo const& lib = m_allLibs.vec()[libHandle];
            StringHandle nameIndex = stringTable.indexForString(lib.name);
            GlobalLibIndex index{ m_usedLibs.size(), nameIndex };
            m_usedLibs.push_back(libHandle);
            m_usedLibMap.insert({ libHandle, index });
            return index;
        }

        LibraryInfo const& getLibInfo(GlobalLibIndex libIndex) const {
            LibraryHandle handle = m_usedLibs[libIndex.index];
            return m_allLibs.vec()[handle];
        }

    private:
        IndexSet<LibraryInfo, LibraryInfo::Hash> m_allLibs;
        std::unordered_map<LibraryHandle, SymbolTable> m_symbolTables;
        std::vector<LibraryHandle> m_usedLibs;
        std::unordered_map<LibraryHandle, GlobalLibIndex> m_usedLibMap;
    };
}

template <>
struct std::hash<fxprof::GlobalLibIndex> {
    size_t operator()(fxprof::GlobalLibIndex const& libIndex) const noexcept {
        auto h1 = std::hash<size_t>()(libIndex.index);
        auto h2 = std::hash<fxprof::StringHandle>()(libIndex.name);
        return h1 ^ (h2 << 1);
    }
};

template <>
struct matjson::Serialize<fxprof::GlobalLibIndex> {
    static Value toJson(fxprof::GlobalLibIndex const& libIndex) {
        return libIndex.index;
    }
};

template <>
struct matjson::Serialize<fxprof::GlobalLibTable> {
    static Value toJson(fxprof::GlobalLibTable const& value) {
        return value.getAllLibs();
    }
};