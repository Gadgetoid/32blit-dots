#include <cinttypes>

#include "normal.hpp"
#include "game.hpp"
#include "main-menu.hpp"
#include "colours.hpp"


void Normal::update(uint32_t time) {
    update_menus(time);

    // Update the dots game state here
    update_particles();
    update_dots();
    update_input();
}