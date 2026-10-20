#include "Hud.hpp"

#include "Config.hpp"
#include "State.hpp"

using namespace geode::prelude;

namespace buf {
    static CCLabelBMFont* g_label = nullptr;
    static CCLabelBMFont* g_debug = nullptr;

    static std::string hudText() {
        auto const& st = state();
        if (st.level <= 0) return "Buffer: OFF";
        if (st.holdDisabled) return "Buffer: PAUSED (key held)";
        return fmt::format(
            "Buffer {}% [{}]",
            static_cast<int>(kLevels[st.level] * 100.f),
            st.situational ? "per-mode" : "flat"
        );
    }

    void createHud(PlayLayer* layer) {
        g_label = CCLabelBMFont::create("", "bigFont.fnt");
        g_label->setAnchorPoint({0.f, 1.f});
        auto win = CCDirector::get()->getWinSize();
        g_label->setPosition({6.f, win.height - 6.f});
        layer->addChild(g_label, 1000);

        g_debug = CCLabelBMFont::create("", "bigFont.fnt");
        g_debug->setAnchorPoint({0.f, 1.f});
        g_debug->setPosition({6.f, win.height - 22.f});
        g_debug->setScale(0.3f);
        g_debug->setOpacity(170);
        layer->addChild(g_debug, 1000);

        refreshHud();
        refreshDebug();
    }

    void destroyHud() {
        g_label = nullptr;
        g_debug = nullptr;
    }

    void refreshDebug() {
        if (!g_debug) return;
        bool on = debugHudEnabled() && hudEnabled();
        g_debug->setVisible(on);
        if (!on) return;
        auto const& d = state().dbg;
        size_t waiting = state().queues[0].size() + state().queues[1].size();
        g_debug->setString(fmt::format(
            "calls {} half {} dt {:.2f}ms | in {} queued {} out {} stale {} wait {} | delay {:.1f}ms",
            d.calls, d.halfCalls, d.lastDtMs, d.in, d.queued, d.out, d.stale, waiting, d.lastDelayMs
        ).c_str());
    }

    void refreshHud() {
        if (!g_label) return;
        bool on = hudEnabled();
        g_label->setVisible(on);
        if (!on) return;
        g_label->setScale(hudScalePercent() / 100.f);
        g_label->setOpacity(static_cast<GLubyte>(hudOpacity()));
        g_label->setString(hudText().c_str());
        refreshDebug();
    }
}
