#pragma once
#include <string>
#include <string_view>

#include <matjson.hpp>

namespace fxprof {
    enum class CategoryColor {
        Transparent,
        LightBlue,
        Red,
        LightRed,
        Orange,
        Blue,
        Green,
        Purple,
        Yellow,
        Brown,
        Magenta,
        LightGreen,
        Gray,
        DarkGray,
    };

    inline std::string_view format_as(CategoryColor color) {
        switch (color) {
            case CategoryColor::Transparent: return "transparent";
            case CategoryColor::LightBlue:   return "lightblue";
            case CategoryColor::Red:         return "red";
            case CategoryColor::LightRed:    return "lightred";
            case CategoryColor::Orange:      return "orange";
            case CategoryColor::Blue:        return "blue";
            case CategoryColor::Green:       return "green";
            case CategoryColor::Purple:      return "purple";
            case CategoryColor::Yellow:      return "yellow";
            case CategoryColor::Brown:       return "brown";
            case CategoryColor::Magenta:     return "magenta";
            case CategoryColor::LightGreen:  return "lightgreen";
            case CategoryColor::Gray:        return "gray";
            case CategoryColor::DarkGray:    return "darkgray";
        }
        return "unknown";
    }
}

template <>
struct matjson::Serialize<fxprof::CategoryColor> {
    static geode::Result<fxprof::CategoryColor> fromJson(Value const& value) {
        GEODE_UNWRAP_INTO(std::string str, value.asString());
        if (str == "transparent") return geode::Ok(fxprof::CategoryColor::Transparent);
        if (str == "lightblue")   return geode::Ok(fxprof::CategoryColor::LightBlue);
        if (str == "red")         return geode::Ok(fxprof::CategoryColor::Red);
        if (str == "lightred")    return geode::Ok(fxprof::CategoryColor::LightRed);
        if (str == "orange")      return geode::Ok(fxprof::CategoryColor::Orange);
        if (str == "blue")        return geode::Ok(fxprof::CategoryColor::Blue);
        if (str == "green")       return geode::Ok(fxprof::CategoryColor::Green);
        if (str == "purple")      return geode::Ok(fxprof::CategoryColor::Purple);
        if (str == "yellow")      return geode::Ok(fxprof::CategoryColor::Yellow);
        if (str == "brown")       return geode::Ok(fxprof::CategoryColor::Brown);
        if (str == "magenta")     return geode::Ok(fxprof::CategoryColor::Magenta);
        if (str == "lightgreen")  return geode::Ok(fxprof::CategoryColor::LightGreen);
        if (str == "gray")        return geode::Ok(fxprof::CategoryColor::Gray);
        if (str == "darkgray")    return geode::Ok(fxprof::CategoryColor::DarkGray);
        return geode::Err("Invalid CategoryColor string: " + str);
    }

    static Value toJson(fxprof::CategoryColor const& value) {
        return std::string(format_as(value));
    }
};