#include <Arduino.h>
#include "control_task.h"
#include "timer.h"
#include "fsm.h"
#include "states.h"
#include "sensor_ir.h"
#include "motor.h"
#include "line_process.h"
#include "pid.h"
#include "lowpassfilter.h"
#include "pin.h"
#include "parameters.h"
#include "robot_setup.h"

static RobotContext ctx;

struct Button {
    uint8_t stable = HIGH;
    uint8_t last = HIGH;
    uint32_t changedMs = 0;
};
static Button runButton, learnButton;

static void button_init(Button &b, uint8_t pin) {
    // Nút bị giữ lúc cấp nguồn không được tính thành một lần nhấn.
    b.stable = digitalRead(pin);
    b.last = b.stable;
    b.changedMs = millis();
}

// Mỗi nút có bộ debounce riêng; true chỉ trong một tick sau khi nhấn.
static bool button_pressed(Button &b, uint8_t pin, uint32_t now) {
    const uint8_t r = digitalRead(pin);
    if (r != b.last) {
        b.last = r;
        b.changedMs = now;
    } else if (r != b.stable && now - b.changedMs >= BTN_DEBOUNCE_MS) {
        b.stable = r;
        if (r == LOW) return true;
    }
    return false;
}

static void buttons_update(RobotContext *c) {
    const uint32_t now = millis();
    c->btnPressed = button_pressed(runButton, PIN_BUTTON, now);
    c->learnPressed = button_pressed(learnButton, PIN_LEARN_BUTTON, now);
}

bool control_task_init() {
    // Trình tự: nút/motor -> ADC -> bộ xử lý line -> PD/FSM -> timer điều khiển.
    pinMode(PIN_BUTTON, INPUT_PULLUP);
    pinMode(PIN_LEARN_BUTTON, INPUT_PULLUP);
    button_init(runButton, PIN_BUTTON);
    button_init(learnButton, PIN_LEARN_BUTTON);
    if (!motor_init()) return false;
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

    // Mỗi tick 2 ms: đọc nút, cập nhật ADC, tìm line rồi chạy state machine.
    buttons_update(&ctx);
    sensor_update();                                   // DMA -> ring -> raw -> norm
    if (!sensor_data_fresh()) {
        // ADC bị ngắt: vô hiệu cả hai nút, dừng motor ngay và rời trạng thái chuyển động.
        ctx.btnPressed = ctx.learnPressed = false;
        ctx.line = {};
        if (ctx.fsm.current == STATE_CALIBRATE || ctx.fsm.current == STATE_FOLLOW ||
            ctx.fsm.current == STATE_LOST) {
            if (ctx.fsm.current == STATE_CALIBRATE) sensor_calib_cancel();
            motor_brake();
            fsm_request(&ctx.fsm, STATE_STOP);
        } else if (ctx.fsm.current != STATE_STOP) {
            motor_stop();
        }
        fsm_update(&ctx.fsm, &ctx);
        return;
    }
    line_process(sensor_norm(), &ctx.line);
    fsm_update(&ctx.fsm, &ctx);
}

const RobotContext *control_task_context() { return &ctx; }
