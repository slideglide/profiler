#pragma once

constexpr size_t MAX_STACK_FRAMES = 64;

namespace platform {
    void init();

    struct StackSample {
        std::chrono::steady_clock::time_point timestamp;
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

    std::optional<uintptr_t> moduleBaseFromAddress(uintptr_t address);
    std::optional<ModuleInfo> moduleInfoFromBase(uintptr_t baseAddress);

    std::vector<FunctionSymbol> getModuleSymbols(uintptr_t baseAddress, std::vector<uintptr_t> const& addresses);

    void captureThread(
        std::atomic_flag& isRunning,
        std::vector<StackSample>& samples
    );

    void enumerateModules(
        std::function<void(ModuleInfo const&)> const& callback
    );
}