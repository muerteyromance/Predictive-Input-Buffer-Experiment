#include <Geode/Geode.hpp>
#include <Geode/modify/CCKeyboardDispatcher.hpp>
#include <Geode/modify/GJBaseGameLayer.hpp>
#include <Geode/modify/PlayLayer.hpp>

#include <algorithm>
#include <string>

#include "Config.hpp"
#include "Hud.hpp"
#include "Situation.hpp"
#include "State.hpp"

using namespace geode::prelude;

namespace {
    bool bufferActive() {
        auto const& st = buf::state();
        return st.level > 0 && !st.holdDisabled;
    }

    // Applies everything still waiting, right now, in order. Used when the
    // buffer gets turned off or paused so no click or release is ever lost
    // (a lost release would leave the button "stuck" down).
    void flushPending(GJBaseGameLayer* layer) {
        auto& st = buf::state();
        st.passthrough = true;
        for (int i = 0; i < 2; i++) {
            while (!st.queues[i].empty()) {
                auto in = st.queues[i].front();
                st.queues[i].pop_front();
                layer->handleButton(in.down, in.button, in.isPlayer1);
            }
            st.lastDueMs[i] = 0.f;
        }
        st.passthrough = false;
    }

    enumKeyCodes holdKeyCode() {
        auto name = buf::holdKeyName();
        if (name == "Ctrl") return enumKeyCodes::KEY_Control;
        if (name == "Tab") return enumKeyCodes::KEY_Tab;
        if (name == "F9") return enumKeyCodes::KEY_F9;
        return enumKeyCodes::KEY_Shift;
    }
}

// Intercepts clicks and releases, and replays them a few milliseconds later.
class $modify(BufferGameLayer, GJBaseGameLayer) {
    void handleButton(bool down, int button, bool isPlayer1) {
        auto& st = buf::state();

        // Our own replay, or buffer off/paused, or not a real level
        // (the editor): pass straight through.
        bool inLevel = typeinfo_cast<PlayLayer*>(this) != nullptr;
        if (st.passthrough || !inLevel || !bufferActive()) {
            GJBaseGameLayer::handleButton(down, button, isPlayer1);
            return;
        }

        int idx = isPlayer1 ? 0 : 1;
        float delay = buf::delayMsFor(this, isPlayer1);

        // Nothing to delay and nothing waiting for this player.
        if (delay <= 0.f && st.queues[idx].empty()) {
            GJBaseGameLayer::handleButton(down, button, isPlayer1);
            return;
        }

        // Order is preserved per player: an input never lands before an
        // earlier one of the same player, so a release can't overtake its press.
        float due = std::max(st.timeMs + delay, st.lastDueMs[idx]);
        st.lastDueMs[idx] = due;
        st.queues[idx].push_back({down, button, isPlayer1, due, buf::wallMs()});
    }

    void processCommands(float dt, bool isHalfTick, bool isLastTick) {
        if (typeinfo_cast<PlayLayer*>(this)) {
            auto& st = buf::state();

            // Game time advances by the dt of every update, so this does not
            // depend on how the game splits its steps.
            if (dt > 0.f && dt < 0.1f) {
                st.timeMs += dt * 1000.f;
            }
            buf::updateSituation(this);

            double nowWall = buf::wallMs();
            for (int i = 0; i < 2; i++) {
                auto& q = st.queues[i];
                while (!q.empty()) {
                    auto const& front = q.front();
                    bool due = front.dueMs <= st.timeMs;
                    bool stale = (nowWall - front.enqueuedWallMs) > buf::kFailsafeMs;
                    if (!due && !stale) break;

                    auto in = front;
                    q.pop_front();
                    GJBaseGameLayer::handleButton(in.down, in.button, in.isPlayer1);
                }
            }
        }
        GJBaseGameLayer::processCommands(dt, isHalfTick, isLastTick);
    }
};

class $modify(BufferPlayLayer, PlayLayer) {
    void setupHasCompleted() {
        PlayLayer::setupHasCompleted();
        buf::state().holdDisabled = false;
        buf::resetRuntime();
        buf::createHud(this);
    }

    void resetLevel() {
        buf::resetRuntime();
        PlayLayer::resetLevel();
    }

    void onQuit() {
        buf::destroyHud();
        buf::state().holdDisabled = false;
        buf::resetRuntime();
        PlayLayer::onQuit();
    }
};

// F6: intensity, F7: per-gamemode logic, F8: on-screen display,
// hold key (setting): temporarily disable the buffer while held.
class $modify(BufferKeyboard, CCKeyboardDispatcher) {
    bool dispatchKeyboardMSG(enumKeyCodes key, bool isKeyDown, bool isKeyRepeat, double timestamp) {
        auto& st = buf::state();

        if (key == holdKeyCode() && !isKeyRepeat) {
            if (isKeyDown != st.holdDisabled) {
                st.holdDisabled = isKeyDown;
                if (isKeyDown) {
                    if (auto pl = PlayLayer::get()) flushPending(pl);
                }
                buf::refreshHud();
            }
        }

        if (isKeyDown && !isKeyRepeat) {
            if (key == enumKeyCodes::KEY_F6) {
                if (auto pl = PlayLayer::get()) flushPending(pl);
                st.level = (st.level + 1) % buf::kLevelCount;
                buf::resetRuntime();
                buf::refreshHud();
                log::info("[InputBuffer] Intensity: {}%", static_cast<int>(buf::kLevels[st.level] * 100.f));
            } else if (key == enumKeyCodes::KEY_F7) {
                st.situational = !st.situational;
                buf::refreshHud();
                log::info("[InputBuffer] Per-gamemode logic: {}", st.situational ? "on" : "off");
            } else if (key == enumKeyCodes::KEY_F8) {
                buf::setHudEnabled(!buf::hudEnabled());
                buf::refreshHud();
            }
        }
        return CCKeyboardDispatcher::dispatchKeyboardMSG(key, isKeyDown, isKeyRepeat, timestamp);
    }
};

$on_mod(Loaded) {
    log::info("[InputBuffer] Loaded. F6 intensity, F7 per-gamemode, F8 display, hold key = pause.");
}
