#pragma once
#include <stdbool.h>
#include "lowpassfilter.h"

// Bộ điều khiển PD (không có I)
typedef struct {
    float   kp, kd;
    float   outLimit;
    float   sampleHz;     // = 1/dt, tính sẵn để khỏi chia mỗi tick
    float   prevErr;
    bool    primed;
    LowPass dFilter;      // lowpass cho đạo hàm
} PD;

void  pd_init(PD *p, float kp, float kd, float dCutoffHz, float sampleHz, float outLimit);
void  pd_reset(PD *p);
float pd_update(PD *p, float error);