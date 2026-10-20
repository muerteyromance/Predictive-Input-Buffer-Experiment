#include <Geode/Geode.hpp>
#include <Geode/modify/CCKeyboardDispatcher.hpp>
#include <Geode/modify/GJBaseGameLayer.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <Geode/modify/PlayerObject.hpp>

#include <algorithm>

#include "Console.hpp"
#include "Engine.hpp"
#include "State.hpp"

using namespace geode::prelude;

// Intercepts clicks and releases and holds them for a few milliseconds.
class $modify(BufferGameLayer, GJBaseGameLayer) {
    void handleButton(bool down, int button, bool isPlayer1) {
        auto& st = buf::state();

        // Our own replay, a paused/off buffer, or not a real level (editor): pass through.
        bool inLevel = typeinfo_cast<PlayLayer*>(this) != nullptr;
        if (st.passthrough || !inLevel || !buf::bufferActive()) {
            GJBaseGameLayer::handleButton(down, button, isPlayer1);
            return;
        }

        int idx = isPlayer1 ? 0 : 1;
        float delay = buf::delayMsFor(this, isPlayer1, down);
        st.c.total++;
        st.dirty = true;

        // Nothing to delay and nothing waiting for this player.
        if (delay <= 0.f && st.queues[idx].empty()) {
            st.c.direct++;
            buf::pushEvent(fmt::format(
                "{} {} {} directo (sin retraso)",
                down ? "click  " : "release", isPlayer1 ? "P1" : "P2",
                buf::modeNameOf(isPlayer1 ? this->m_player1 : this->m_player2)
            ));
            GJBaseGameLayer::handleButton(down, button, isPlayer1);
            return;
        }

        // Order is preserved per player: an input never lands before an earlier
        // one of the same player, so a release can never overtake its press.
        double now = buf::wallMs();
        double due = std::max(now + delay, st.lastDueMs[idx]);
        st.lastDueMs[idx] = due;
        st.queues[idx].push_back({down, button, isPlayer1, due, now});
        st.c.held++;
        buf::pushEvent(fmt::format(
            "{} {} {} retenido +{:.1f} ms",
            down ? "click  " : "release", isPlayer1 ? "P1" : "P2",
            buf::modeNameOf(isPlayer1 ? this->m_player1 : this->m_player2), due - now
        ));
    }

    void processCommands(float dt, bool isHalfTick, bool isLastTick) {
        if (typeinfo_cast<PlayLayer*>(this)) {
            auto& st = buf::state();
            st.c.callsPC++;
            if (isHalfTick) st.c.callsPCHalf++;
            st.c.lastDtMs = dt * 1000.f;
            buf::tick(this, 0);
        }
        GJBaseGameLayer::processCommands(dt, isHalfTick, isLastTick);
    }
};

// Second place where held inputs can be released (the original mod hooks this one).
class $modify(BufferPlayer, PlayerObject) {
    void update(float dt) {
        if (auto pl = PlayLayer::get()) {
            if (this == pl->m_player1) {
                buf::state().c.callsPU++;
                buf::tick(pl, 1);
            }
        }
        PlayerObject::update(dt);
    }
};

class $modify(BufferPlayLayer, PlayLayer) {
    void setupHasCompleted() {
        PlayLayer::setupHasCompleted();
        auto& st = buf::state();
        st.holdDisabled = false;
        st.c = buf::Counters{};
        st.events.clear();
        buf::resetRuntime();
        buf::refreshConfig(true);
        buf::pushEvent("nivel listo");
        buf::renderConsole(true);
    }

    void resetLevel() {
        buf::resetRuntime();
        PlayLayer::resetLevel();
    }

    void onQuit() {
        buf::state().holdDisabled = false;
        buf::resetRuntime();
        buf::pushEvent("saliste del nivel");
        buf::renderConsole(true);
        PlayLayer::onQuit();
    }
};

class $modify(BufferKeyboard, CCKeyboardDispatcher) {
    bool dispatchKeyboardMSG(enumKeyCodes key, bool isKeyDown, bool isKeyRepeat, double timestamp) {
        if (isKeyDown && !isKeyRepeat && key == buf::cycleKeyCode()) {
            buf::cyclePreset();
        }
        return CCKeyboardDispatcher::dispatchKeyboardMSG(key, isKeyDown, isKeyRepeat, timestamp);
    }
};

$on_mod(Loaded) {
    auto& st = buf::state();
    buf::refreshConfig(true);

    // Preset at game start, from the settings.
    auto want = Mod::get()->getSettingValue<std::string>("start-preset");
    for (int i = 0; i < buf::kPresetCount; i++) {
        if (want == buf::kPresets[i].name) st.preset = i;
    }

    buf::initConsole();
    buf::pushEvent("mod cargado");
    buf::renderConsole(true);
    log::info("[IB] loaded. preset={} cycleKey={}", buf::kPresets[st.preset].name, st.cfg.cycleKey);
}
