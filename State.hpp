#pragma once

#include <deque>

namespace buf {
    // F6 cycles through these multipliers. Level 0 = buffer fully off.
    inline constexpr float kLevels[] = {0.f, 0.5f, 1.f, 1.5f, 2.f};
    inline constexpr int kLevelCount = 5;

    // Safety net: any input waiting longer than this (real time) is released
    // no matter what, so a click can never get stuck in the queue.
    inline constexpr double kFailsafeMs = 250.0;

    struct PendingInput {
        bool down;
        int button;
        bool isPlayer1;
        double dueMs;           // real time (ms since mod start) when it should be applied
        double enqueuedWallMs;  // real time when it was queued
    };

    // What we remember about one player between updates, to detect portals
    // and gravity flips.
    struct PlayerTrack {
        int gamemode = -1;
        bool upsideDown = false;
        double transitionUntilMs = 0.0;
    };

    // Counters shown in the on-screen debug panel.
    struct Debug {
        int calls = 0;       // processCommands calls
        int halfCalls = 0;   // of those, how many had isHalfTick = true
        int in = 0;          // clicks/releases intercepted while the buffer was active
        int queued = 0;      // of those, how many were actually delayed
        int out = 0;         // released from the queue on time
        int stale = 0;       // released by the failsafe
        float lastDtMs = 0.f;
        float lastDelayMs = 0.f;
    };

    struct State {
        int level = 2;              // index into kLevels (2 = 100%)
        bool situational = true;    // F7: per-gamemode logic on/off
        bool holdDisabled = false;  // true while the buffer is paused by the hold key
        bool holdKeyDown = false;   // raw state of Tab / F9 (event based)
        bool passthrough = false;   // true while we are replaying inputs ourselves

        // One queue per player so player 1 and player 2 never delay each other.
        // Index 0 = player 1, index 1 = player 2.
        std::deque<PendingInput> queues[2];
        double lastDueMs[2] = {0.0, 0.0};

        PlayerTrack players[2];
        Debug dbg;
    };

    State& state();
    double wallMs();      // monotonic real time in ms since the mod started
    void resetRuntime();  // clears queues and per-level tracking
}
