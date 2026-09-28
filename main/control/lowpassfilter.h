#ifndef __LOWPASS__H
#define __LOWPASS__H

#pragma once
#include <stdint.h>
#include <stdbool.h>

// Lowpass bậc 1: y += alpha * (x - y)
typedef struct {
    float alpha;
    float y;
    bool  primed;
} LowPass;

void lp_init(LowPass *f, float cutoffHz, float sampleHz);
void lp_reset(LowPass *f);

static inline float lp_update(LowPass *f, float x) {
    if (!f->primed) {          // mẫu đầu tiên: khởi tạo, tránh giật
        f->y = x;
        f->primed = true;
        return x;
    }
    f->y += f->alpha * (x - f->y);
    return f->y;
}

#endif