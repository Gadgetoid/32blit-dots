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
        Menu_Quit_Level,
        Menu_Start,
        Menu_Restart,
        Menu_Quit
    };

    void render() override;
    virtual void restart();
    virtual void render_level_select(PauseMenuItem id, bool in_menu, blit::Rect level_select_rect);
    virtual void on_won();
    virtual void on_failed();
    virtual void quit();

    virtual void render_won();
    virtual void render_failed();
    virtual void load_level();

    void on_menu_rendered(const ::Menu::Item &item);
    void on_menu_updated(const ::Menu::Item &item);
    void on_menu_activated(const Menu::Item &item);

    void update_particles();

    void reset_score() {
        score = 0;
        multiplier = 1;
    }

    void explode_all_dots();

    void update_dots();

    void clear_particles() {particles.clear();}

    void clear_dots() {dots.clear();}

    void update_input(bool refill_destroyed_dots=true);

    void update_menus(uint32_t time);

    GameState get_state() {return state;}
    void set_state(GameState new_state) {state = new_state;}
    void pause() {state = Paused;}
    void resume() {state = Running;}

    void prev_level() {
        if(current_level > 0) {
            current_level--;
            load_level();
        }
    }

    void next_level() {
        if(current_level < max_level) {
            current_level++;
            load_level();
        }
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

    void clear_game_state() {
        // Reset the game state to empty
        for(auto y = 0; y < game_grid.h; y++) {
            for(auto x = 0; x < game_grid.w; x++) {
                game_state[x][y] = nullptr;
            }
        }
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

    void refill_dots(const uint8_t *column_counts);
    void explode_chain(bool refill=false);
    void explode(blit::Point origin, blit::Pen colour);
};