#include "dots.hpp"

using namespace blit;

constexpr Pen COLOUR_SELECTED(128, 128, 128);
constexpr Pen COLOUR_BACKGROUND(255, 255, 255);
constexpr Pen COLOUR_DOT_NONE(0, 0, 0);
constexpr Pen COLOUR_LINE(0, 0, 0);
const uint8_t P_MAX_AGE = 255;
const uint8_t dot_radius = 12;

constexpr Size game_grid(6, 6);
constexpr Size game_bounds(220, 220);
constexpr Vec2 dot_spacing = Vec2(game_bounds.w / game_grid.w, game_bounds.h / game_grid.h);
Vec2 global_dot_offset;

uint32_t score;


Pen DOT_COLOURS[5] = {
    Pen(0x99, 0x00, 0xcc), // Purple
    Pen(0x00, 0xcc, 0xff), // Blue
    Pen(0x00, 0x99, 0x99), // Green
    Pen(0xff, 0x33, 0x33), // Red
    Pen(0xff, 0x99, 0x33)  // Yellow
};

Pen DOT_COLOURS_SELECTED[5] = {
    Pen(0xBB, 0x22, 0xEE), // Purple
    Pen(0x22, 0xEE, 0xff), // Blue
    Pen(0x22, 0xBB, 0xBB), // Green
    Pen(0xff, 0x55, 0x55), // Red
    Pen(0xff, 0xBB, 0x55)  // Yellow
};

struct Dot {
    Vec2 position;
    Point grid_location;
    Pen colour;
    Pen selected_colour;
    bool explode;

    Dot (Vec2 p) {
        position = p;
        explode = false;
        uint8_t c = blit::random() % 5;
        colour = DOT_COLOURS[c];
        selected_colour = DOT_COLOURS_SELECTED[c];
        update_location();
    };

    void update_location() {
        Vec2 loc = position / dot_spacing;
        grid_location = Point(floor(loc.x), floor(loc.y));
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
    
	const bool operator < (const Dot& rhs) const {
        Point a = position / dot_spacing;
        Point b = rhs.position / dot_spacing;
        uint32_t offset_a = a.x + a.y * game_grid.w;
        uint32_t offset_b = b.x + b.y * game_grid.w;
        return offset_a > offset_b;
    };
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

Dot* game_state[game_grid.w][game_grid.h];

Dot *dot_at(Point position) {
    for(auto &dot : dots) {
        if(position == dot.grid_location) {
            return &dot;
        }
    }
    return nullptr;
}

bool adjacent(Dot *a, Dot *b) {
    Point a_pos = a->grid_location;
    Point b_pos = b->grid_location;
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
        (screen.bounds.h - game_bounds.h)
    ) + (dot_spacing / 2.0f);

    for(auto y = 0; y < game_grid.h; y++) {
        for(auto x = 0; x < game_grid.w; x++) {
            auto dot = Dot(Vec2(x, y) * dot_spacing);
            dots.emplace_back(dot);
            game_state[x][y] = &dot;
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
    
        Vec2 dot_origin = global_dot_offset + dot.position;
    
        if(dot.grid_location == selected || dot.in(chain)) {
            screen.pen = dot.selected_colour;
        } else {
            screen.pen = dot.colour;
        }
        screen.circle(dot_origin, dot_radius);
    };

    if(chain.size() > 0) {
        Dot *last = nullptr;
        Pen chain_colour = chain.front()->selected_colour;
        for(auto &dot : chain) {
            Vec2 dot_origin = global_dot_offset + dot->position; 

            //screen.pen = COLOUR_LINE;
            //screen.circle(dot_origin, 5);

            if(last) {
                screen.pen = chain_colour;
                Rect link(0, 0, 0, 0);
                if(last->position.y == dot->position.y) {
                    link.x = std::min(last->position.x, dot->position.x);
                    link.w = abs(last->position.x - dot->position.x);
                    link.h = 2 * dot_radius + 1; // Wtf?
                    link.y = last->position.y - dot_radius;

                }
                if(last->position.x == dot->position.x) {
                    link.y = std::min(last->position.y, dot->position.y);
                    link.h = abs(last->position.y - dot->position.y);
                    link.w = 2 * dot_radius + 1;
                    link.x = last->position.x - dot_radius;
                }
                link.x += global_dot_offset.x;
                link.y += global_dot_offset.y;
                screen.rectangle(link);
                last = dot;
            }

            last = dot;
        };
    }

    for(auto &p: particles){
        screen.pen = p.color;
        screen.alpha = P_MAX_AGE - (uint8_t)p.age;
        p.age += 2;
        p.vel += Vec2(0.0f, 0.098f);
        p.pos += p.vel;
        screen.circle(p.pos + global_dot_offset, 3);
    }
    screen.alpha = 255;

    particles.erase(std::remove_if(particles.begin(), particles.end(), [](SpaceDust particle) { return (particle.age >= P_MAX_AGE); }), particles.end());

    screen.pen = Pen(50, 50, 50);
    screen.rectangle(Rect(0, 0, screen.bounds.w, 20));
    screen.pen = Pen(200, 200, 200);
    screen.text(std::to_string(score), minimal_font, Point(global_dot_offset.x - dot_radius, 5));


    for(auto x = 0; x < game_grid.w; x++) {
        for(auto y = 0; y < game_grid.h; y++) {
            auto dot = game_state[x][y];
            screen.pen = dot ? dot->colour : Pen(0, 0, 0);
            screen.rectangle(Rect(Point(x * 4, y * 4 + 20), Size(3, 3)));
        }
    }
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
    uint8_t column_counts[game_grid.w] = {0, 0, 0, 0, 0, 0};

    if(chain.size() < 2) {
        chain.clear();
        return;
    };

    for(auto &dot : chain) {
        dot->explode = true;
        explode(dot->position, dot->colour);
        column_counts[dot->grid_location.x] += 1;
        game_state[dot->grid_location.x][dot->grid_location.y] = nullptr;
    }

    dots.erase(std::remove_if(dots.begin(), dots.end(), [](Dot dot){return dot.explode;}), dots.end());

    for(auto x = 0u; x < game_grid.w; x++) {
        uint8_t count = column_counts[x];
        while(count--) {
            dots.push_back(Dot(Vec2(x, -1 - count) * dot_spacing));
        }
    }

	std::sort(dots.begin(), dots.end());
    for(auto &dot : dots) {
        if(dot.grid_location.y < 0) continue;
        game_state[dot.grid_location.x][dot.grid_location.y] = &dot;
    }

    score += chain.size() * chain.size() * chain.size();

    chain.clear();
}

void update(uint32_t time) {
    Point movement(0, 0);
    static Point last_movement(0, 0);
    static bool needs_update = false;
    bool falling = false;

    for(auto &dot : dots) {
        if(dot.grid_location.y == game_grid.h - 1) continue; // Easy way to collide with the "floor"
        auto dot_below = dot_at(dot.grid_location + Point(0, 1));
        if(!dot_below) {
            dot.position.y += dot_spacing.y / float(dot_radius);
            dot.update_location();
            falling = true;
        }
    }

    if(!falling && needs_update) {
        for(auto &dot : dots) {
            if(dot.grid_location.y < 0) continue;
            game_state[dot.grid_location.x][dot.grid_location.y] = &dot;
        }
        needs_update = false;
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
        auto dot = game_state[selected.x][selected.y];
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
        explode_chain();
        needs_update = true;
    }

    last_movement = movement;
}