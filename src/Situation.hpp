#pragma once

#include <Geode/Geode.hpp>

namespace buf {
    enum class GameMode { Cube, Ship, Ufo, Ball, Wave, Robot, Spider, Swing };

    // Called once per physics tick: detects portals / gravity flips.
    void updateSituation(GJBaseGameLayer* layer);

    // How many ticks the next input of this player should be delayed.
    int delayTicksFor(GJBaseGameLayer* layer, bool isPlayer1);

    GameMode gameModeOf(PlayerObject* player);
    char const* gameModeName(GameMode mode);
}
