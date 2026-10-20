#pragma once

#include <Geode/Geode.hpp>

namespace buf {
    void createHud(PlayLayer* layer);
    void destroyHud();   // just forgets the pointers; the nodes die with the layer
    void refreshHud();
    void refreshDebug();
}
