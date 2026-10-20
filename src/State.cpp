#include "State.hpp"

namespace buf {
    State& state() {
        static State s;
        return s;
    }

    void resetRuntime() {
        auto& s = state();
        for (int i = 0; i < 2; i++) {
            s.queues[i].clear();
            s.lastDueTick[i] = 0;
            s.players[i] = PlayerTrack{};
        }
        s.tick = 0;
    }
}
