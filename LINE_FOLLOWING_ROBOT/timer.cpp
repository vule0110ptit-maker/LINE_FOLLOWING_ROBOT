#include <Arduino.h>
#include "timer.h"

namespace {
hw_timer_t       *s_timer    = nullptr;
volatile uint32_t s_ticks    = 0;
uint32_t          s_seen     = 0;
uint32_t          s_overruns = 0;

void IRAM_ATTR onTimer() { s_ticks = s_ticks + 1; }   // ISR chỉ đếm, không tính toán float
}

void timer_init(uint32_t periodUs) {
    s_timer = timerBegin(1000000);              // 1 MHz
    timerAttachInterrupt(s_timer, &onTimer);
    timerAlarm(s_timer, periodUs, true, 0);     // tự nạp lại
}

bool timer_tick() {
    const uint32_t t = s_ticks;
    if (t == s_seen) return false;
    s_overruns += (t - s_seen - 1);
    s_seen = t;
    return true;
}

uint32_t timer_overruns() { return s_overruns; }