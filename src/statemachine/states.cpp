#include <Arduino.h>
#include "statemachine/states.h"
#include "statemachine/fsm.h"
#include "utils/types.h"
#include "config/parameters.h"
#include "drivers/sensor_ir.h"
#include "control/pid.h"
#include "control/lowpassfilter.h"
#include "drivers/motor.h"

// ---------------- IDLE: chờ bấm nút để calibrate ----------------
static void idle_enter(RobotContext *) { motor_stop(); }
static void idle_update(RobotContext *c) {
    if (c->btnPressed) fsm_request(&c->fsm, STATE_CALIBRATE);
}

// ---------------- CALIBRATE: quét min/max cảm biến ----------------
static void calib_enter(RobotContext *) { sensor_calib_begin(); }
static void calib_update(RobotContext *c) {
    static const int8_t PATTERN[4] = {1, -1, -1, 1};   // phải, trái, trái, phải -> quét 2 phía rồi về giữa
    sensor_calib_step();

    const uint32_t t = fsm_elapsed_ms(&c->fsm);
    const float s = (float)PATTERN[(t / CALIB_SWEEP_MS) & 3] * CALIB_SPIN_SPEED;
    motor_set(s, -s);

    if (t >= CALIB_TIME_MS) fsm_request(&c->fsm, STATE_READY);
}
static void calib_exit(RobotContext *) {
    motor_stop();
    sensor_calib_end();
}

// ---------------- READY: đặt xe lên line, bấm nút để chạy ----------------
static void ready_enter(RobotContext *) { motor_stop(); }
static void ready_update(RobotContext *c) {
    if (c->btnPressed) fsm_request(&c->fsm, STATE_FOLLOW);
}

// ---------------- FOLLOW: bám line bằng PD ----------------
static void follow_enter(RobotContext *c) {
    pd_reset(&c->pd);
    lp_reset(&c->posFilter);
    c->lastFoundMs  = millis();
    c->crossSinceMs = 0;
}
static void follow_update(RobotContext *c) {
    if (c->btnPressed) { fsm_request(&c->fsm, STATE_STOP); return; }

    const LineData &ln = c->line;
    const uint32_t now = millis();

    if (ln.found) {
        c->lastFoundMs = now;
    } else if (now - c->lastFoundMs >= LOST_GRACE_MS) {
        fsm_request(&c->fsm, STATE_LOST);
        return;
    }

    if (ln.crossing) {
        if (c->crossSinceMs == 0) {
            c->crossSinceMs = now;
        } else if (now - c->crossSinceMs >= FINISH_HOLD_MS) {
            fsm_request(&c->fsm, STATE_STOP);
            return;
        }
    } else {
        c->crossSinceMs = 0;
    }

    const float pos   = lp_update(&c->posFilter, ln.position);
    const float steer = pd_update(&c->pd, pos);                 // >0: rẽ phải
    const float base  = BASE_SPEED - SPEED_DROP * fabsf(pos);   // vào cua thì giảm tốc
    motor_set(base + steer, base - steer);
}

// ---------------- LOST: xoay tại chỗ về phía cuối cùng thấy line ----------------
static void lost_update(RobotContext *c) {
    if (c->btnPressed) {
        fsm_request(&c->fsm, STATE_STOP);
    } else if (c->line.found) {
        fsm_request(&c->fsm, STATE_FOLLOW);
    } else if (fsm_elapsed_ms(&c->fsm) >= LOST_TIMEOUT_MS) {
        fsm_request(&c->fsm, STATE_STOP);
    } else {
        const float s = (float)c->line.lastSide * LOST_SPIN_SPEED;
        motor_set(s, -s);
    }
}

// ---------------- STOP: phanh rồi thả, bấm nút để về READY ----------------
static void stop_enter(RobotContext *) { motor_brake(); }
static void stop_update(RobotContext *c) {
    if (fsm_elapsed_ms(&c->fsm) >= STOP_BRAKE_MS) motor_stop();
    if (c->btnPressed) fsm_request(&c->fsm, STATE_READY);
}

// ---------------- Bảng state (theo thứ tự enum StateId) ----------------
const StateDef STATE_TABLE[STATE_COUNT] = {
    /* IDLE      */ {idle_enter,   idle_update,   nullptr},
    /* CALIBRATE */ {calib_enter,  calib_update,  calib_exit},
    /* READY     */ {ready_enter,  ready_update,  nullptr},
    /* FOLLOW    */ {follow_enter, follow_update, nullptr},
    /* LOST      */ {nullptr,      lost_update,   nullptr},
    /* STOP      */ {stop_enter,   stop_update,   nullptr},
};