#include "State.hpp"

#include <Geode/Geode.hpp>

#include <chrono>

using namespace geode::prelude;

namespace buf {
    State& state() {
        static State s;
        return s;
    }

    double wallMs() {
        using namespace std::chrono;
        static auto const start = steady_clock::now();
        return duration<double, std::milli>(steady_clock::now() - start).count();
    }

    void refreshConfig(bool force) {
        auto& st = state();
        double now = wallMs();
        if (!force && now - st.lastCfgMs < 250.0) return;
        st.lastCfgMs = now;

        auto m = Mod::get();
        auto geti = [&](char const* k) { return static_cast<int>(m->getSettingValue<int64_t>(k)); };
        auto& c = st.cfg;

        c.tps = geti("tps");
        c.releasePct = geti("release-percent");
        c.maxMs = geti("max-ms");
        c.wavePct = geti("wave-percent");
        c.shipPct = geti("ship-percent");
        c.ufoPct = geti("ufo-percent");
        c.cubePct = geti("cube-percent");
        c.ballPct = geti("ball-percent");
        c.robotPct = geti("robot-percent");
        c.spiderPct = geti("spider-percent");
        c.swingPct = geti("swing-percent");
        c.debug = m->getSettingValue<bool>("debug");
        c.consoleEnabled = m->getSettingValue<bool>("console-enabled");
        c.releasePoint = (m->getSettingValue<std::string>("release-point") == "processCommands") ? 0 : 1;
        c.cycleKey = m->getSettingValue<std::string>("cycle-key");
        c.holdKey = m->getSettingValue<std::string>("hold-disable-key");
        st.dirty = true;
    }

    void pushEvent(std::string text) {
        auto& st = state();
        if (st.cfg.debug) log::info("[IB] {}", text);
        st.events.push_back(std::move(text));
        while (st.events.size() > 7) st.events.pop_front();
        st.dirty = true;
    }

    void resetRuntime() {
        auto& st = state();
        for (int i = 0; i < 2; i++) {
            st.queues[i].clear();
            st.lastDueMs[i] = 0.0;
        }
        st.dirty = true;
    }
}
