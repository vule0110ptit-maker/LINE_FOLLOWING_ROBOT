#include <Arduino.h>
#include "drivers/motor.h"
#include "config/pin.h"
#include "config/robot_setup.h"
#include "utils/math_utils.h"

namespace {

struct Channel {
    uint8_t pwm, in1, in2;
    int8_t  dir;   // -1, 0, 1; 2 = chưa xác định (buộc ghi lại chân IN)
};

Channel chL = {PIN_MOTOR_L_PWM, PIN_MOTOR_L_IN1, PIN_MOTOR_L_IN2, 2};
Channel chR = {PIN_MOTOR_R_PWM, PIN_MOTOR_R_IN1, PIN_MOTOR_R_IN2, 2};

inline void drive(Channel &c, float v) {
    v = clampf(v, -1.0f, 1.0f);
    const int8_t dir = sgnf(v);
    if (dir != c.dir) {            // chỉ ghi chân hướng khi đổi chiều
        c.dir = dir;
        digitalWrite(c.in1, dir > 0);
        digitalWrite(c.in2, dir < 0);
    }
    ledcWrite(c.pwm, (uint32_t)(fabsf(v) * (float)PWM_MAX_DUTY + 0.5f));
}

inline void brake(Channel &c) {
    digitalWrite(c.in1, HIGH);
    digitalWrite(c.in2, HIGH);
    ledcWrite(c.pwm, PWM_MAX_DUTY);
    c.dir = 2;
}

}  // namespace

void motor_init() {
    pinMode(PIN_MOTOR_STBY, OUTPUT);
    digitalWrite(PIN_MOTOR_STBY, HIGH);

    Channel *chs[2] = {&chL, &chR};
    for (Channel *c : chs) {
        pinMode(c->in1, OUTPUT);
        pinMode(c->in2, OUTPUT);
        ledcAttach(c->pwm, PWM_FREQ_HZ, PWM_RES_BITS);
    }
    motor_stop();
}

void motor_set(float left, float right) {
    drive(chL, left);
    drive(chR, right);
}

void motor_stop() {
    drive(chL, 0.0f);
    drive(chR, 0.0f);
}

void motor_brake() {
    brake(chL);
    brake(chR);
}