#include <Arduino.h>
#include "systems/control_task.h"
#include "systems/timer.h"
#include "statemachine/fsm.h"
#include "statemachine/states.h"
#include "drivers/sensor_ir.h"
#include "control/line_process.h"
#include "control/pid.h"
#include "control/lowpassfilter.h"
#include "config/pin.h"
#include "config/parameters.h"
#include "config/robot_setup.h"

static RobotContext ctx;

// Debounce nút, chỉ báo cạnh nhấn (1 tick)
static void button_update(RobotContext *c) {
    static uint8_t  stable = HIGH, last = HIGH;
    static uint32_t t0 = 0;

    const uint8_t  r   = digitalRead(PIN_BUTTON);
    const uint32_t now = millis();
    c->btnPressed = false;

    if (r != last) {
        last = r;
        t0 = now;
    } else if (r != stable && (now - t0) >= BTN_DEBOUNCE_MS) {
        stable = r;
        if (stable == LOW) c->btnPressed = true;
    }
}

bool control_task_init() {
    pinMode(PIN_BUTTON, INPUT_PULLUP);
    motor_init();
    if (!sensor_init()) return false;
    line_init();

    pd_init(&ctx.pd, PD_KP, PD_KD, PD_D_FILTER_HZ, (float)CONTROL_FREQ_HZ, PD_OUT_LIMIT);
    lp_init(&ctx.posFilter, POS_FILTER_HZ, (float)CONTROL_FREQ_HZ);
    fsm_init(&ctx.fsm, STATE_TABLE, STATE_IDLE, &ctx);

    timer_init(CONTROL_PERIOD_US);
    return true;
}

void control_task_run() {
    if (!timer_tick()) return;

    button_update(&ctx);
    sensor_update();                                   // DMA -> ring -> raw -> norm
    line_process(sensor_norm(), &ctx.line);
    fsm_update(&ctx.fsm, &ctx);
}

const RobotContext *control_task_context() { return &ctx; }