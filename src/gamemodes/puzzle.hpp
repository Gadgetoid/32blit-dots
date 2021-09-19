#pragma once

#include <cstdint>
#include "fonts.hpp"
#include "stage.hpp"
#include "menu.hpp"

class Game;

class Puzzle final : public Stage {
public:
    Puzzle(Game *game);
    ~Puzzle() override;

    void update(uint32_t time) override;
    void render() override;

private:
    enum PauseMenuItem {
        Menu_Continue = 0,
        Menu_Restart,
        Menu_Quit
    }
}