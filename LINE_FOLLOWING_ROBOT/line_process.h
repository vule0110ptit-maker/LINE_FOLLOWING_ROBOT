#pragma once
// Vai trò: đổi 8 giá trị cảm biến thành bit line, vị trí lệch và số vùng line rời nhau.
#include <stdint.h>
#include <stdbool.h>

typedef struct {
    float   position;    // -1 (trái) .. +1 (phải)
    bool    found;       // có thấy line
    bool    crossing;    // gần hết cảm biến cùng thấy line (vạch ngang/đích)
    uint8_t active;      // số cảm biến trên ngưỡng
    uint8_t mask;        // bit i = 1 khi cảm biến i đang nhận line (đã qua hysteresis)
    uint8_t segments;    // số vùng line rời nhau trên dãy cảm biến
    int8_t  lastSide;    // phía cuối cùng thấy line: -1 trái, +1 phải
} LineData;

void line_init();
void line_process(const uint16_t *norm, LineData *out);
