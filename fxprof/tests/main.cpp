#include <fstream>
#include <fxprof/Profile.hpp>

int main() {
    auto profile = fxprof::Profile("Geometry Dash", fxprof::ReferenceTimestamp::now(), fxprof::SamplingInterval::fromMillis(10));
    profile.setOSName("Windows");
    auto process = profile.addProcess("GeometryDash.exe", 1234, fxprof::Timestamp());
    auto thread = profile.addThread(process, 123, fxprof::Timestamp(), true);
    profile.setThreadName(thread, "Main Thread");

    profile.addSample(
        thread,
        fxprof::Timestamp(),
        std::nullopt,
        fxprof::ZERO_DELTA,
        1
    );

    // save to json
    std::ofstream("profile.json") << matjson::Value(profile).dump();
}