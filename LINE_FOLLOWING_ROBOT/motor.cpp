#include <Arduino.h>
#include "motor.h"
#include "pin.h"
#include "robot_setup.h"
#include "math_utils.h"

namespace {

struct Channel {
    uint8_t pwm, in1, in2;
    int8_t  dir;   // -1, 0, 1; 2 = chưa xác định (buộc ghi lại chân IN)
};

Channel chL = {PIN_MOTOR_L_PWM, PIN_MOTOR_L_IN1, PIN_MOTOR_L_IN2, 2};
Channel chR = {PIN_MOTOR_R_PWM, PIN_MOTOR_R_IN1, PIN_MOTOR_R_IN2, 2};
float s_left = 0.0f, s_right = 0.0f;

inline void drive(Channel &c, float v) {
    // Giá trị [-1, 1]: dấu chọn chiều, độ lớn chọn duty PWM.
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

bool motor_init() {
    pinMode(PIN_MOTOR_STBY, OUTPUT);
    digitalWrite(PIN_MOTOR_STBY, LOW);  // giữ driver tắt đến khi PWM và chiều quay sẵn sàng

    Channel *chs[2] = {&chL, &chR};
    bool pwmReady = true;
    for (Channel *c : chs) {
        pinMode(c->in1, OUTPUT);
        pinMode(c->in2, OUTPUT);
        digitalWrite(c->in1, LOW);
        digitalWrite(c->in2, LOW);
        if (!ledcAttach(c->pwm, PWM_FREQ_HZ, PWM_RES_BITS)) pwmReady = false;
    }
    if (!pwmReady) return false;
    motor_stop();
    digitalWrite(PIN_MOTOR_STBY, HIGH);
    return true;
}

void motor_set(float left, float right) {
    s_left = clampf(left, -1.0f, 1.0f);
    s_right = clampf(right, -1.0f, 1.0f);
    drive(chL, s_left);
    drive(chR, s_right);
}

void motor_stop() {
    motor_set(0.0f, 0.0f);
}

void motor_brake() {
    // TB6612FNG: hai chân IN cùng HIGH và PWM bật tạo phanh điện.
    s_left = s_right = 0.0f;
    brake(chL);
    brake(chR);
}

void motor_get_command(float *left, float *right) {
    *left = s_left;
    *right = s_right;
}
