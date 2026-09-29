#pragma once
// Vai trò: hàm toán nhỏ dùng chung, như giới hạn giá trị và lấy dấu tốc độ.
#include <math.h>
#include <stdint.h>

constexpr float LF_TWO_PI = 6.28318531f;

static inline float clampf(float x, float lo, float hi) {
    return x < lo ? lo : (x > hi ? hi : x);
}

static inline int8_t sgnf(float x) {
    return (int8_t)((x > 0.0f) - (x < 0.0f));
}
