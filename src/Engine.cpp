#include "Engine.hpp"

#include "Console.hpp"
#include "State.hpp"

#include <algorithm>
#include <cmath>

using namespace geode::prelude;

namespace buf {
    bool bufferActive() {
        auto const& st = state();
        return kPresets[st.preset].enabled && !st.holdDisabled;
    }

    static PlayerObject* playerFor(GJBaseGameLayer* layer, bool isPlayer1) {
        return isPlayer1 ? layer->m_player1 : layer->m_player2;
    }

    char const* modeNameOf(PlayerObject* p) {
        if (!p) return "?";
        if (p->m_isDart) return "wave";
        if (p->m_isShip) return "ship";
        if (p->m_isBird) return "ufo";
        if (p->m_isBall) return "ball";
        if (p->m_isRobot) return "robot";
        if (p->m_isSpider) return "spider";
        if (p->m_isSwing) return "swing";
        return "cube";
    }

    static int percentFor(PlayerObject* p, Cfg const& c) {
        if (!p) return c.cubePct;
        if (p->m_isDart) return c.wavePct;
        if (p->m_isShip) return c.shipPct;
        if (p->m_isBird) return c.ufoPct;
        if (p->m_isBall) return c.ballPct;
        if (p->m_isRobot) return c.robotPct;
        if (p->m_isSpider) return c.spiderPct;
        if (p->m_isSwing) return c.swingPct;
        return c.cubePct;
    }

    float delayMsFor(GJBaseGameLayer* layer, bool isPlayer1, bool down) {
        auto& st = state();
        auto const& pr = kPresets[st.preset];
        if (!pr.enabled) return 0.f;

        auto p = playerFor(layer, isPlayer1);
        bool wave = p && p->m_isDart;
        int baseTicks = wave ? pr.waveTicks : pr.allTicks;

        double ms = baseTicks * st.stepMs() * percentFor(p, st.cfg) / 100.0;
        if (!down) ms *= st.cfg.releasePct / 100.0;
        ms = std::min(ms, static_cast<double>(st.cfg.maxMs));
        return static_cast<float>(std::max(ms, 0.0));
    }

    // Releases what is due. With onlyStale, only the safety net acts.
    static void releaseDue(GJBaseGameLayer* layer, bool onlyStale) {
        auto& st = state();
        double now = wallMs();
        bool any = false;

        st.passthrough = true;
        for (int i = 0; i < 2; i++) {
            auto& q = st.queues[i];
            while (!q.empty()) {
                auto const& front = q.front();
                bool due = !onlyStale && front.dueMs <= now;
                bool stale = (now - front.enqueuedMs) > kFailsafeMs;
                if (!due && !stale) break;

                auto in = front;
                q.pop_front();
                if (due) st.c.delivered++; else st.c.failsafe++;
                pushEvent(fmt::format(
                    "{} {} entregado {} (tarde {:.1f} ms)",
                    in.down ? "click  " : "release", in.isPlayer1 ? "P1" : "P2",
                    due ? "en hora" : "POR SEGURO", now - in.dueMs
                ));
                layer->handleButton(in.down, in.button, in.isPlayer1);
                any = true;
            }
            if (q.empty()) st.lastDueMs[i] = 0.0;
        }
        st.passthrough = false;
        if (any) st.dirty = true;
    }

    void flushAll(GJBaseGameLayer* layer) {
        auto& st = state();
        st.passthrough = true;
        for (int i = 0; i < 2; i++) {
            while (!st.queues[i].empty()) {
                auto in = st.queues[i].front();
                st.queues[i].pop_front();
                layer->handleButton(in.down, in.button, in.isPlayer1);
            }
            st.lastDueMs[i] = 0.0;
        }
        st.passthrough = false;
        st.dirty = true;
    }

    // Shift / Ctrl / Alt are tracked by the game as modifier keys, so they are polled.
    static bool holdKeyHeld() {
        auto const& name = state().cfg.holdKey;
        auto kd = CCKeyboardDispatcher::get();
        if (name == "None") return false;
        if (name == "Ctrl") return kd->getControlKeyPressed();
        if (name == "Alt") return kd->getAltKeyPressed();
        return kd->getShiftKeyPressed();
    }

    static void updateHold(GJBaseGameLayer* layer) {
        auto& st = state();
        bool held = holdKeyHeld();
        if (held == st.holdDisabled) return;
        st.holdDisabled = held;
        if (held) flushAll(layer);
        pushEvent(held ? "pausa: tecla apretada" : "pausa: tecla soltada");
        renderConsole(true);
    }

    void tick(GJBaseGameLayer* layer, int point) {
        auto& st = state();
        refreshConfig(false);
        updateHold(layer);
        // The chosen release point does the normal work; the other one only
        // runs the safety net, so a click can never get stuck.
        releaseDue(layer, st.cfg.releasePoint != point);
        renderConsole(false);
    }

    void cyclePreset() {
        auto& st = state();
        if (auto pl = PlayLayer::get()) flushAll(pl);
        st.preset = (st.preset + 1) % kPresetCount;
        resetRuntime();
        pushEvent(fmt::format("preset -> {}", kPresets[st.preset].name));
        renderConsole(true);
    }

    cocos2d::enumKeyCodes cycleKeyCode() {
        auto const& k = state().cfg.cycleKey;
        if (k == "F8") return enumKeyCodes::KEY_F8;
        if (k == "F9") return enumKeyCodes::KEY_F9;
        if (k == "F10") return enumKeyCodes::KEY_F10;
        return enumKeyCodes::KEY_F7;
    }
}
