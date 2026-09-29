#include <Arduino.h>
#include <stdio.h>
#include "control_task.h"
#include "motor.h"
#include "parameters.h"
#include "sensor_ir.h"
#include "states.h"
#include "timer.h"

// Sketch chính chỉ khởi động và giám sát. ADC, thuật toán bám line và motor
// chạy trong control_task.cpp / states.cpp ở nhịp 500 Hz.
static void write_if_room(const char *msg, int len) {
    // Không đợi cổng USB/Serial: việc in log không được làm trễ điều khiển xe.
    if (len > 0 && Serial.availableForWrite() >= len)
        Serial.write((const uint8_t *)msg, len);
}

static const char *state_name(uint8_t state) {
    switch (state) {
        case STATE_IDLE: return "IDLE";
        case STATE_CALIBRATE: return "LEARN";
        case STATE_READY: return "READY";
        case STATE_FOLLOW: return "FOLLOW";
        case STATE_LOST: return "LOST";
        case STATE_STOP: return "STOP";
        default: return "UNKNOWN";
    }
}

static const char *motion_name(uint8_t state, uint32_t stateMs, float left, float right) {
    if (state == STATE_STOP && stateMs < STOP_BRAKE_MS) return "BRAKE";
    if (left == 0.0f && right == 0.0f) return "STILL";
    if (left < 0.0f && right > 0.0f) return "SPIN_L";
    if (left > 0.0f && right < 0.0f) return "SPIN_R";
    if (left < 0.0f && right < 0.0f) return "BACK";
    if (left > right + 0.05f) return "RIGHT";
    if (right > left + 0.05f) return "LEFT";
    return "FORWARD";
}

static void print_telemetry() {
    const RobotContext *c = control_task_context();
    if (!sensor_data_fresh()) {
        static const char fault[] = "IR=-------- state=ADC_FAULT move=STILL\n";
        write_if_room(fault, sizeof(fault) - 1);
        return;
    }

    // IR[0] là cảm biến trái nhất. mask là dữ liệu nhị phân mà thuật toán thật sử dụng.
    char bits[SENSOR_COUNT + 1];
    for (uint8_t i = 0; i < SENSOR_COUNT; ++i)
        bits[i] = (c->line.mask & (1u << i)) ? '1' : '0';
    bits[SENSOR_COUNT] = '\0';

    float left, right;
    motor_get_command(&left, &right);
    char msg[96];
    // GAP là nhãn giám sát khi FOLLOW đang vượt một đoạn ngắt ngắn.
    const char *state = c->fsm.current == STATE_FOLLOW && !c->line.found ?
                        "GAP" : state_name(c->fsm.current);
    const int len = snprintf(msg, sizeof(msg), "IR=%s state=%s move=%s L=%+.2f R=%+.2f sg=%u\n",
                             bits, state,
                             motion_name(c->fsm.current, fsm_elapsed_ms(&c->fsm), left, right),
                             left, right, c->line.segments);
    if (len > 0 && len < (int)sizeof(msg)) write_if_room(msg, len);
}

static void print_health() {
    const RobotContext *c = control_task_context();
    static uint32_t previousOverruns = 0;
    const uint32_t overruns = timer_overruns();
    const uint32_t newOverruns = overruns - previousOverruns;
    previousOverruns = overruns;
    char msg[80];
    const int len = snprintf(msg, sizeof(msg), "HEALTH adc=%s cal=%s ovr_1s=%lu lap=%u\n",
                             sensor_data_fresh() ? "OK" : "STALE",
                             sensor_calib_valid() ? "OK" : "NONE",
                             (unsigned long)newOverruns, c->marker.laps);
    if (len > 0 && len < (int)sizeof(msg)) write_if_room(msg, len);
}

void setup() {
    Serial.begin(115200);
    if (!control_task_init()) {
        // motor_init đã giữ STBY LOW nếu PWM lỗi; nếu ADC lỗi thì PWM đang ở 0.
        Serial.println("INIT=FAILED MOTOR_PWM_OR_ADC");
        while (true) delay(1000);
    }
    Serial.println("INIT=OK ESP32-S3");
    Serial.println(sensor_calib_loaded() ? "CAL=LOADED_NVS" : "CAL=DEFAULT PRESS_LEARN");
    Serial.println("GPIO10: HOC LINE | GPIO0: CHAY/DUNG | IR: trai -> phai, 1 = tren line");
}

void loop() {
    control_task_run();

    // Giám sát tách khỏi tick điều khiển; bỏ qua bản tin nếu buffer Serial đầy.
    static uint32_t lastPrint = 0;
    static uint32_t lastHealth = 0;
    const uint32_t now = millis();
    if (now - lastHealth >= 1000) {
        lastHealth = now;
        print_health();
    }
    if (now - lastPrint >= 100) {
        lastPrint = now;
        print_telemetry();
    }
}
