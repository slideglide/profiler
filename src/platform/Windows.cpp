#include "IPlatform.hpp"

#include <Windows.h>
#include <TlHelp32.h>
#include <DbgHelp.h>
#include <Psapi.h>
#pragma comment(lib, "dbghelp.lib")

using namespace geode::prelude;

namespace platform {
    static DWORD MainThreadID = (queueInMainThread([] {
        MainThreadID = GetCurrentThreadId();
    }), -1);

    void init() {

    }

    void captureThread(
        std::atomic_flag& isRunning,
        std::vector<StackSample>& samples
    ) {
        HANDLE hThread = OpenThread(
            THREAD_SUSPEND_RESUME | THREAD_GET_CONTEXT,
            FALSE,
            MainThreadID
        );

        if (hThread == nullptr) {
            return;
        }

        CONTEXT context = {};
        context.ContextFlags = CONTEXT_FULL;

        while (isRunning.test_and_set()) {
            SuspendThread(hThread);
            if (GetThreadContext(hThread, &context)) {
                StackSample sample = {};
                RtlCaptureStackBackTrace(
                    0,
                    MAX_STACK_FRAMES,
                    reinterpret_cast<void**>(sample.frames),
                    nullptr
                );
                sample.frameCount = MAX_STACK_FRAMES;
                samples.push_back(sample);
            }
            ResumeThread(hThread);
            Sleep(10); // Adjust the sleep duration as needed
        }
    }
}
