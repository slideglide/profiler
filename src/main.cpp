#include <Geode/Geode.hpp>
#include "Profiler.hpp"
#include <fxprof/Profile.hpp>

using namespace geode::prelude;

$on_mod(Loaded) {
    listenForKeybindSettingPresses("toggle-profiler", [](Keybind const& keybind, bool down, bool repeat, double timestamp) {
        if (down && !repeat) {
            auto& profiler = Profiler::get();
            if (profiler.isRunning()) {
                profiler.stop();
                log::info("Profiler stopped.");
            }
            else {
                profiler.start();
                log::info("Profiler started.");
            }
        }
    });
};