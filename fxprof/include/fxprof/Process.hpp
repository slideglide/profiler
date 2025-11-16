#pragma once
#include <cstddef>
#include <optional>
#include <string>
#include <vector>

#include "FrameTable.hpp"
#include "GlobalLibTable.hpp"
#include "LibMappings.hpp"
#include "Timestamp.hpp"

namespace fxprof {
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

        void addLibMapping(LibraryHandle lib, uint64_t startAVMA, uint64_t endAVMA, uint32_t relativeAddressAtStart) {
            m_libs.addMapping(startAVMA, endAVMA, relativeAddressAtStart, lib);
        }

        InternalFrameAddress convertAddress(
            GlobalLibTable& globalLibs,
            LibMappings<LibraryHandle> const& kernelLibs,
            ProfileStringTable& stringTable,
            uint64_t address
        ) const {
            auto addr = kernelLibs.convertAddress(address);
            if (!addr.has_value()) {
                addr = m_libs.convertAddress(address);
            }

            if (!addr.has_value()) {
                return InternalFrameAddress{.address = address, .type = InternalFrameAddress::Type::Unknown};
            }

            auto& libHandle = addr->second;
            auto libIndex = globalLibs.indexForUsedLib(libHandle, stringTable);
            return InternalFrameAddress{
                .library = libIndex,
                .offset = addr->first,
                .type = InternalFrameAddress::Type::InLib
            };
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