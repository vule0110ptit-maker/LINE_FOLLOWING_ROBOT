#pragma once
#include <math.h>
#include <stdint.h>

constexpr float LF_TWO_PI = 6.28318531f;

static inline float clampf(float x, float lo, float hi) {
    return x < lo ? lo : (x > hi ? hi : x);
}

static inline int8_t sgnf(float x) {
    return (int8_t)((x > 0.0f) - (x < 0.0f));
}