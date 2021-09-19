#pragma once
#include "32blit.hpp"

const uint8_t P_MAX_AGE = 255;

struct SpaceDust {
    blit::Vec2 pos;
    blit::Vec2 vel;
    uint32_t age = 0;
    blit::Pen color;

    SpaceDust(blit::Vec2 pos, blit::Vec2 vel, blit::Pen color) : pos(pos), vel(vel), color(color) {};
};