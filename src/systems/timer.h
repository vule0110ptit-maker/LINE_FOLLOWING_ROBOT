#pragma once
#include <stdint.h>

void     timer_init(uint32_t periodUs);
bool     timer_tick();        // true nếu có tick mới (ISR chỉ tăng bộ đếm)
uint32_t timer_overruns();    // số tick bị lỡ do vòng điều khiển chạy quá lâu