#pragma once

#include <Geode/Geode.hpp>

namespace buf {
    bool bufferActive();

    // Milliseconds the next input of this player should be delayed (0 = none).
    float delayMsFor(GJBaseGameLayer* layer, bool isPlayer1, bool down);

    // Called from the hooks. point: 0 = processCommands, 1 = PlayerObject::update.
    void tick(GJBaseGameLayer* layer, int point);

    // Applies everything still waiting, right now, in order.
    void flushAll(GJBaseGameLayer* layer);

    void cyclePreset();
    cocos2d::enumKeyCodes cycleKeyCode();
    char const* modeNameOf(PlayerObject* player);
}
