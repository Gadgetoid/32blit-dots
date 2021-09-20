#pragma once
#include "32blit.hpp"
#include "random.hpp"

const blit::Pen DOT_COLOURS[5] = {
    blit::Pen(0x99, 0x00, 0xcc), // Purple
    blit::Pen(0x00, 0xcc, 0xff), // Blue
    blit::Pen(0x00, 0x99, 0x99), // Green
    blit::Pen(0xff, 0x33, 0x33), // Red
    blit::Pen(0xff, 0x99, 0x33)  // Yellow
};

const blit::Pen DOT_COLOURS_SELECTED[5] = {
    blit::Pen(0xBB, 0x22, 0xEE), // Purple
    blit::Pen(0x22, 0xEE, 0xff), // Blue
    blit::Pen(0x22, 0xBB, 0xBB), // Green
    blit::Pen(0xff, 0x55, 0x55), // Red
    blit::Pen(0xff, 0xBB, 0x55)  // Yellow
};

const uint8_t dot_radius = 10;
constexpr blit::Size game_grid(6, 6);
constexpr blit::Size game_bounds(240 - 24 - 24, 240 - 24 - 24);
constexpr blit::Point dot_spacing = blit::Point(game_bounds.w / game_grid.w, game_bounds.h / game_grid.h);

struct Dot {
    blit::Point position;
    blit::Point grid_location;
    blit::Pen colour;
    blit::Pen selected_colour;
    bool explode;

    Dot (blit::Point p) {
        position = p * 256;
        explode = false;
        uint8_t c = get_random_int() % 5;
        colour = DOT_COLOURS[c];
        selected_colour = DOT_COLOURS_SELECTED[c];
        update_location();
    };

    blit::Point screen_location() {
        return blit::Point(
            (position.x * dot_spacing.x) >> 8,
            (position.y * dot_spacing.y) >> 8
        );
    }

    void update_location() {
        grid_location = blit::Point(position.x >> 8, position.y >> 8);
    }

    int grid_offset() {
        return grid_location.x + grid_location.y * game_grid.w;
    }

    bool in(std::vector<Dot *> chain) {
        for(auto &dot : chain) {
            if(dot == this) return true;
        }
        return false;
    }
    
	bool operator < (const Dot& rhs) const {
        blit::Point a = grid_location;
        blit::Point b = rhs.grid_location;
        uint32_t offset_a = a.x + a.y * game_grid.w;
        uint32_t offset_b = b.x + b.y * game_grid.w;
        return offset_a > offset_b;
    };
};