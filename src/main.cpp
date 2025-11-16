#include <Geode/Geode.hpp>
#include <Geode/modify/MenuLayer.hpp>

#include <geode.custom-keybinds/include/OptionalAPI.hpp>

#include "Profiler.hpp"
#include "platform/IPlatform.hpp"

#include <fxprof/Profile.hpp>

using namespace geode::prelude;

$on_mod(Loaded) {
    // platform::init();

    using namespace keybinds;

    (void)[]() -> Result<> {
        GEODE_UNWRAP(BindManagerV2::registerBindable(GEODE_UNWRAP(BindableActionV2::create(
            "toggle_profiler"_spr,
            "Toggle Profiler",
            "Starts or stops the profiler.",
            { GEODE_UNWRAP(KeybindV2::create(KEY_F6, ModifierV2::Control)) },
            GEODE_UNWRAP(CategoryV2::create("Profiler")),
            false
        ))));
        return Ok();
    }();

    new EventListener(+[](InvokeBindEventV2* event) {
        if (!event->isDown()) {
            auto& profiler = Profiler::get();
            if (profiler.isRunning()) {
                profiler.stop();
                log::info("Profiler stopped.");
            } else {
                profiler.start();
                log::info("Profiler started.");
            }
        }
        return ListenerResult::Propagate;
    }, InvokeBindFilterV2(nullptr, "toggle_profiler"_spr));
}