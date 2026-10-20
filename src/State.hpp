#pragma once

#include <deque>
#include <string>

namespace buf {
    struct Preset {
        char const* name;
        int allTicks;   // ticks for every gamemode except wave
        int waveTicks;  // ticks for wave
        bool enabled;
    };

    // Same ladder as the original mod: 2/2, 4/4, 8/8, 16/8, off.
    inline constexpr Preset kPresets[] = {
        {"Suave", 2, 2, true},
        {"Medio", 4, 4, true},
        {"Alto", 8, 8, true},
        {"Fuerte", 16, 8, true},
        {"Off", 0, 0, false}
    };
    inline constexpr int kPresetCount = 5;

    // Any input waiting longer than this (real time) is released no matter what.
    inline constexpr double kFailsafeMs = 250.0;

    struct PendingInput {
        bool down;
        int button;
        bool isPlayer1;
        double dueMs;       // real time (ms since mod start) when it should land
        double enqueuedMs;  // real time when it was queued
    };

    // Settings, cached so hot paths do not touch the settings map.
    struct Cfg {
        int tps = 240;
        int releasePoint = 1;  // 0 = processCommands, 1 = PlayerObject::update
        int releasePct = 100;
        int maxMs = 70;
        int wavePct = 100, shipPct = 0, ufoPct = 100, cubePct = 100;
        int ballPct = 100, robotPct = 100, spiderPct = 100, swingPct = 0;
        bool debug = false;
        bool consoleEnabled = true;
        std::string cycleKey = "F7";
        std::string holdKey = "Shift";
    };

    struct Counters {
        int total = 0;      // inputs seen while the buffer was active
        int held = 0;       // delayed
        int delivered = 0;  // released on time
        int direct = 0;     // passed straight through
        int failsafe = 0;   // released by the safety net
        int callsPC = 0;    // processCommands calls
        int callsPCHalf = 0;
        int callsPU = 0;    // PlayerObject::update calls (player 1)
        float lastDtMs = 0.f;
    };

    struct State {
        int preset = 0;
        bool holdDisabled = false;
        bool passthrough = false;  // true while we replay inputs ourselves
        bool dirty = true;         // console needs a redraw
        double lastRenderMs = 0.0;
        double lastCfgMs = -1e9;

        std::deque<PendingInput> queues[2];  // [0] player 1, [1] player 2
        double lastDueMs[2] = {0.0, 0.0};

        std::deque<std::string> events;  // last events, shown in the console
        Cfg cfg;
        Counters c;

        double stepMs() const { return 1000.0 / (cfg.tps < 30 ? 30 : cfg.tps); }
    };

    State& state();
    double wallMs();                 // monotonic real time in ms since the mod started
    void refreshConfig(bool force);  // re-reads settings (throttled unless force)
    void pushEvent(std::string text);
    void resetRuntime();             // clears queues, keeps counters
}
