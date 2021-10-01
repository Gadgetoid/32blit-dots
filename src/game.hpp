#pragma once

#include <cstdint>

#include "stage.hpp"

class Game final {
public:
    Game();
    ~Game();

    void update(uint32_t time);
    void render();

    template <class T, typename ...Args>
    void change_state(Args ...args);

private:
    // state of the game
    Stage *state = nullptr, *next_state = nullptr;
};

template<class T, typename ...Args>
void Game::change_state(Args ...args) {
    // clean up pending state
    if(next_state)
        delete next_state;

    next_state = new T(this, args...);

    if(!state) // start immediately if no current state
        std::swap(state, next_state);
}