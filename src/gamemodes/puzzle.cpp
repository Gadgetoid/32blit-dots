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
}

void Puzzle::load_level() {
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

    if (id == Menu_Change_Seed && in_menu) {
        blit::screen.pen = colour_sky_blue;
        blit::screen.rectangle(level_select_rect);
        level_select_rect.deflate(2);
        blit::screen.pen = colour_white;
        blit::screen.rectangle(level_select_rect);
        level_select_rect.inflate(2);
    }

    if((id == Menu_Change_Seed || id == Menu_Restart) && in_menu) {
        uint32_t level = get_level();
        uint8_t *level_data = levels[level];
        blit::Point dot_offset(
            (blit::screen.bounds.w - menu_target_width - 90) / 2,
            (blit::screen.bounds.h - 90) / 2);

        blit::Rect outline(dot_offset, blit::Size(87, 87));

        dot_offset += blit::Point(5, 5);

        outline.inflate(2);

        if(id == Menu_Change_Seed) {
            outline.inflate(2);
            blit::screen.pen = colour_sky_blue;
            blit::screen.rectangle(outline);
            outline.deflate(2);
        }
        blit::screen.pen = colour_background;
        blit::screen.rectangle(outline);


        for(auto x = 0u; x < game_grid.w; x++) {
            for(auto y = 0u; y < game_grid.h; y++) {
                int8_t color = level_data[x + y * game_grid.w];
                if(color > 0) {
                    blit::screen.pen = DOT_COLOURS[color - 1];
                    blit::screen.circle(dot_offset + blit::Point(x, y) * blit::Point(15, 15), 5);
                }
            }
        }
    }

    if(id == Menu_Change_Seed && in_menu) {text += "< ";};
    text += "Level: ";
    text += std::to_string(get_level() + 1);
    if(id == Menu_Change_Seed && in_menu) {text += " >";};
    blit::screen.pen = colour_sky_blue;
    blit::screen.text(text, outline_font_10x14, level_select_rect, true, blit::TextAlign::center_center);


}