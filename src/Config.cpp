#include "Config.hpp"

#include <Geode/Geode.hpp>

using namespace geode::prelude;

namespace buf {
    static int getInt(char const* key) {
        return static_cast<int>(Mod::get()->getSettingValue<int64_t>(key));
    }

    Config loadConfig() {
        Config c{};
        c.waveMs = getInt("wave-ms");
        c.shipMs = getInt("ship-ms");
        c.ufoMs = getInt("ufo-ms");
        c.cubeMs = getInt("cube-ms");
        c.ballMs = getInt("ball-ms");
        c.robotMs = getInt("robot-ms");
        c.spiderMs = getInt("spider-ms");
        c.swingMs = getInt("swing-ms");
        c.flatMs = getInt("flat-ms");
        c.transitionPercent = getInt("transition-percent");
        c.transitionWindowMs = getInt("transition-window-ms");
        c.maxMs = getInt("max-ms");
        c.speedScaling = Mod::get()->getSettingValue<bool>("speed-scaling");
        return c;
    }

    std::string holdKeyName() {
        return Mod::get()->getSettingValue<std::string>("hold-disable-key");
    }

    bool hudEnabled() {
        return Mod::get()->getSettingValue<bool>("show-hud");
    }

    void setHudEnabled(bool value) {
        Mod::get()->setSettingValue<bool>("show-hud", value);
    }

    int hudScalePercent() { return getInt("hud-scale"); }
    int hudOpacity() { return getInt("hud-opacity"); }
}
