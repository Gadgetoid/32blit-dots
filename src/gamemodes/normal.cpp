#include <cinttypes>

#include "normal.hpp"
#include "game.hpp"
#include "main-menu.hpp"
#include "colours.hpp"

Normal::Normal(Game *game) : game(game),
    pause_menu("Paused", {
        {Menu_Continue, "Continue"},
        {Menu_Restart, "Restart"},
        {Menu_Change_Seed, "Change Seed"},
        {Menu_Change_Brightness, "Brightness"},
        {Menu_Quit, "Quit"}
    }, outline_font_10x14),
    end_menu("Normal", {
        {Menu_Restart, "Start"},
        {Menu_Change_Seed, "Change Seed"},
        {Menu_Change_Brightness, "Brightness"},
        {Menu_Quit, "Quit"}
    }, outline_font_10x14) {

    state = Failed;

    pause_menu.set_display_rect({blit::screen.bounds.w - menu_target_width, 0, menu_target_width, blit::screen.bounds.h});
    pause_menu.set_on_item_activated(std::bind(&Normal::on_menu_activated, this, std::placeholders::_1));
    //pause_menu.set_on_item_rendered(std::bind(&Normal::on_menu_rendered, this, std::placeholders::_1));
    pause_menu.set_on_item_updated(std::bind(&Normal::on_menu_updated, this, std::placeholders::_1));

    end_menu.set_display_rect({blit::screen.bounds.w - menu_target_width, 0, menu_target_width, blit::screen.bounds.h});
    end_menu.set_on_item_activated(std::bind(&Normal::on_menu_activated, this, std::placeholders::_1));
    //end_menu.set_on_item_rendered(std::bind(&Normal::on_menu_rendered, this, std::placeholders::_1));
    end_menu.set_on_item_updated(std::bind(&Normal::on_menu_updated, this, std::placeholders::_1));

    global_dot_offset = blit::Point(
        (blit::screen.bounds.w - game_bounds.w) / 2,
        (blit::screen.bounds.h - game_bounds.h) / 2
    ) + (dot_spacing / 2);
};

void Normal::render() {
    char buf[9];
    std::string text = "";
    blit::Rect message_rect(0, 0, blit::screen.bounds.w, 24);
    auto item = state == Failed ? end_menu.get_selected_item() : pause_menu.get_selected_item();
    bool running = state == Running;

    blit::Rect brightness_rect(2, 0, blit::screen.bounds.w - menu_target_width - 4, 24);

    brightness_rect.y += (blit::screen.bounds.h / 2) - (brightness_rect.h / 2);

    message_rect.y = blit::screen.bounds.h - message_rect.h;

    blit::screen.pen = colour_background;
    blit::screen.clear();

    // Draw the cursor
    if(state != Failed) {
        blit::Point selected_origin = global_dot_offset + selected * dot_spacing;
        blit::screen.pen = colour_selected;
        blit::screen.circle(selected_origin, 15);
        blit::screen.pen = colour_background;
        blit::screen.circle(selected_origin, 13);
    }

    for(auto &dot : dots) {
        if(dot.explode) continue;
    
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

    if(!running) {
        message_rect.w -= menu_target_width;

        switch(item.id) {
            case Menu_Change_Seed:
                blit::screen.pen = colour_sky_blue;
                blit::screen.rectangle(message_rect);
                message_rect.deflate(2);
                blit::screen.pen = colour_white;
                blit::screen.rectangle(message_rect);
                message_rect.inflate(2);
                break;
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

    snprintf(buf, 9, "%08" PRIX32, current_random_seed);
    if(item.id == Menu_Change_Seed && !running) {text += "< ";};
    text += "Seed: ";
    text += buf;
    if(item.id == Menu_Change_Seed && !running) {text += " >";};
    blit::screen.pen = colour_sky_blue;
    blit::screen.text(text, outline_font_10x14, message_rect, true, blit::TextAlign::center_center);

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

    if(state == Paused) {pause_menu.render();return;}
    if(state == Failed) {end_menu.render();return;}
}

void Normal::update(uint32_t time) {
    blit::Point movement(0, 0);
    static bool needs_update = false;
    bool falling = false;

    // Button B lets us enter/exit the pause menu
    if(blit::buttons.pressed & blit::Button::B) {
        if(state == Running) {
            state = Paused;
        } else if(state == Paused) {
            state = Running;
        }
    }

    if(state == Paused) {pause_menu.update(time);return;}
    if(state == Failed) {end_menu.update(time);return;}

    // Update the dots game state here

    for(auto &p: particles){
        p.update();
    }

    particles.erase(std::remove_if(particles.begin(), particles.end(), [](SpaceDust particle) { return (particle.age >= P_MAX_AGE); }), particles.end());

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
        for(auto &dot : dots) {
            if(dot.grid_location.y < 0) continue;
            game_state[dot.grid_location.x][dot.grid_location.y] = &dot;
        }
        if(!move_available()) {
            state = Failed;
            return;
        }
        needs_update = false;
    }

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
        explode_chain(true);
        needs_update = true;
    }
}

Normal::~Normal() {

}

void Normal::on_menu_rendered(const ::Menu::Item &item) {

}

void Normal::refill_dots(uint8_t *column_counts) {
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

void Normal::explode(blit::Point origin, blit::Pen colour, float factor) {
    blit::channels[2].frequency = 800;
    blit::channels[2].trigger_attack();
    uint8_t count = 5 + (blit::random() % 5);
    count *= factor;
    for(auto x = 0u; x < count; x++) {
        particles.push_back(SpaceDust(origin, colour));
    }
}

void Normal::explode_chain(bool refill) {
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

    chain.clear();
}

void Normal::restart() {
    for(auto &dot : dots) {
        chain.push_back(&dot);
    }
    explode_chain();

    score = 0;
    multiplier = 1;
    uint8_t column_counts[game_grid.w];
    for(auto i = 0u; i < game_grid.w; i++) {
        column_counts[i] = game_grid.h;
    }
    random_reset(current_random_seed);
    refill_dots(column_counts);
    state = Running;
}

void Normal::on_menu_updated(const ::Menu::Item &item) {
    switch(item.id) {
        case Menu_Continue:
            break;
        case Menu_Change_Seed:
            if (blit::buttons.pressed & blit::Button::DPAD_LEFT) {
                current_random_seed++;
            }
            if (blit::buttons.pressed & blit::Button::DPAD_RIGHT) {
                current_random_seed--;
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

void Normal::on_menu_activated(const ::Menu::Item &item) {
    switch(item.id) {
        case Menu_Continue:
            state = Running;
            break;
        case Menu_Change_Seed:
            break;
        case Menu_Restart:
            restart();
            break;
        case Menu_Quit:
            game->change_state<MainMenu>();
            break;
    }
}