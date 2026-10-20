#pragma once

#include <string>

namespace buf {
    // Snapshot of the mod settings. Read fresh on every click so changes in
    // the Geode settings menu apply immediately.
    struct Config {
        int waveMs, shipMs, ufoMs, cubeMs, ballMs, robotMs, spiderMs, swingMs;
        int flatMs;
        int transitionPercent, transitionWindowMs, maxMs;
        bool speedScaling;
    };

    Config loadConfig();

    std::string holdKeyName();

    bool hudEnabled();
    void setHudEnabled(bool value);
    int hudScalePercent();
    int hudOpacity();
}
