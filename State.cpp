#include "State.hpp"

#include <chrono>

namespace buf {
    State& state() {
        static State s;
        return s;
    }

    double wallMs() {
        using namespace std::chrono;
        return duration<double, std::milli>(steady_clock::now().time_since_epoch()).count();
    }

    void resetRuntime() {
        auto& s = state();
        for (int i = 0; i < 2; i++) {
            s.queues[i].clear();
            s.lastDueMs[i] = 0.f;
            s.players[i] = PlayerTrack{};
        }
        s.timeMs = 0.f;
    }
}
