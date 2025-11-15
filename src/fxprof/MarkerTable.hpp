#pragma once

namespace fxprof {
    class MarkerTable {
    };
};

template <>
struct matjson::Serialize<fxprof::MarkerTable> {
    static Value toJson(fxprof::MarkerTable const& value) {
        return makeObject({
            {"length", 0},
            {"category", std::vector<Value>()},
            {"data", std::vector<Value>()},
            {"startTime", std::vector<Value>()},
            {"endTime", std::vector<Value>()},
            {"name", std::vector<Value>()},
            {"phase", std::vector<Value>()},
        });
    }
};