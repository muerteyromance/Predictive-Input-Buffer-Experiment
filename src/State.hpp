#pragma once

#include <deque>

namespace buf {
    // F6 cycles through these multipliers. Level 0 = buffer fully off.
    inline constexpr float kLevels[] = {0.f, 0.5f, 1.f, 1.5f, 2.f};
    inline constexpr int kLevelCount = 5;

    struct PendingInput {
        bool down;
        int button;
        bool isPlayer1;
        int dueTick;
    };

    // What we remember about one player between ticks, to detect portals
    // and gravity flips.
    struct PlayerTrack {
        int gamemode = -1;
        bool upsideDown = false;
        int transitionUntilTick = 0;
    };

    struct State {
        int level = 2;            // index into kLevels (2 = 100%)
        bool situational = true;  // F7: per-gamemode logic on/off
        bool holdDisabled = false;  // true while the hold-to-disable key is held
        bool passthrough = false;   // true while we are replaying inputs ourselves

        // One queue per player so player 1 and player 2 never delay each other.
        // Index 0 = player 1, index 1 = player 2.
        std::deque<PendingInput> queues[2];
        int lastDueTick[2] = {0, 0};

        int tick = 0;
        float stepMs = 1000.f / 240.f;  // measured from the game each tick

        PlayerTrack players[2];
    };

    State& state();
    void resetRuntime();  // clears queues and per-level tracking
}
