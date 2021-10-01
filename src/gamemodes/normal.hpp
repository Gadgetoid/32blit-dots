#pragma once

#include <cstdint>
#include "fonts.hpp"
#include "stage.hpp"
#include "menu.hpp"
#include "dot.hpp"
#include "particles.hpp"

#include "gamemode.hpp"

class Normal final : public GameMode {
    public:
        Normal(Game *game) : GameMode(game, "Normal", "Change Seed") {
            set_level(0x00750075, 0xFFFFFFFF);
            load_level();
        };

        void update(uint32_t time) override;
};