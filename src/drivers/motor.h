#pragma once

void motor_init();
void motor_set(float left, float right);  // -1..1, âm = lùi
void motor_stop();                        // thả trôi
void motor_brake();                       // phanh ngắn mạch