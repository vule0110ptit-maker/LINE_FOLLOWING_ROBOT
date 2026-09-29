#include "line_process.h"
#include "robot_setup.h"
#include "parameters.h"

namespace {
int8_t s_lastSide = 1;
}

void line_init() {
    s_lastSide = 1;
}

void line_process(const uint16_t *norm, LineData *out) {
    int32_t  wsum   = 0;
    uint32_t sum    = 0;
    uint8_t  active = 0;

    // Trọng số i: 2*i - (N-1)  ->  -(N-1) .. +(N-1), không cần bảng
    for (uint8_t i = 0; i < SENSOR_COUNT; i++) {
        uint32_t v = norm[i];
        if (v < SENSOR_NOISE_FLOOR) v = 0;
        if (v > LINE_ON_THRESHOLD) active++;
        sum  += v;
        wsum += (int32_t)v * (2 * (int32_t)i - (int32_t)(SENSOR_COUNT - 1));
    }

    out->active   = active;
    out->crossing = (active >= CROSS_MIN_ACTIVE);
    out->found    = (sum >= LINE_FOUND_SUM_MIN);

    if (out->found) {
        const float pos = (float)wsum / ((float)sum * (float)(SENSOR_COUNT - 1));
        out->position = pos;
        if (pos > LAST_SIDE_MIN)       s_lastSide = 1;
        else if (pos < -LAST_SIDE_MIN) s_lastSide = -1;
    } else {
        out->position = (float)s_lastSide;   // mất line: kéo hết cỡ về phía cuối cùng thấy line
    }
    out->lastSide = s_lastSide;
}