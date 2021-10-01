#pragma once

#include <cstdint>
#include <cmath>
#include "32blit.hpp"

#ifdef PICO_BOARD
#include "hardware/pwm.h"
#endif

const uint32_t SAVE_HEADER = 0x444f5453;

class System {
public:

    struct SaveData {
        uint32_t header;
        int32_t brightness;
    };

    SaveData savedata;

    uint16_t value = 65535;
    uint16_t target_value = 0;

    void init() {
        if(!blit::read_save(savedata) || savedata.header != SAVE_HEADER) {
            savedata.header = SAVE_HEADER;
            savedata.brightness = 255;
            blit::write_save(savedata);
        }
        set_backlight(savedata.brightness);
    }

    void backlight_down() {
        savedata.brightness -= 16;
        if(savedata.brightness < 0) savedata.brightness = 0;
        set_backlight((uint8_t)savedata.brightness);
        blit::write_save(savedata);
    }

    void backlight_up() {
        savedata.brightness += 16;
        if(savedata.brightness > 255) savedata.brightness = 255;
        set_backlight((uint8_t)savedata.brightness);
        blit::write_save(savedata);
    }

    uint8_t get_brightness() {
        return (uint8_t)savedata.brightness;
    }

    void render() {
        if(value < target_value) value += (target_value - value) / 10;
        if(value > target_value) value -= (value - target_value) / 10;
#ifdef PICO_BOARD
        pwm_set_gpio_level(PICOSYSTEM_BACKLIGHT_PIN, value);
#endif
    }

    void set_backlight(uint8_t brightness) {
        // gamma correct the provided 0-255 brightness value onto a
        // 0-65535 range for the pwm counter
        float gamma = 2.8;
        target_value = (uint16_t)(powf((float)(brightness) / 255.0f, gamma) * 65535.0f + 0.5f);
    }
};

extern System picosystem;