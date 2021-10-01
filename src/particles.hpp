#pragma once
#include "32blit.hpp"

const uint8_t P_MAX_AGE = 255;

struct SpaceDust {
    blit::Point pos;
    blit::Point vel;
    uint32_t age = 0;
    blit::Pen color;

    SpaceDust(blit::Point pos, blit::Pen color) : pos(pos), color(color) {
        float r = blit::random() % 360;
        // we'd normally /100 here
        // but the fixed 24-8 value must be 255x bigger
        float v = (50.0f + (blit::random() % 100));
        r  = r * blit::pi / 180.0f;
        vel = blit::Vec2(cosf(r) * v, sinf(r) * v);
    };

    blit::Point screen_position() {
        // Convert internal position back
        return blit::Point(pos.x >> 8, pos.y >> 8);
    };

    void set_velocity(blit::Vec2 velocity) {
        vel.x = velocity.x * 255;
        vel.y = velocity.y * 255;
    }

    void update() {
        age += 2;
        vel += blit::Point(0, 3); // 24-8 fixed "gravity"
        pos += vel;
    }

    void render(const blit::Point offset) {
        blit::screen.pen = color;
        blit::screen.alpha = P_MAX_AGE - (uint8_t)age;
        blit::Point ppos = blit::Point(pos.x >> 8, pos.y >> 8);
        blit::screen.circle(ppos + offset, 3);
    }
};