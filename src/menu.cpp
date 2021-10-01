#include "menu.hpp"

#include "engine/engine.hpp"

Menu::Menu(std::string_view title, std::vector<Item> items, const blit::Font &font) : blit::Menu(title, nullptr, 0, font), items_vec(std::move(items)) {
    this->items = items_vec.data();
    num_items = items_vec.size();

    item_h = font.char_h + 10;
    item_padding_x = 9;
    item_adjust_y = 0;

    header_h = title.empty() ? 0 : item_h;
    footer_h = 0;
    margin_y = 0;

    background_colour = blit::Pen(0x11, 0x11, 0x11, 200);
    foreground_colour = blit::Pen(0xF7, 0xF7, 0xF7);
    selected_item_background = blit::Pen(0x22, 0x22, 0x22);
}

void Menu::add_item(Item &&item) {
    items_vec.emplace_back(std::move(item));

    items = items_vec.data();
    num_items = items_vec.size();
}

void Menu::set_on_item_activated(std::function<void(const Item &)> func) {
    on_item_pressed = func;
}

void Menu::set_on_item_rendered(std::function<void(const Item &)> func) {
    on_item_rendered = func;
}

void Menu::set_on_item_updated(std::function<void(const Item &)> func) {
    on_item_updated = func;
}

void Menu::update_item(const Item &item) {
    if(on_item_updated)
        on_item_updated(item);
}

void Menu::render_item(const Item &item, int y, int index) const {
    blit::Menu::render_item(item, y, index);

    if(index == current_item) {
        blit::screen.pen = foreground_colour;
        int x = display_rect.x + 2;
        int size = item_h / 3;
        int pointer_y = y + (item_h - size) / 2;
    
        blit::screen.triangle({x, pointer_y}, {x + size / 2, pointer_y + size / 2}, {x, pointer_y + size});
    }

    if(on_item_rendered)
        on_item_rendered(item);
}

void Menu::item_activated(const Item &item) {
    if(on_item_pressed)
        on_item_pressed(item);
}