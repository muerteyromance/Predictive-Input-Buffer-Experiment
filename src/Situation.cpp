#include "Situation.hpp"

#include "Config.hpp"
#include "State.hpp"

#include <algorithm>

using namespace geode::prelude;

namespace buf {
    GameMode gameModeOf(PlayerObject* p) {
        if (p->m_isDart) return GameMode::Wave;
        if (p->m_isShip) return GameMode::Ship;
        if (p->m_isBird) return GameMode::Ufo;
        if (p->m_isBall) return GameMode::Ball;
        if (p->m_isRobot) return GameMode::Robot;
        if (p->m_isSpider) return GameMode::Spider;
        if (p->m_isSwing) return GameMode::Swing;
        return GameMode::Cube;
    }

    char const* gameModeName(GameMode m) {
        switch (m) {
            case GameMode::Cube: return "cube";
            case GameMode::Ship: return "ship";
            case GameMode::Ufo: return "ufo";
            case GameMode::Ball: return "ball";
            case GameMode::Wave: return "wave";
            case GameMode::Robot: return "robot";
            case GameMode::Spider: return "spider";
            case GameMode::Swing: return "swing";
        }
        return "?";
    }

    static int modeMs(GameMode m, Config const& c) {
        switch (m) {
            case GameMode::Cube: return c.cubeMs;
            case GameMode::Ship: return c.shipMs;
            case GameMode::Ufo: return c.ufoMs;
            case GameMode::Ball: return c.ballMs;
            case GameMode::Wave: return c.waveMs;
            case GameMode::Robot: return c.robotMs;
            case GameMode::Spider: return c.spiderMs;
            case GameMode::Swing: return c.swingMs;
        }
        return c.flatMs;
    }

    static PlayerObject* playerFor(GJBaseGameLayer* layer, bool isPlayer1) {
        return isPlayer1 ? layer->m_player1 : layer->m_player2;
    }

    void updateSituation(GJBaseGameLayer* layer) {
        auto& st = state();
        auto cfg = loadConfig();

        for (int i = 0; i < 2; i++) {
            auto p = playerFor(layer, i == 0);
            if (!p) continue;

            auto& track = st.players[i];
            int gm = static_cast<int>(gameModeOf(p));
            bool up = p->m_isUpsideDown;

            // Ignore the very first reading after a reset so the level start
            // does not count as a transition.
            if (track.gamemode != -1 && (gm != track.gamemode || up != track.upsideDown)) {
                track.transitionUntilMs = wallMs() + cfg.transitionWindowMs;
            }
            track.gamemode = gm;
            track.upsideDown = up;
        }
    }

    float delayMsFor(GJBaseGameLayer* layer, bool isPlayer1) {
        auto& st = state();
        if (st.level <= 0) return 0.f;

        auto cfg = loadConfig();
        auto p = playerFor(layer, isPlayer1);

        float ms = static_cast<float>(cfg.flatMs);

        if (st.situational && p) {
            ms = static_cast<float>(modeMs(gameModeOf(p), cfg));

            if (ms > 0.f && cfg.speedScaling && p->m_playerSpeed > 0.f) {
                // 0.9 is the game's normal speed. Faster -> smaller delay.
                ms *= std::clamp(0.9f / p->m_playerSpeed, 0.5f, 1.3f);
            }

            auto const& track = st.players[isPlayer1 ? 0 : 1];
            if (ms > 0.f && wallMs() < track.transitionUntilMs) {
                ms *= cfg.transitionPercent / 100.f;
            }
        }

        ms *= kLevels[st.level];
        ms = std::min(ms, static_cast<float>(cfg.maxMs));
        return std::max(ms, 0.f);
    }
}
