#ifndef __PIN__H
#define __PIN__H

#pragma once
// Vai trò: khai báo chân của 8 cảm biến, hai motor và hai nút bấm.
#include <stdint.h>
#include "robot_setup.h"

// ESP32-S3-DevKitC-1: 8 đầu vào ADC1 cùng hàng J1.
// Thứ tự trong mảng là vị trí cảm biến trên xe: trái -> phải.
constexpr uint8_t IR_PINS[SENSOR_COUNT] = {4, 5, 6, 7, 8, 3, 9, 10};

// Driver TB6612FNG: 6 chân liên tiếp trên hàng J3 (vị trí 4..9).
// GPIO39..42 dùng cho motor, không dùng JTAG ngoài trên các chân này.
constexpr uint8_t PIN_MOTOR_L_PWM = 1;
constexpr uint8_t PIN_MOTOR_L_IN1 = 2;
constexpr uint8_t PIN_MOTOR_L_IN2 = 42;
constexpr uint8_t PIN_MOTOR_R_PWM = 41;
constexpr uint8_t PIN_MOTOR_R_IN1 = 40;
constexpr uint8_t PIN_MOTOR_R_IN2 = 39;

constexpr uint8_t PIN_BUTTON = 47;       // J3: nút CHẠY/DỪNG ngoài, nối GPIO47 xuống GND
constexpr uint8_t PIN_LEARN_BUTTON = 21; // J3: nút HỌC LINE, nối GPIO21 xuống GND

#endif
