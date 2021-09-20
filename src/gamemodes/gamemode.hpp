#pragma once

#include <cstdint>
#include "fonts.hpp"
#include "stage.hpp"
#include "menu.hpp"
#include "dot.hpp"
#include "particles.hpp"

#ifdef PICO_BOARD
#include "hardware/pwm.h"
#endif

// TODO merge this upstream? We're using "Point" as a fixed 24-8 alternative to Vec2
namespace blit {
    inline Point operator*  (Point lhs, const Point &rhs) { lhs.x *= rhs.x; lhs.y *= rhs.y; return lhs; }
}

class Game;

class GameMode : public Stage {
public:
    GameMode(Game *game, std::string_view game_mode_title, const char *change_level_text);
    ~GameMode() override;

    enum GameState {
        Running = 0,
        Paused,
        Failed,
        Won,
        EndMenu
    };

    enum PauseMenuItem {
        Menu_Continue = 0,
        Menu_Change_Seed,
        Menu_Change_Brightness,
        Menu_Restart,
        Menu_Quit
    };

    void render() override;
    virtual void restart();
    virtual void render_level_select(PauseMenuItem id, bool in_menu, blit::Rect level_select_rect);
    virtual void on_won();
    virtual void on_failed();

    virtual void render_won();
    virtual void render_failed();

    void on_menu_rendered(const ::Menu::Item &item);
    void on_menu_updated(const ::Menu::Item &item);
    void on_menu_activated(const Menu::Item &item);

    void update_particles() {
        if(state != Running && state != Failed) return;

        for(auto &p: particles){
            p.update();
        }
        particles.erase(std::remove_if(particles.begin(), particles.end(), [](SpaceDust particle) { return (particle.age >= P_MAX_AGE); }), particles.end());
    };

    void reset_score() {
        score = 0;
        multiplier = 1;
    }

    void explode_all_dots() {
        for(auto &dot : dots) {
            chain.push_back(&dot);
        }
        explode_chain();
    }

    void update_dots() {
        if(state != Running) return;

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
            for(auto y = 0u; y < game_grid.h; y++) {
                for(auto x = 0u; x < game_grid.w; x++) {
                    game_state[x][y] = nullptr;
                }
            }
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

    void update_input(bool refill_destroyed_dots=true) {
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

    void update_menus(uint32_t time) {
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

    GameState get_state() {return state;}
    void set_state(GameState new_state) {state = new_state;}
    void pause() {state = Paused;}
    void resume() {state = Running;}

    void prev_level() {
        if(current_level > 0) current_level--;
    }

    void next_level() {
        if(current_level < max_level) current_level++;
    }

    void set_level(uint32_t current_level, uint32_t max_level) {
        this->current_level = current_level;
        this->max_level = max_level;
    }

    uint32_t get_level() {
        return current_level;
    }

    void add_dot(Dot dot) {
        dots.push_back(dot);
    }

private:

    uint32_t current_level;
    uint32_t max_level;

    GameState state = Running;

    Game *game;

    Menu pause_menu, end_menu;

    std::vector<Dot *> chain;
    std::vector<Dot> dots;
    std::vector<SpaceDust> particles;

    blit::Point global_dot_offset;

    uint32_t score = 0;
    uint32_t multiplier = 1;

    blit::Point selected;

    int32_t brightness = 255;

    bool needs_update = false;

    Dot* game_state[game_grid.w][game_grid.h];

    Dot *dot_at(blit::Point position) {
        for(auto &dot : dots) {
            if(position == dot.grid_location) {
                return &dot;
            }
        }
        return nullptr;
    }

    void set_backlight(uint8_t brightness) {
#ifdef PICO_BOARD
        // gamma correct the provided 0-255 brightness value onto a
        // 0-65535 range for the pwm counter
        float gamma = 2.8;
        uint16_t value = (uint16_t)(pow((float)(brightness) / 255.0f, gamma) * 65535.0f + 0.5f);
        pwm_set_gpio_level(PICOSYSTEM_BACKLIGHT_PIN, value);
#endif
    }

    bool grid_empty() {
        for(auto x = 0; x < game_grid.w; x++) {
            for(auto y = 0; y < game_grid.h; y++) {
                auto curr = game_state[x][y];
                if(curr) return false;
            }
        }
        return true;
    }

    bool move_available() {
        /* This code is going to be truly bad, but broadly the simplest check seems to be the most sensible approach.

        A game has an available move if any two dots of the same colour are adjacent. Therefore we can scan through the dots
        a row and/or column at a time and return True if there are adjacent matches*/

        // Do X first since Y dots are contiguous and this *may* be faster...
        for(auto x = 0; x < game_grid.w; x++) {
            auto prev = game_state[x][0];
            for(auto y = 1; y < game_grid.h; y++) {
                auto curr = game_state[x][y];
                // TODO maybe internally colour should be an int lookup
                if(curr && prev && curr->colour == prev->colour) return true;
                prev = curr;
            }
        }

        for(auto y = 0; y < game_grid.h; y++) {
            auto prev = game_state[0][y];
            for(auto x = 1; x < game_grid.w; x++) {
                auto curr = game_state[x][y];
                // TODO maybe internally colour should be an int lookup
                if(curr && prev && curr->colour == prev->colour) return true;
                prev = curr;
            }
        }

        return false;
    }

    bool adjacent(Dot *a, Dot *b) {
        blit::Point a_pos = a->grid_location;
        blit::Point b_pos = b->grid_location;
        int ax = a_pos.x - b_pos.x;
        int ay = a_pos.y - b_pos.y;
        if((ax == -1 || ax == 1) && ay == 0) return true;
        if((ay == -1 || ay == 1) && ax == 0) return true;
        return false;
    }

    void refill_dots(uint8_t *column_counts);
    void explode_chain(bool refill=false);
    void explode(blit::Point origin, blit::Pen colour, float factor=1.0f);
};