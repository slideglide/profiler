#include "IPlatform.hpp"

#include <DbgHelp.h>
#include <Windows.h>
#include <Psapi.h>
#include <mutex>
#pragma comment(lib, "dbghelp.lib")

PVOID GeodeFunctionTableAccess64(HANDLE hProcess, DWORD64 AddrBase);

using namespace geode::prelude;

namespace platform {
    static DWORD MainThreadID = (queueInMainThread([] {
        MainThreadID = GetCurrentThreadId();
    }), -1);

    namespace {
        std::once_flag gSymInitFlag;

        DWORD64 MyGetModuleBase64(HANDLE /*hProcess*/, DWORD64 address) {
            DWORD64 imageBase = 0;
            if (RtlLookupFunctionEntry(address, &imageBase, nullptr)) {
                return imageBase;
            }
            HMODULE mod = nullptr;
            if (GetModuleHandleExA(
                    GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                    reinterpret_cast<LPCSTR>(address),
                    &mod)) {
                return reinterpret_cast<DWORD64>(mod);
                    }
            return 0;
        }

        PVOID __stdcall FunctionTableAccess(HANDLE process, DWORD64 addrBase) {
            if (auto ret = GeodeFunctionTableAccess64(process, addrBase)) {
                return ret;
            }

            if (auto ret = SymFunctionTableAccess64(process, addrBase)) {
                return ret;
            }

            DWORD64 imageBase = 0;
            return RtlLookupFunctionEntry(addrBase, &imageBase, nullptr);
        }

        DWORD64 __stdcall ModuleBase(HANDLE process, DWORD64 address) {
            if (GeodeFunctionTableAccess64(process, address)) {
                return address & (~0xffffull);
            }

            if (auto base = SymGetModuleBase64(process, address)) {
                return base;
            }

            return MyGetModuleBase64(process, address);
        }
    }

    void init() {
        std::call_once(gSymInitFlag, [] {
            SymSetOptions(SYMOPT_UNDNAME | SYMOPT_DEFERRED_LOADS | SYMOPT_LOAD_LINES);
            if (!SymInitialize(GetCurrentProcess(), nullptr, true)) {
                log::warn("Failed to initialize DbgHelp symbols ({}). Stack walking may be incomplete.", GetLastError());
            }
        });
    }

    void captureThread(
        std::atomic_flag& isRunning,
        std::vector<StackSample>& samples
    ) {
        init();

        HANDLE hThread = OpenThread(
            THREAD_SUSPEND_RESUME | THREAD_GET_CONTEXT,
            FALSE,
            MainThreadID
        );

        if (hThread == nullptr) {
            return;
        }

        while (isRunning.test_and_set()) {
            if (SuspendThread(hThread) == static_cast<DWORD>(-1)) {
                Sleep(1);
                continue;
            }

            CONTEXT ctx = {};
            ctx.ContextFlags = CONTEXT_FULL;

            if (GetThreadContext(hThread, &ctx)) {
                StackSample sample = {};

                sample.timestamp = std::chrono::steady_clock::now();

                STACKFRAME64 frame = {};
                frame.AddrPC.Offset = ctx.Rip;
                frame.AddrPC.Mode = AddrModeFlat;
                frame.AddrFrame.Offset = ctx.Rbp;
                frame.AddrFrame.Mode = AddrModeFlat;
                frame.AddrStack.Offset = ctx.Rsp;
                frame.AddrStack.Mode = AddrModeFlat;

                HANDLE hProcess = GetCurrentProcess();
                size_t count = 0;

                auto outBuf = reinterpret_cast<void**>(sample.frames.data());

                while (count < MAX_STACK_FRAMES) {
                    BOOL ok = StackWalk64(
                        IMAGE_FILE_MACHINE_AMD64,
                        hProcess,
                        hThread,
                        &frame,
                        &ctx,
                        nullptr,
                        FunctionTableAccess,
                        ModuleBase,
                        nullptr
                    );

                    if (!ok || frame.AddrPC.Offset == 0) {
                        break;
                    }

                    outBuf[count] = reinterpret_cast<void*>(frame.AddrPC.Offset);
                    ++count;
                }

                sample.frameCount = count;
                samples.push_back(sample);
            }

            ResumeThread(hThread);
            Sleep(1);
        }
    }

    void enumerateModules(std::function<void(ModuleInfo const&)> const& callback) {
        HANDLE hProcess = GetCurrentProcess();
        HMODULE hMods[1024];
        DWORD cbNeeded;

        if (EnumProcessModules(hProcess, hMods, sizeof(hMods), &cbNeeded)) {
            size_t moduleCount = cbNeeded / sizeof(HMODULE);
            for (size_t i = 0; i < moduleCount; ++i) {
                char modName[MAX_PATH];
                char modPath[MAX_PATH];
                char debugName[MAX_PATH];
                char debugPath[MAX_PATH];
                MODULEINFO modInfo;

                if (GetModuleFileNameExA(hProcess, hMods[i], modPath, sizeof(modPath)) == 0) {
                    continue;
                }

                if (GetModuleBaseNameA(hProcess, hMods[i], modName, sizeof(modName)) == 0) {
                    continue;
                }

                if (GetModuleInformation(hProcess, hMods[i], &modInfo, sizeof(modInfo)) == 0) {
                    continue;
                }

                // For simplicity, we use the same name/path for debug info
                strcpy_s(debugName, modName);
                strcpy_s(debugPath, modPath);

                ModuleInfo info;
                info.name = modName;
                info.path = modPath;
                info.debugName = debugName;
                info.debugPath = debugPath;
                info.breakpadId = ""; // Breakpad ID generation is omitted for brevity
                info.codeId = std::nullopt;
                info.arch = "x86_64";
                info.baseAddress = reinterpret_cast<uintptr_t>(modInfo.lpBaseOfDll);
                info.size = static_cast<size_t>(modInfo.SizeOfImage);
                info.relativeAddressAtStart = 0; // Omitted for brevity

                callback(info);
            }
        }
    }

    std::vector<FunctionSymbol> getModuleSymbols(uintptr_t baseAddress, std::vector<uintptr_t> const& addresses) {
        init();

        std::vector<FunctionSymbol> symbols;
        HANDLE hProcess = GetCurrentProcess();

        char buffer[sizeof(SYMBOL_INFO) + MAX_SYM_NAME * sizeof(TCHAR)];
        PSYMBOL_INFO symbolInfo = reinterpret_cast<PSYMBOL_INFO>(buffer);

        for (auto addr : addresses) {
            symbolInfo->SizeOfStruct = sizeof(SYMBOL_INFO);
            symbolInfo->MaxNameLen = MAX_SYM_NAME;

            DWORD64 displacement;
            if (SymFromAddr(hProcess, addr, &displacement, symbolInfo)) {
                // Only include if it belongs to this module
                if (symbolInfo->ModBase == baseAddress) {
                    FunctionSymbol sym;
                    sym.name = std::string(symbolInfo->Name, symbolInfo->NameLen);
                    sym.address = static_cast<uint32_t>(symbolInfo->Address - symbolInfo->ModBase);
                    sym.size = symbolInfo->Size > 0 ? std::optional<uint32_t>(symbolInfo->Size) : std::nullopt;
                    symbols.push_back(std::move(sym));
                }
            }
        }

        return symbols;
    }

    std::optional<uintptr_t> moduleBaseFromAddress(uintptr_t address) {
        init();
        HANDLE hProcess = GetCurrentProcess();
        auto base = ModuleBase(hProcess, address);
        if (base == 0) {
            return std::nullopt;
        }
        return base;
    }

    std::optional<ModuleInfo> moduleInfoFromBase(uintptr_t baseAddress) {
        HANDLE hProcess = GetCurrentProcess();
        auto module = reinterpret_cast<HMODULE>(baseAddress);
        if (module == nullptr) {
            return std::nullopt;
        }

        MODULEINFO modInfo;
        if (GetModuleInformation(hProcess, module, &modInfo, sizeof(modInfo)) == 0) {
            return std::nullopt;
        }

        char modName[MAX_PATH];
        char modPath[MAX_PATH];
        char debugName[MAX_PATH];
        char debugPath[MAX_PATH];

        if (GetModuleFileNameExA(hProcess, module, modPath, sizeof(modPath)) == 0) {
            return std::nullopt;
        }

        if (GetModuleBaseNameA(hProcess, module, modName, sizeof(modName)) == 0) {
            return std::nullopt;
        }

        strcpy_s(debugName, modName);
        strcpy_s(debugPath, modPath);

        ModuleInfo info;
        info.name = modName;
        info.path = modPath;
        info.debugName = debugName;
        info.debugPath = debugPath;
        info.breakpadId = "";
        info.codeId = std::nullopt;
        info.arch = "x86_64";
        info.baseAddress = baseAddress;
        info.size = static_cast<size_t>(modInfo.SizeOfImage);
        info.relativeAddressAtStart = 0;

        return info;
    }
}
