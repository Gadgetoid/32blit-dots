#include "engine/engine.hpp"

#include "game.hpp"

#include "main-menu.hpp"

#include "system.hpp"

using namespace blit;

Game::Game() {
}

Game::~Game() {
    delete state;
    delete next_state;
}

void Game::update(uint32_t time) {
    if(state)
        state->update(time);

    // change state if requested
    if(next_state) {
        delete state;
        state = next_state;
        next_state = nullptr;
    }
}

void Game::render() {
    if(state)
        state->render();
}

static Game game;

void init() {
    set_screen_mode(ScreenMode::hires);

    channels[2].waveforms = SQUARE;
    channels[2].volume = 0x8000;
    channels[2].sustain = 0;
    channels[2].decay_ms = 20;
    channels[2].release_ms = 20;

    game.change_state<MainMenu>(true);

    picosystem.init();
}

void render(uint32_t time) {
    game.render();
    picosystem.render();
}

void update(uint32_t time) {
    game.update(time);
}