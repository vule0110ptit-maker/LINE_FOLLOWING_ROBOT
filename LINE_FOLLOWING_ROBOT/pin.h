#ifndef __PIN__H
#define __PIN__H

#pragma once
// Vai trò: khai báo chân của 8 cảm biến, hai motor và hai nút bấm.
#include <stdint.h>
#include "robot_setup.h"

// ADC1 trên ESP32-S3 = GPIO1..GPIO10. Thứ tự: trái -> phải
constexpr uint8_t IR_PINS[SENSOR_COUNT] = {1, 2, 3, 4, 5, 6, 7, 8};

// Driver motor (TB6612FNG) - đổi theo mạch của bạn
constexpr uint8_t PIN_MOTOR_L_PWM = 15;
constexpr uint8_t PIN_MOTOR_L_IN1 = 16;
constexpr uint8_t PIN_MOTOR_L_IN2 = 17;
constexpr uint8_t PIN_MOTOR_R_PWM = 18;
constexpr uint8_t PIN_MOTOR_R_IN1 = 21;
constexpr uint8_t PIN_MOTOR_R_IN2 = 47;
constexpr uint8_t PIN_MOTOR_STBY  = 14;

constexpr uint8_t PIN_BUTTON = 0;        // nút CHẠY/DỪNG hiện có, active LOW
constexpr uint8_t PIN_LEARN_BUTTON = 10; // nút HỌC LINE mới, nối GPIO10 xuống GND

#endif
