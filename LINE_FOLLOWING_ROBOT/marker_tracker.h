#pragma once
// Vai trò: nhận mốc vạch ngang theo mẫu đã chọn và đếm số vòng hợp lệ.
#include <stdint.h>
#include "parameters.h"

struct MarkerTracker {
    uint32_t startMs;
    uint32_t lastLapMs;
    uint32_t wideSinceMs;
    uint32_t clearSinceMs;
    uint32_t firstBarMs;
    uint8_t laps;
    bool armed;
    bool inWide;
    bool confirmed;
    bool hasFirstBar;
};

void marker_reset(MarkerTracker *m, uint32_t now);
// Trả về true đúng một lần cho mỗi vòng hợp lệ.
bool marker_update(MarkerTracker *m, bool wide, uint32_t now, MarkerPattern pattern);
