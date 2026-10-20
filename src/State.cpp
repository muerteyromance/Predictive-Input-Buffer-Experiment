#include "State.hpp"

#include <chrono>

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

    void resetRuntime() {
        auto& s = state();
        for (int i = 0; i < 2; i++) {
            s.queues[i].clear();
            s.lastDueMs[i] = 0.0;
            s.players[i] = PlayerTrack{};
        }
    }
}
