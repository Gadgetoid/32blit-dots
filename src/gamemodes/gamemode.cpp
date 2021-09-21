#include <cinttypes>

#include "normal.hpp"
#include "game.hpp"
#include "main-menu.hpp"
#include "colours.hpp"

GameMode::GameMode(Game *game, std::string_view game_mode_title, const char *change_level_text) : game(game),
    pause_menu("Paused", {
        {Menu_Continue, "Continue"},
        {Menu_Restart, "Restart"},
        {Menu_Change_Brightness, "Brightness"},
        {Menu_Quit_Level, "Quit"}
    }, outline_font_10x14),
    end_menu(game_mode_title, {
        {Menu_Start, "Start"},
        {Menu_Change_Seed, change_level_text},
        {Menu_Change_Brightness, "Brightness"},
        {Menu_Quit, "Mein Menu"}
    }, outline_font_10x14) {

    state = EndMenu;

    pause_menu.set_display_rect({blit::screen.bounds.w - menu_target_width, 0, menu_target_width, blit::screen.bounds.h});
    pause_menu.set_on_item_activated(std::bind(&GameMode::on_menu_activated, this, std::placeholders::_1));
    //pause_menu.set_on_item_rendered(std::bind(&GameMode::on_menu_rendered, this, std::placeholders::_1));
    pause_menu.set_on_item_updated(std::bind(&GameMode::on_menu_updated, this, std::placeholders::_1));

    end_menu.set_display_rect({blit::screen.bounds.w - menu_target_width, 0, menu_target_width, blit::screen.bounds.h});
    end_menu.set_on_item_activated(std::bind(&GameMode::on_menu_activated, this, std::placeholders::_1));
    //end_menu.set_on_item_rendered(std::bind(&GameMode::on_menu_rendered, this, std::placeholders::_1));
    end_menu.set_on_item_updated(std::bind(&GameMode::on_menu_updated, this, std::placeholders::_1));

    global_dot_offset = blit::Point(
        (blit::screen.bounds.w - game_bounds.w) / 2,
        (blit::screen.bounds.h - game_bounds.h) / 2
    ) + (dot_spacing / 2);

    chain.reserve(game_grid.area());
    dots.reserve(game_grid.area());
};

void GameMode::render_level_select(PauseMenuItem id, bool in_menu, blit::Rect level_select_rect) {
    char buf[9];
    std::string text = "";

    if (in_menu) {
        blit::screen.pen = colour_sky_blue;
        blit::screen.rectangle(level_select_rect);
        level_select_rect.deflate(2);
        blit::screen.pen = colour_white;
        blit::screen.rectangle(level_select_rect);
        level_select_rect.inflate(2);
    }

    snprintf(buf, 9, "%08" PRIX32, current_level);
    if(id == Menu_Change_Seed && in_menu) {text += "< ";};
    text += "Seed: ";
    text += buf;
    if(id == Menu_Change_Seed && in_menu) {text += " >";};
    blit::screen.pen = colour_sky_blue;
    blit::screen.text(text, outline_font_10x14, level_select_rect, true, blit::TextAlign::center_center);
}

void GameMode::render() {
    blit::Rect message_rect(0, 0, blit::screen.bounds.w, 24);
    auto item = state == EndMenu ? end_menu.get_selected_item() : pause_menu.get_selected_item();
    bool in_menu = state == EndMenu || state == Paused;

    blit::Rect brightness_rect(2, 0, blit::screen.bounds.w - menu_target_width - 4, 24);

    brightness_rect.y += (blit::screen.bounds.h / 2) - (brightness_rect.h / 2);

    message_rect.y = blit::screen.bounds.h - message_rect.h;

    blit::screen.pen = colour_background;
    blit::screen.clear();

    // Draw the cursor
    if(state == Running) {
        blit::Point selected_origin = global_dot_offset + selected * dot_spacing;
        blit::screen.pen = colour_selected;
        blit::screen.circle(selected_origin, 15);
        blit::screen.pen = colour_background;
        blit::screen.circle(selected_origin, 13);
    }

    for(auto &dot : dots) {
        if(dot.grid_location == selected || dot.in(chain)) {
            blit::screen.pen = dot.selected_colour;
        } else {
            blit::screen.pen = dot.colour;
        }
        blit::screen.circle(global_dot_offset + dot.screen_location(), dot_radius);
    };

    if(chain.size() > 0) {
        Dot *last = nullptr;
        blit::Pen chain_colour = chain.front()->selected_colour;
        for(auto &dot : chain) {
            if(last) {
                blit::screen.pen = chain_colour;
                blit::Rect link(0, 0, 0, 0);

                blit::Point last_position = last->screen_location();
                blit::Point dot_position = dot->screen_location();
        
                if(last_position.y == dot_position.y) {
                    link.x = std::min(last_position.x, dot_position.x);
                    link.w = abs(last_position.x - dot_position.x);
                    link.h = 2 * dot_radius + 1; // Wtf?
                    link.y = last_position.y - dot_radius;

                }
                if(last_position.x == dot_position.x) {
                    link.y = std::min(last_position.y, dot_position.y);
                    link.h = abs(last_position.y - dot_position.y);
                    link.w = 2 * dot_radius + 1;
                    link.x = last_position.x - dot_radius;
                }
                link.x += global_dot_offset.x;
                link.y += global_dot_offset.y;
                blit::screen.rectangle(link);
                last = dot;
            }

            last = dot;
        };
    }

    for(auto &p: particles){
        p.render(global_dot_offset);
    }
    blit::screen.alpha = 255;

    if(in_menu) {
        message_rect.w -= menu_target_width;

        switch(item.id) {
            case Menu_Change_Brightness:
                blit::screen.pen = colour_sky_blue;
                blit::screen.rectangle(brightness_rect);
                brightness_rect.deflate(2);
                blit::screen.pen = colour_background;
                blit::screen.rectangle(brightness_rect);
                brightness_rect.deflate(2);
                blit::screen.pen = colour_sky_blue;
                brightness_rect.w *= brightness;
                brightness_rect.w /= 255;
                blit::screen.rectangle(brightness_rect);
                break;
            default:
                break;
        }
    }

    render_level_select((PauseMenuItem)item.id, in_menu, message_rect);

    // Draw the score
    message_rect.y = 0;
    blit::screen.pen = colour_white;
    blit::screen.rectangle(message_rect);
    blit::screen.pen = colour_sky_blue;

    if(chain.size() >= 2) {
        uint32_t chain_score = chain.size() * chain.size() * chain.size();
        if(multiplier > 1) {
            blit::screen.text(std::to_string(score) + " + (" + std::to_string(chain_score) + " * " + std::to_string(multiplier) + ")", outline_font_10x14, message_rect, true, blit::TextAlign::center_center);
        } else {
            blit::screen.text(std::to_string(score) + " + " + std::to_string(chain_score), outline_font_10x14, message_rect, true, blit::TextAlign::center_center);
        }
    } else {
        blit::screen.text(std::to_string(score), outline_font_10x14, message_rect, true, blit::TextAlign::center_center);
    }

    if(state == Failed) {render_failed();return;}
    if(state == Won) {render_won();return;}
    if(state == Paused) {pause_menu.render();return;}
    if(state == EndMenu) {end_menu.render();return;}
}

GameMode::~GameMode() {

}

void GameMode::on_won() {

}

void GameMode::on_failed() {

}

void GameMode::render_won() {

}

void GameMode::render_failed() {

}

void GameMode::on_menu_rendered(const ::Menu::Item &item) {

}

void GameMode::refill_dots(const uint8_t *column_counts) {
    for(auto x = 0u; x < game_grid.w; x++) {
        uint8_t count = column_counts[x];
        while(count--) {
            dots.push_back(Dot(blit::Point(x, -1 - count)));
        }
    }

	std::sort(dots.begin(), dots.end());
    for(auto &dot : dots) {
        if(dot.grid_location.y < 0) continue;
        game_state[dot.grid_location.x][dot.grid_location.y] = &dot;
    }
}

void GameMode::explode(blit::Point origin, blit::Pen colour) {
    for(auto x = 0u; x < 5; x++) {
        particles.push_back(SpaceDust(origin, colour));
    }
}

void GameMode::explode_chain(bool refill) {
    uint8_t column_counts[game_grid.w] = {0, 0, 0, 0, 0, 0};

    // This will cause a fail to explode in a failed puzzle mode with 1 dot left
    if(chain.size() < 2) {
        chain.clear();
        return;
    };

    for(auto &dot : chain) {
        dot->explode = true;
        explode(dot->position * dot_spacing, dot->colour);
        column_counts[dot->grid_location.x] += 1;
        game_state[dot->grid_location.x][dot->grid_location.y] = nullptr;
    }

    dots.erase(std::remove_if(dots.begin(), dots.end(), [](Dot dot){return dot.explode;}), dots.end());

    if(refill) {
        refill_dots(column_counts);
    }

    score += chain.size() * chain.size() * chain.size() * multiplier;

    if(chain.size() >= 4) {
        multiplier++;
    } else {
        multiplier = 1;
    }

    blit::channels[2].frequency = 400 + (100 * chain.size());
    blit::channels[2].trigger_attack();

    chain.clear();
}

void GameMode::load_level() {
    uint8_t column_counts[game_grid.w];
    for(auto i = 0u; i < game_grid.w; i++) {
        column_counts[i] = game_grid.h;
    }
    random_reset(current_level);
    refill_dots(column_counts);
}

void GameMode::restart() {
    explode_all_dots();
    reset_score();
    load_level();
    state = Running;
}

void GameMode::quit() {
    explode_all_dots();
    reset_score();
    load_level();
    state = EndMenu;
}

void GameMode::on_menu_updated(const ::Menu::Item &item) {
    switch(item.id) {
        case Menu_Continue:
            break;
        case Menu_Change_Seed:
            if (blit::buttons.pressed & blit::Button::DPAD_LEFT) {
                prev_level();
            }
            if (blit::buttons.pressed & blit::Button::DPAD_RIGHT) {
                next_level();
            }
            break;
        case Menu_Change_Brightness:
            if (blit::buttons.pressed & blit::Button::DPAD_LEFT) {
                brightness -= 16;
                if(brightness < 0) brightness = 0;
                set_backlight((uint8_t)brightness);
            }
            if (blit::buttons.pressed & blit::Button::DPAD_RIGHT) {
                brightness += 16;
                if(brightness > 255) brightness = 255;
                set_backlight((uint8_t)brightness);
            }
            break;
        case Menu_Restart:
            break;
        case Menu_Quit:
            break;
    }
}

void GameMode::on_menu_activated(const ::Menu::Item &item) {
    switch(item.id) {
        case Menu_Continue:
            state = Running;
            break;
        case Menu_Change_Seed:
            break;
        case Menu_Start:
            state = Running;
            break;
        case Menu_Restart:
            restart();
            break;
        case Menu_Quit_Level:
            quit();
            break;
        case Menu_Quit:
            game->change_state<MainMenu>();
            break;
    }
}

void GameMode::update_particles() {
    if(state != Running && state != Failed && state != EndMenu) return;

    for(auto &p: particles){
        p.update();
    }

    particles.erase(std::remove_if(particles.begin(), particles.end(), [](SpaceDust particle) { return (particle.age >= P_MAX_AGE); }), particles.end());
}

void GameMode::explode_all_dots() {
    for(auto &dot : dots) {
        explode(dot.position * dot_spacing, dot.colour);
    }
    clear_game_state();
    dots.clear();
}

void GameMode::update_dots() {
    if(state != Running && state != EndMenu) return;

    bool falling = false;

    for(auto &dot : dots) {
        // Easy way to collide with the "floor"
        if(dot.grid_location.y == game_grid.h - 1) {
            dot.position.y &= 0xffffff00;
            continue;
        };
        auto dot_below = dot_at(dot.grid_location + blit::Point(0, 1));
        if(dot_below) {
            dot.position.y &= 0xffffff00;
        } else {
            dot.position.y += 25;
            falling = true;
        }
        dot.update_location();
    }

    if(!falling && needs_update) {
        // Reset the game state to empty
        clear_game_state();
        // Rebuild the game state from the available dots
        for(auto &dot : dots) {
            if(dot.grid_location.y < 0) continue;
            game_state[dot.grid_location.x][dot.grid_location.y] = &dot;
        }
        if(!move_available()) {
            if(grid_empty()) {
                state = Won;
                on_won();
            } else {
                state = Failed;
                on_failed();
            }
            return;
        }
        needs_update = false;
    }
}

void GameMode::update_input(bool refill_destroyed_dots) {
    if(state != Running) return;

    blit::Point movement(0, 0);

    if(blit::buttons.pressed & blit::Button::DPAD_RIGHT) {
        movement.x = 1;
    }else if(blit::buttons.pressed & blit::Button::DPAD_LEFT) {
        movement.x = -1;
    }
    if(!game_grid.contains(selected + movement)) {
        movement.x = 0;
    }
    if(blit::buttons.pressed & blit::Button::DPAD_DOWN) {
        movement.y = 1;
    }else if(blit::buttons.pressed & blit::Button::DPAD_UP) {
        movement.y = -1;
    }
    if(!game_grid.contains(selected + movement)) {
        movement.y = 0;
    }
    selected += movement;

    if(blit::buttons & blit::Button::A) {
        auto dot = game_state[selected.x][selected.y];
        if(dot) {
            if(chain.size() == 0) {
                chain.push_back(dot);
            } else if(dot->colour == chain.front()->colour && adjacent(dot, chain.back())) {
                if(!dot->in(chain)) {
                    chain.push_back(dot);
                } else {
                    // Dot is in chain already
                    if(dot == chain[chain.size() - 2]) {
                        chain.pop_back();
                    } else {
                        selected -= movement;
                    }
                }
            } else {
                selected -= movement;
            }
        } else {
            selected -= movement;
        }
    } else {
        explode_chain(refill_destroyed_dots);
        needs_update = true;
    }
}

void GameMode::update_menus(uint32_t time) {
    // Button B lets us enter/exit the pause menu
    if(blit::buttons.pressed & blit::Button::B) {
        if(state == Running || state == Failed) {
            pause();
        } else if(state == Paused) {
            resume();
        }
    }

    if(state == Paused) {pause_menu.update(time);return;}
    if(state == EndMenu) {end_menu.update(time);return;}
}