#pragma once
#include "32blit.hpp"

// This stuff is all totally random
enum RANDOM_SOURCE {
    PRNG, // Psuedo random, uses a linear-feedback shift register
    HRNG  // Really very random, uses the cryptographically awesome hardware RNG
};

const uint16_t prng_tap = 0x74b8; // Magic number, do not drink

void random_reset(uint32_t current_random_seed);

uint32_t get_random_int();

blit::Point get_random_point(blit::Size within);