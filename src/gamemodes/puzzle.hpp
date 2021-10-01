#pragma once

#include <cstdint>
#include "fonts.hpp"
#include "stage.hpp"
#include "menu.hpp"
#include "dot.hpp"
#include "particles.hpp"

#include "gamemode.hpp"


class Puzzle final : public GameMode {
    public:
        Puzzle(Game *game) : GameMode(game, "Puzzle", "Select Level")  {
            set_level(0, 3);
            load_level();
        };;

        void update(uint32_t time) override;
        void restart() override;
        void render_level_select(PauseMenuItem id, bool in_menu, blit::Rect level_select_rect) override;
        void on_won() override;
        void on_failed() override;
        void load_level() override;
};