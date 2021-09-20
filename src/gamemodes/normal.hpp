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

class Normal final : public Stage {
public:
    Normal(Game *game);
    ~Normal() override;

    void update(uint32_t time) override;
    void render() override;
    void restart();

    void on_menu_rendered(const ::Menu::Item &item);
    void on_menu_updated(const ::Menu::Item &item);
    void on_menu_activated(const Menu::Item &item);

private:
    enum PauseMenuItem {
        Menu_Continue = 0,
        Menu_Change_Seed,
        Menu_Change_Brightness,
        Menu_Restart,
        Menu_Quit
    };

    enum GameState {
        Running = 0,
        Paused = 1,
        Failed = 2
    };

    uint32_t current_random_seed = 0x00750075;

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