#pragma once
#include <cstdint>
#include <vector>

#include "gecko/Markers.hpp"
#include "gecko/Profile.hpp"

// Firefox Profiler JSON format
// https://github.com/firefox-devtools/profiler/blob/main/src/types/profile.ts
struct Profile {
    using Milliseconds = uint64_t;

    struct ProfileMeta {
        // The interval at which the threads are sampled.
        Milliseconds interval;
        // When the main process started. Timestamp expressed in milliseconds since
        // midnight January 1, 1970 GMT.
        Milliseconds startTime;
        // The number of milliseconds since midnight January 1, 1970 GMT.
        std::optional<Milliseconds> endTime;
        // The list of categories used in this profile. If present, it must contain at least the
        // "default category" which is defined as the first category whose color is "grey" - this
        // category usually has the name "Other".
        // If meta.categories is not present, a default list is substituted.
        struct Category {
            std::string name;
            std::string_view color;
            std::vector<std::string> subcategories;
        };
        std::optional<std::vector<Category>> categories;
        // The name of the product, most likely "Firefox".
        std::string product = "Geometry Dash";
        // Arguments to the program (currently only used for imported profiles)
        std::optional<std::string> arguments;
        // The amount of logically available CPU cores for the program.
        std::optional<uint32_t> logicalCPUs;
        // A boolean flag indicating whether we symbolicated this profile. If this is
        // false we'll start a symbolication process when the profile is loaded.
        // A missing property means that it's an older profile, it stands for an
        // "unknown" state. For now we don't do much with it but we may want to
        // propose a manual symbolication in the future.
        std::optional<bool> symbolicated;
        // A boolean flag indicating that symbolication is not supported
        // Used for imported profiles that cannot be symbolicated
        std::optional<bool> symbolicationNotSupported;
        // Profile importers can optionally add information about where they are imported from.
        // They also use the "product" field in the meta information, but this is somewhat
        // ambiguous. This field, if present, is unambiguous that it was imported.
        std::optional<std::string> importedFrom;
        // Do not distinguish between different stack types?
        std::optional<bool> usesOnlyOneStackType;
    } meta;

    struct Lib {

    };
    std::vector<Lib> libs;

    struct Counter {

    };
    std::vector<Counter> counters;

    struct Thread {

    };
    std::vector<Thread> threads;
};