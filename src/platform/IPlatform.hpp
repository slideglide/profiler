#pragma once

constexpr size_t MAX_STACK_FRAMES = 64;

namespace platform {
    void init();

    struct StackSample {
        std::array<uintptr_t, MAX_STACK_FRAMES> frames;
        size_t frameCount;
    };

    struct ModuleInfo {
        std::string name;
        std::string path;
        std::string debugName;
        std::string debugPath;
        std::string breakpadId;
        std::optional<std::string> codeId;
        std::string arch;
        uintptr_t baseAddress;
        size_t size;
        uint32_t relativeAddressAtStart;
    };

    struct FunctionSymbol {
        std::string name;
        uint32_t address;
        std::optional<uint32_t> size;
    };

    std::vector<FunctionSymbol> getModuleSymbols(uintptr_t baseAddress, std::vector<uintptr_t> const& addresses);

    void captureThread(
        std::atomic_flag& isRunning,
        std::vector<StackSample>& samples
    );

    void enumerateModules(
        std::function<void(ModuleInfo const&)> const& callback
    );
}

template<>
struct matjson::Serialize<platform::StackSample> {
    static geode::Result<platform::StackSample> fromJson(Value const& value) {
        platform::StackSample sample;
        if (!value.isArray()) {
            return geode::Err("Expected array for StackSample");
        }
        size_t count = value.size();
        if (count > MAX_STACK_FRAMES) {
            return geode::Err("StackSample array size exceeds MAX_STACK_FRAMES");
        }
        sample.frameCount = count;
        for (size_t i = 0; i < count; ++i) {
            auto frameValue = value[i];
            if (!frameValue.isNumber()) {
                return geode::Err("Expected number for StackSample frame");
            }
            sample.frames[i] = frameValue.asUInt().unwrapOrDefault();
        }
        return geode::Ok(sample);
    }

    static Value toJson(platform::StackSample const& value) {
        auto arr = Value::array();
        for (size_t i = 0; i < value.frameCount; ++i) {
            arr.push(value.frames[i]);
        }
        return arr;
    }
};