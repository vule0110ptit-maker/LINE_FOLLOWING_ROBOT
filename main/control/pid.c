#include "control/pid.h"
#include "utils/math_utils.h"

void pd_init(PD *p, float kp, float kd, float dCutoffHz, float sampleHz, float outLimit) {
    p->kp = kp;
    p->kd = kd;
    p->outLimit = outLimit;
    p->sampleHz = sampleHz;
    lp_init(&p->dFilter, dCutoffHz, sampleHz);
    pd_reset(p);
}

void pd_reset(PD *p) {
    p->prevErr = 0.0f;
    p->primed = false;
    lp_reset(&p->dFilter);
}

float pd_update(PD *p, float error) {
    float d = 0.0f;
    if (p->primed) d = (error - p->prevErr) * p->sampleHz;
    p->prevErr = error;
    p->primed  = true;

    d = lp_update(&p->dFilter, d);
    return clampf(p->kp * error + p->kd * d, -p->outLimit, p->outLimit);
}