#pragma once

#include <cstdint>

class Stage {
    public:
        virtual ~Stage(){}

        virtual void update(uint32_t time) = 0;
        virtual void render() = 0;

        int32_t menu_target_width = 90;
};