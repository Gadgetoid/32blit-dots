#include <cinttypes>
#include "main-menu.hpp"
#include "game.hpp"
#include "normal.hpp"
#include "puzzle.hpp"

MainMenu::MainMenu(Game *game, bool initial_state) : game(game), menu("", 
    {{Menu_Normal, "Normal"},
    {Menu_Puzzle, "Puzzle"}},
    outline_font_10x14) {

    menu.set_display_rect({blit::screen.bounds.w - menu_target_width, 0, menu_target_width, blit::screen.bounds.h});

    // TODO: nicer menu, maybe some sprites or something

    menu.set_on_item_activated(std::bind(&MainMenu::on_menu_item_selected, this, std::placeholders::_1));
}

MainMenu::~MainMenu() {
}

void MainMenu::update(uint32_t time) {
    menu.update(time);
}

void MainMenu::render() {
    using blit::screen;

    screen.pen = {0x63, 0x9b, 0xff}; // "sky" colour
    screen.clear();

    menu.render();

    if(!message.empty())
        screen.text(
            message,
            outline_font_10x14,
            {screen.bounds.w / 2, screen.bounds.h - 20},
            true,
            blit::TextAlign::center_center);

}

void MainMenu::on_menu_item_selected(const ::Menu::Item &item) {
    switch(item.id) {
        case Menu_Normal:
            game->change_state<Normal>();
            break;
        case Menu_Puzzle:
            game->change_state<Puzzle>();
            break;
    }
}
