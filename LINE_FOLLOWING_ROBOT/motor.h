#pragma once
// Vai trò: điều khiển hai motor qua TB6612FNG bằng chân hướng và PWM LEDC.

bool motor_init();                      // false nếu không khởi tạo được PWM
void motor_set(float left, float right);  // -1..1, âm = lùi
void motor_stop();                        // thả trôi
void motor_brake();                       // phanh ngắn mạch
void motor_get_command(float *left, float *right); // lệnh tốc độ cuối, để giám sát
