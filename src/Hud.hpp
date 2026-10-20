#pragma once

#include <Geode/Geode.hpp>

namespace buf {
    void createHud(PlayLayer* layer);
    void destroyHud();   // just forgets the pointer; the node dies with the layer
    void refreshHud();
}
