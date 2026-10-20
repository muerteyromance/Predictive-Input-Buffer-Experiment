#pragma once

#include <Geode/Geode.hpp>

namespace buf {
    enum class GameMode { Cube, Ship, Ufo, Ball, Wave, Robot, Spider, Swing };

    // Called on every update: detects portals / gravity flips.
    void updateSituation(GJBaseGameLayer* layer);

    // How many milliseconds the next input of this player should be delayed.
    float delayMsFor(GJBaseGameLayer* layer, bool isPlayer1);

    GameMode gameModeOf(PlayerObject* player);
    char const* gameModeName(GameMode mode);
}
