#pragma once
#include "32blit.hpp"

// This stuff is all totally random
enum RANDOM_SOURCE {
    PRNG, // Psuedo random, uses a linear-feedback shift register
    HRNG  // Really very random, uses the cryptographically awesome hardware RNG
};

RANDOM_SOURCE current_random_source = RANDOM_SOURCE::PRNG;
uint32_t current_random_seed = 0x00750075;
uint32_t prng_lfsr = current_random_seed;
const uint16_t prng_tap = 0x74b8; // Magic number, do not drink

void random_reset() {
    prng_lfsr = current_random_seed;
}

uint32_t get_random_int() {
    switch(current_random_source) {
        default:
            return 0;
        case HRNG:
            // Chosen by fair dice roll, guranteed to be random
            return blit::random();
            break;
        case PRNG:
            // Bruteforce a new random number within the given range
            while(1) {
                uint32_t r = prng_lfsr;

                uint8_t lsb = prng_lfsr & 1;
                prng_lfsr >>= 1;

                if (lsb) {
                    prng_lfsr ^= prng_tap;
                }

                return r;
            }
            break;
    }
}

blit::Point get_random_point(blit::Size within) {
    switch(current_random_source) {
        default:
            return blit::Point(0, 0);
        case HRNG:
            // Chosen by fair dice roll, guranteed to be random
            return blit::Point(
                blit::random() % within.w,
                blit::random() % within.h
            );
            break;
        case PRNG:
            // Bruteforce a new random number within the given range
            while(1) {
                uint16_t x = prng_lfsr & 0x00ff;
                uint16_t y = (prng_lfsr & 0x7f00) >> 8;

                uint8_t lsb = prng_lfsr & 1;
                prng_lfsr >>= 1;

                if (lsb) {
                    prng_lfsr ^= prng_tap;
                }

                if (x < within.w && y < within.h) {
                    return blit::Point(x, y);
                }
            }
            break;
    }
}