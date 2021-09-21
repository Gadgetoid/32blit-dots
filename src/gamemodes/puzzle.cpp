#include <cinttypes>

#include "puzzle.hpp"
#include "game.hpp"
#include "main-menu.hpp"
#include "colours.hpp"

uint8_t levels[][game_grid.w * game_grid.h] = {
    {
        0, 0, 0, 0, 0, 0,
        0, 0, 1, 0, 0, 0,
        0, 0, 1, 0, 0, 0,
        0, 0, 2, 0, 3, 0,
        0, 0, 1, 1, 1, 0,
        2, 2, 1, 2, 2, 3
    },
    {
        0, 2, 2, 2, 2, 0,
        0, 2, 4, 4, 2, 0,
        0, 2, 4, 4, 2, 0,
        1, 3, 5, 5, 3, 1,
        1, 3, 5, 5, 3, 1,
        1, 3, 5, 5, 3, 1
    },
    {
        0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0,
        0, 0, 1, 1, 0, 0,
        0, 0, 3, 3, 0, 0,
        0, 1, 3, 3, 1, 0,
        1, 2, 2, 2, 2, 1
    },
    {
        1, 2, 3, 4, 5, 0,
        0, 0, 0, 0, 0, 0,
        2, 2, 2, 2, 2, 2,
        0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0,
        1, 2, 3, 4, 5, 5
    }
};

void Puzzle::update(uint32_t time) {
    update_menus(time);

    // Update the dots game state here
    update_particles();
    update_dots();
    update_input(false);
}

void Puzzle::on_won() {
    next_level();
    load_level();
    resume();
}

void Puzzle::on_failed() {
    explode_all_dots();
    load_level();
    resume();
}

void Puzzle::load_level() {
    clear_dots();
    clear_game_state();

    uint32_t level = get_level();

    uint8_t *level_data = levels[level];

    for(auto x = 0u; x < game_grid.w; x++) {
        for(auto y = 0u; y < game_grid.h; y++) {
            int8_t color = level_data[x + y * game_grid.w];
            if(color > 0) {
                add_dot(Dot(blit::Point(x, y - 7), color - 1));
            }
        }
    }
}

void Puzzle::restart() {
    explode_all_dots();
    reset_score();
    load_level();
    resume();
}

void Puzzle::render_level_select(PauseMenuItem id, bool in_menu, blit::Rect level_select_rect) {
    std::string text = "";
    if(id == Menu_Change_Seed && in_menu) {text += "< ";};
    text += "Level: ";
    text += std::to_string(get_level() + 1);
    if(id == Menu_Change_Seed && in_menu) {text += " >";};
    blit::screen.pen = colour_sky_blue;
    blit::screen.text(text, outline_font_10x14, level_select_rect, true, blit::TextAlign::center_center);
}