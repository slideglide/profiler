#pragma once
#include <cstdint>
#include <variant>

#include "GlobalLibTable.hpp"

namespace fxprof {
    struct FrameAddress {
        struct InstructionPointer {
            uintptr_t value;
        };
        struct ReturnAddress {
            uintptr_t value;
        };
        struct AdjustedReturnAddress {
            uintptr_t value;
        };
        struct RelativeAddressFromInstructionPointer {
            LibraryHandle library;
            uint32_t offset;
        };
        struct RelativeAddressFromReturnAddress {
            LibraryHandle library;
            uint32_t offset;
        };
        struct RelativeAddressFromAdjustedReturnAddress {
            LibraryHandle library;
            uint32_t offset;
        };

        constexpr FrameAddress() : address{ InstructionPointer{ 0 } } {}
        constexpr FrameAddress(InstructionPointer ip) : address{ ip } {}
        constexpr FrameAddress(ReturnAddress ra) : address{ ra } {}
        constexpr FrameAddress(AdjustedReturnAddress ara) : address{ ara } {}
        constexpr FrameAddress(RelativeAddressFromInstructionPointer rip) : address{ rip } {}
        constexpr FrameAddress(RelativeAddressFromReturnAddress rra) : address{ rra } {}
        constexpr FrameAddress(RelativeAddressFromAdjustedReturnAddress raa) : address{ raa } {}

        std::variant<
            InstructionPointer,
            ReturnAddress,
            AdjustedReturnAddress,
            RelativeAddressFromInstructionPointer,
            RelativeAddressFromReturnAddress,
            RelativeAddressFromAdjustedReturnAddress
        > address;
    };

    enum class FrameFlags : uint8_t {
        None = 0,
        IsJS = 1 << 0,
        IsRelevantForJs = 1 << 1,
    };

    constexpr FrameFlags operator|(FrameFlags a, FrameFlags b) {
        return static_cast<FrameFlags>(static_cast<uint8_t>(a) | static_cast<uint8_t>(b));
    }

    constexpr FrameFlags operator&(FrameFlags a, FrameFlags b) {
        return static_cast<FrameFlags>(static_cast<uint8_t>(a) & static_cast<uint8_t>(b));
    }
}
