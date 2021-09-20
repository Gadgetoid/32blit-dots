#include <cinttypes>
#include "main-menu.hpp"
#include "game.hpp"
#include "normal.hpp"
#include "puzzle.hpp"
#include "colours.hpp"

uint8_t maskdata[320 * 240];

MainMenu::MainMenu(Game *game, bool initial_state) : game(game), menu("", 
    {{Menu_Normal, "Normal"},
    {Menu_Puzzle, "Puzzle"}},
    outline_font_10x14) {

    mask = new blit::Surface(maskdata, blit::PixelFormat::M, blit::Size(blit::screen.bounds.w, blit::screen.bounds.h));

    menu.set_display_rect({blit::screen.bounds.w - menu_target_width, 0, menu_target_width, blit::screen.bounds.h});

    // TODO: nicer menu, maybe some sprites or something

    menu.set_on_item_activated(std::bind(&MainMenu::on_menu_item_selected, this, std::placeholders::_1));
}

MainMenu::~MainMenu() {
    delete mask;
}

void MainMenu::update(uint32_t time) {
    menu.update(time);
}

void MainMenu::render() {
    using blit::screen;

    screen.pen = blit::hsv_to_rgba((blit::now() + 5000.0f) / 10000.0f, 1.0f, 1.0f);
    screen.clear();

    for(auto y = 0; y < 16; y++) {
        mask->pen = 254 - y * 16;
        mask->text("Dots dots dots dots dots dots dots", outline_font_10x14, blit::Point(8, 8 + y * 15));
    }

    screen.pen = blit::hsv_to_rgba(blit::now() / 10000.0f, 1.0f, 1.0f);
    screen.mask = mask;
    screen.clear();
    screen.mask = nullptr;

    screen.alpha = 200;
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
