#include "control/lowpassfilter.h"
#include "utils/math_utils.h"

void lp_init(LowPass *f, float cutoffHz, float sampleHz) {
    const float dt = 1.0f / sampleHz;
    const float rc = 1.0f / (LF_TWO_PI * cutoffHz);
    f->alpha = dt / (rc + dt);
    lp_reset(f);
}

void lp_reset(LowPass *f) {
    f->y = 0.0f;
    f->primed = false;
}