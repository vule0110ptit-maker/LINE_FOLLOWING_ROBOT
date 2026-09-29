#include "line_process.h"
#include "robot_setup.h"
#include "parameters.h"
#include <math.h>

namespace {
int8_t s_lastSide = 1;
float s_lastPosition = 0.0f;
// Ghi nhớ cảm biến nào đang thấy line để dùng hai ngưỡng bật/tắt (hysteresis).
uint8_t s_activeMask = 0;
static_assert(LINE_OFF_THRESHOLD < LINE_ON_THRESHOLD, "Line hysteresis thresholds invalid");
}

void line_init() {
    s_lastSide = 1;
    s_lastPosition = 0.0f;
    s_activeMask = 0;
}

void line_process(const uint16_t *norm, LineData *out) {
    uint32_t total  = 0;
    uint8_t  active = 0;
    uint8_t  segments = 0;
    uint8_t  selectedStart = SENSOR_COUNT;
    uint8_t  selectedEnd = SENSOR_COUNT;
    float bestDistance = 3.0f;
    uint8_t nextMask = 0;

    // Đã thấy line thì chỉ tắt khi giảm dưới LINE_OFF_THRESHOLD, tránh chớp tắt do nhiễu.
    for (uint8_t i = 0; i < SENSOR_COUNT; ++i) {
        const bool wasOn = (s_activeMask & (1u << i)) != 0;
        if (norm[i] >= LINE_ON_THRESHOLD || (wasOn && norm[i] >= LINE_OFF_THRESHOLD))
            nextMask |= (1u << i);
    }
    s_activeMask = nextMask;

    // Chia các cảm biến trên line thành các vùng liên tiếp. Khi có hai line,
    // không lấy trung bình cả hai vì điểm trung bình có thể nằm trên nền trắng.
    for (uint8_t i = 0; i < SENSOR_COUNT; i++) {
        if (norm[i] >= SENSOR_NOISE_FLOOR) total += norm[i];
        if (!(nextMask & (1u << i))) continue;
        const uint8_t start = i;
        do { ++active; ++i; } while (i < SENSOR_COUNT && (nextMask & (1u << i)));
        const uint8_t end = i - 1;
        ++segments;

        const float center = (float)((int)start + (int)end - (int)(SENSOR_COUNT - 1)) /
                             (float)(SENSOR_COUNT - 1);
        const float distance = fabsf(center - s_lastPosition);
        bool choose = false;
        // Keep chọn vùng gần line trước đó; quét từ trái sang nên hòa điểm sẽ chọn trái.
        switch (BRANCH_CHOICE) {
            case BranchChoice::Left: choose = selectedStart == SENSOR_COUNT || start < selectedStart; break;
            case BranchChoice::Right: choose = selectedStart == SENSOR_COUNT || start > selectedStart; break;
            case BranchChoice::Keep:
                choose = selectedStart == SENSOR_COUNT || distance < bestDistance;
                break;
        }
        if (choose) {
            selectedStart = start;
            selectedEnd = end;
            bestDistance = distance;
        }
        --i; // vòng for sẽ chuyển đến cảm biến đầu của vùng tiếp theo
    }

    out->active   = active;
    out->mask     = nextMask;
    out->segments = segments;
    out->crossing = (active >= CROSS_MIN_ACTIVE);
    out->found    = (selectedStart != SENSOR_COUNT && total >= LINE_FOUND_SUM_MIN);

    if (out->found) {
        // Chỉ tính trọng tâm của vùng được chọn, không tính cả nhánh bên cạnh.
        uint32_t sum = 0;
        int32_t wsum = 0;
        for (uint8_t i = selectedStart; i <= selectedEnd; ++i) {
            const uint32_t v = norm[i];
            sum += v;
            wsum += (int32_t)v * (2 * (int32_t)i - (int32_t)(SENSOR_COUNT - 1));
        }
        const float pos = (float)wsum / ((float)sum * (float)(SENSOR_COUNT - 1));
        out->position = pos;
        s_lastPosition = pos;
        if (pos > LAST_SIDE_MIN)       s_lastSide = 1;
        else if (pos < -LAST_SIDE_MIN) s_lastSide = -1;
    } else {
        out->position = (float)s_lastSide;   // mất line: kéo hết cỡ về phía cuối cùng thấy line
    }
    out->lastSide = s_lastSide;
}
