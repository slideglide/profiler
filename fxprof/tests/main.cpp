#include <array>
#include <fstream>
#include <fxprof/Frame.hpp>
#include <fxprof/Profile.hpp>

using namespace fxprof;

int main() {
    auto profile = Profile(
        "Geometry Dash",
        ReferenceTimestamp(1636162232627),
        SamplingInterval::fromMillis(1)
    );

    profile.setOSName("Windows");
    auto process = profile.addProcess("GeometryDash.exe", 123, Timestamp());
    auto thread = profile.addThread(process, 12345, Timestamp(), true);
    profile.setThreadName(thread, "Main Thread");

    profile.addSample(
        thread,
        Timestamp(),
        std::nullopt,
        ZERO_DELTA,
        1
    );

    constexpr std::array frames1 = {
        0x7f76b7ffc0e7,
        0x55ba9eda3d7f,
        0x55ba9ed8bb62,
        0x55ba9ec92419,
        0x55ba9ec2b778,
        0x55ba9ec0f705,
        0x7ffdb4824838,
    };

    std::vector<FrameAddress> frameAddresses1;
    for (int i = frames1.size() - 1; i >= 0; --i) {
        uintptr_t addr = frames1[i];
        if (i == 0) {
            frameAddresses1.push_back(FrameAddress::InstructionPointer{ addr });
        } else {
            frameAddresses1.push_back(FrameAddress::ReturnAddress{ addr });
        }
    }

    auto s1 = profile.handleForStackFrames(
        thread,
        [&](Profile& p, size_t i) -> std::optional<FrameHandle> {
            if (i >= frameAddresses1.size()) return std::nullopt;
            return p.handleForFrameWithAddress(
                thread,
                frameAddresses1[i],
                Category{ "Other", CategoryColor::Grey },
                FrameFlags::None
            );
        }
    );

    profile.addSample(
        thread,
        Timestamp::fromMillis(1.0),
        s1,
        ZERO_DELTA,
        1
    );

    constexpr std::array frames2 = {
        0x55ba9eda018e,
        0x55ba9ec3c3cf,
        0x55ba9ec2a2d7,
        0x55ba9ec53993,
        0x7f76b7e8707d,
        0x55ba9ec0f705,
        0x7ffdb4824838,
    };

    std::vector<FrameAddress> frameAddresses2;
    for (int i = frames2.size() - 1; i >= 0; --i) {
        uintptr_t addr = frames2[i];
        if (i == 0) {
            frameAddresses2.push_back(FrameAddress::InstructionPointer{ addr });
        } else {
            frameAddresses2.push_back(FrameAddress::ReturnAddress{ addr });
        }
    }

    auto s2 = profile.handleForStackFrames(
        thread,
        [&](Profile& p, size_t i) -> std::optional<FrameHandle> {
            if (i >= frameAddresses2.size()) return std::nullopt;
            return p.handleForFrameWithAddress(
                thread,
                frameAddresses2[i],
                Category{ "Other", CategoryColor::Grey },
                FrameFlags::None
            );
        }
    );

    profile.addSample(
        thread,
        Timestamp::fromMillis(2.0),
        s2,
        ZERO_DELTA,
        1
    );

    constexpr std::array frames3 = {
        0x7f76b7f019c6,
        0x55ba9edc48f5,
        0x55ba9ec010e3,
        0x55ba9eca41b9,
        0x7f76b7e8707d,
        0x55ba9ec0f705,
        0x7ffdb4824838,
    };

    std::vector<FrameAddress> frameAddresses3;
    for (int i = frames3.size() - 1; i >= 0; --i) {
        uintptr_t addr = frames3[i];
        if (i == 0) {
            frameAddresses3.push_back(FrameAddress::InstructionPointer{ addr });
        } else {
            frameAddresses3.push_back(FrameAddress::ReturnAddress{ addr });
        }
    }

    auto s3 = profile.handleForStackFrames(
        thread,
        [&](Profile& p, size_t i) -> std::optional<FrameHandle> {
            if (i >= frameAddresses3.size()) return std::nullopt;
            return p.handleForFrameWithAddress(
                thread,
                frameAddresses3[i],
                Category{ "Other", CategoryColor::Grey },
                FrameFlags::None
            );
        }
    );

    profile.addSample(
        thread,
        Timestamp::fromMillis(3.0),
        s3,
        ZERO_DELTA,
        1
    );

    // save to json
    std::ofstream("profile.json") << matjson::Value(profile).dump();
}