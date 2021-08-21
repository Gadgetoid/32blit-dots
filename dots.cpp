#include "dots.hpp"

using namespace blit;

constexpr Pen COLOUR_SELECTED(128, 128, 128);
constexpr Pen COLOUR_BACKGROUND(255, 255, 255);
constexpr Pen COLOUR_DOT_NONE(0, 0, 0);
constexpr Pen COLOUR_LINE(0, 0, 0);
const uint8_t P_MAX_AGE = 255;

constexpr Size game_grid(6, 6);
constexpr Size game_bounds(220, 220);
constexpr Vec2 dot_spacing = Vec2(game_bounds.w / game_grid.w, game_bounds.h / game_grid.h);
Vec2 global_dot_offset;

Pen DOT_COLOURS[5] = {
    Pen(0x99, 0x00, 0xcc), // Purple
    Pen(0x00, 0xcc, 0xff), // Blue
    Pen(0x00, 0x99, 0x99), // Green
    Pen(0xff, 0x33, 0x33), // Red
    Pen(0xff, 0x99, 0x33)  // Yellow
};

struct Dot {
    Vec2 position;
    Pen colour;
    bool explode;

    Dot (Vec2 position) : position(position) {
        explode = false;
        colour = DOT_COLOURS[blit::random() % 5];
    };

    Point grid_location() {
        return position / dot_spacing;
    }

    bool in(std::vector<Dot *> chain) {
        for(auto &dot : chain) {
            if(dot == this) return true;
        }
        return false;
    }
};

struct SpaceDust {
    blit::Vec2 pos;
    blit::Vec2 vel;
    uint32_t age = 0;
    Pen color;

    SpaceDust(blit::Vec2 pos, blit::Vec2 vel, Pen color) : pos(pos), vel(vel), color(color) {};
};

std::vector<Dot *> chain;
std::vector<Dot> dots;
std::vector<SpaceDust> particles;

Point selected(0, 0);

Dot *dot_at(Point position) {
    for(auto &dot : dots) {
        if(position == dot.grid_location()) {
            return &dot;
        }
    }
    return nullptr;
}

bool adjacent(Dot *a, Dot *b) {
    Point a_pos = a->grid_location();
    Point b_pos = b->grid_location();
    int ax = a_pos.x - b_pos.x;
    int ay = a_pos.y - b_pos.y;
    if((ax == -1 || ax == 1) && ay == 0) return true;
    if((ay == -1 || ay == 1) && ax == 0) return true;
    return false;
}

void init() {
    set_screen_mode(ScreenMode::hires);

    global_dot_offset = Vec2(
        (screen.bounds.w - game_bounds.w) / 2,
        (screen.bounds.h - game_bounds.h) / 2
    ) + (dot_spacing / 2.0f);

    for(auto y = 0u; y < game_grid.h; y++) {
        for(auto x = 0u; x < game_grid.w; x++) {
            dots.push_back(Dot(Vec2(x, y) * dot_spacing));
        }
    }
}

void render(uint32_t time) {
    screen.pen = COLOUR_BACKGROUND;
    screen.clear();

    // Draw the cursor
    Vec2 selected_origin = global_dot_offset + Vec2(selected) * dot_spacing;
    screen.pen = COLOUR_SELECTED;
    screen.circle(selected_origin, 15);
    screen.pen = COLOUR_BACKGROUND;
    screen.circle(selected_origin, 13);

    for(auto &dot : dots) {
        if(dot.explode) continue;
    
        Pen c = dot.colour;
        Vec2 dot_origin = global_dot_offset + dot.position;
    
        if(Point(dot.position) == selected) {
            c = Pen(
                (uint8_t)std::min(255.0f, c.r * 1.2f),
                (uint8_t)std::min(255.0f, c.g * 1.2f),
                (uint8_t)std::min(255.0f, c.b * 1.2f)
            );
        }
        screen.pen = c;
        screen.circle(dot_origin, 12);
    };

    Point last(-1, -1);
    for(auto &dot : chain) {
        Vec2 dot_origin = global_dot_offset + dot->position; 

        screen.pen = COLOUR_LINE;
        screen.circle(dot_origin, 5);

        if(last != Point(-1, -1)) {
            screen.pen = COLOUR_LINE;
            screen.line(last, dot->position);
            last = dot->position;
        }
    };

    for(auto &p: particles){
        screen.pen = p.color;
        screen.alpha = P_MAX_AGE - (uint8_t)p.age;
        p.age += 2;
        p.pos += p.vel;
        screen.circle(p.pos + global_dot_offset, 3);
    }
    screen.alpha = 255;

    particles.erase(std::remove_if(particles.begin(), particles.end(), [](SpaceDust particle) { return (particle.age >= P_MAX_AGE); }), particles.end());
}

void explode(Vec2 origin, Pen colour, float factor=1.0f) {
    channels[2].frequency = 800;
    channels[2].trigger_attack();
    uint8_t count = 5 + (blit::random() % 5);
    count *= factor;
    for(auto x = 0u; x < count; x++) {
        float r = blit::random() % 360;
        float v = (50.0f + (blit::random() % 100)) / 100.0f;
        r  = r * pi / 180.0f;

        particles.push_back(SpaceDust(
                origin,
                Vec2(cosf(r) * v, sinf(r) * v),
                colour
        ));
    }
}

void explode_chain() {
    for(auto &dot : chain) {
        dot->explode = true;
        explode(dot->position, dot->colour);
    }
}

void update(uint32_t time) {
    Point movement(0, 0);
    static Point last_movement(0, 0);

    uint8_t column_counts[game_grid.w] = {0, 0, 0, 0, 0, 0};

    for(auto &dot : dots) {
        Point position = dot.grid_location();
        if(position.y == game_grid.h - 1) continue; // Easy way to collide with the "floor"
        auto dot_below = dot_at(position + Point(0, 1));
        if(!dot_below) {
            dot.position.y += dot_spacing.y / 12.0f;
        }
        column_counts[position.x] += 1;
    }

    for(auto x = 0u; x < game_grid.w; x++) {
        uint8_t count = column_counts[x];
        if(count < game_grid.h - 1 && !dot_at(Point(x, 0)) && !dot_at(Point(x, 0))) {
            dots.push_back(Dot(Vec2(x, -1) * dot_spacing));
        }
    }

    if(buttons.pressed & Button::DPAD_RIGHT) {
        movement.x = 1;
    }else if(buttons.pressed & Button::DPAD_LEFT) {
        movement.x = -1;
    }
    if(!game_grid.contains(selected + movement)) {
        movement.x = 0;
    }
    if(buttons.pressed & Button::DPAD_DOWN) {
        movement.y = 1;
    }else if(buttons.pressed & Button::DPAD_UP) {
        movement.y = -1;
    }
    if(!game_grid.contains(selected + movement)) {
        movement.y = 0;
    }
    selected += movement;

    if(buttons & Button::A) {
        auto dot = dot_at(selected);
        if(dot) {
            if(chain.size() == 0) {
                chain.push_back(dot);
            } else if(dot->colour == chain.front()->colour && adjacent(dot, chain.back())) {
                if(!dot->in(chain)) {
                    chain.push_back(dot);
                } else {
                    // Dot is in chain already
                    if(dot == chain[chain.size() - 2]) {
                        chain.pop_back();
                    } else {
                        selected -= movement;
                    }
                }
            } else {
                selected -= movement;
            }
        } else {
            selected -= movement;
        }
    } else {
        if(chain.size() > 1) {
            // DOTS GO BOOM
            explode_chain();
            dots.erase(std::remove_if(dots.begin(), dots.end(), [](Dot dot){return dot.explode;}), dots.end());
        }
        chain.clear();
    }

    last_movement = movement;
}