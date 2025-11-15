#pragma once
#include <cstdint>

namespace fxprof {
    using FuncIndex = uint32_t;

    class FuncTable {};
};

template <>
struct matjson::Serialize<fxprof::FuncTable> {
    static Value toJson(fxprof::FuncTable const& value) {
        return makeObject({
            {"length", 0},
            {"name", std::vector<Value>()},
            {"isJS", std::vector<Value>()},
            {"relevantForJS", std::vector<Value>()},
            {"resource", std::vector<Value>()},
            {"fileName", std::vector<Value>()},
            {"lineNumber", std::vector<Value>()},
            {"columnNumber", std::vector<Value>()},
        });
    }
};