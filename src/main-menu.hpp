#pragma once

#include "menu.hpp"
#include "stage.hpp"
#include "fonts.hpp"

class Game;

class MainMenu final : public Stage {
public:
    MainMenu(Game *game, bool initial_state = false);
    ~MainMenu() override;

    void update(uint32_t time) override;
    void render() override;

private:
    enum MainMenuItem {
        Menu_Normal = 0,
        Menu_Puzzle,
        Menu_Seed
    };

    void on_menu_item_selected(const Menu::Item &item);

    Game *game;

    Menu menu;
    MainMenuItem selected = Menu_Normal;
    std::string message;
    uint32_t current_random_seed;
};