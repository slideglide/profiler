#pragma once
#include <cstddef>
#include <optional>
#include <string>
#include <vector>

#include "GlobalLibTable.hpp"
#include "LibMappings.hpp"
#include "Timestamp.hpp"

namespace fxprof {
    using ThreadHandle = size_t;

    class Process {
    public:
        Process(
            std::string name,
            std::string pid,
            Timestamp startTime
        ) : m_pid(std::move(pid)), m_name(std::move(name)), m_startTime(startTime) {}

        std::string const& getPid() const {
            return m_pid;
        }

        std::string const& getName() const {
            return m_name;
        }

        std::vector<ThreadHandle> const& getThreads() const {
            return m_threads;
        }

        std::optional<ThreadHandle> getMainThread() const {
            return m_mainThread;
        }

        Timestamp getStartTime() const {
            return m_startTime;
        }

        std::optional<Timestamp> getEndTime() const {
            return m_endTime;
        }

        LibMappings<LibraryHandle> const& getLibs() const {
            return m_libs;
        }

        void addThread(ThreadHandle threadHandle, bool isMain = false) {
            m_threads.push_back(threadHandle);
            if (isMain) {
                m_mainThread = threadHandle;
            }
        }

    private:
        std::string m_pid;
        std::string m_name;
        std::vector<ThreadHandle> m_threads;
        std::optional<ThreadHandle> m_mainThread;
        Timestamp m_startTime;
        std::optional<Timestamp> m_endTime;
        LibMappings<LibraryHandle> m_libs;
    };
}