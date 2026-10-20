#include "Hud.hpp"

#include "Config.hpp"
#include "State.hpp"

using namespace geode::prelude;

namespace buf {
    static CCLabelBMFont* g_label = nullptr;

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
        refreshHud();
    }

    void destroyHud() {
        g_label = nullptr;
    }

    void refreshHud() {
        if (!g_label) return;
        bool on = hudEnabled();
        g_label->setVisible(on);
        if (!on) return;
        g_label->setScale(hudScalePercent() / 100.f);
        g_label->setOpacity(static_cast<GLubyte>(hudOpacity()));
        g_label->setString(hudText().c_str());
    }
}
