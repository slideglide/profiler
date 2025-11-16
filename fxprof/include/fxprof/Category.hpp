#pragma once
#include <string>
#include <unordered_set>
#include <matjson.hpp>
#include <matjson/std.hpp>

#include "CategoryColor.hpp"
#include "IndexSet.hpp"

namespace fxprof {
    using CategoryHandle = uint16_t;
    using SubcategoryIndex = uint16_t;

    struct SubcategoryHandle {
        CategoryHandle category;
        SubcategoryIndex subcategory;
        bool operator==(SubcategoryHandle const& subcategory_handle) const = default;
    };

    struct Category {
        std::string_view name;
        CategoryColor color = CategoryColor::Grey;
    };

    constexpr Category OTHER_CATEGORY{"Other", CategoryColor::Grey};

    struct Subcategory {
        Category category;
        std::string_view name;
    };

    class InternalCategory {
    public:
        InternalCategory(std::string name) : m_name(std::move(name)) {}
        InternalCategory(std::string name, CategoryColor color)
            : m_name(std::move(name)), m_color(color) {}

        std::string const& name() const {
            return m_name;
        }

        CategoryColor color() const {
            return m_color;
        }

        IndexSet<std::string> const& subcategories() const {
            return m_subcategories;
        }

        bool operator==(InternalCategory const& other) const {
            return m_name == other.m_name;
        }

    private:
        std::string m_name;
        CategoryColor m_color = CategoryColor::Grey;
        IndexSet<std::string> m_subcategories;
    };
}

template <>
struct std::hash<fxprof::InternalCategory> {
    size_t operator()(fxprof::InternalCategory const& category) const noexcept {
        return std::hash<std::string>()(category.name());
    }
};

template <>
struct matjson::Serialize<fxprof::InternalCategory> {
    static Value toJson(fxprof::InternalCategory const& value) {
        return makeObject({
            {"name", value.name()},
            {"color", value.color()},
            {"subcategories", value.subcategories()},
        });
    }
};
