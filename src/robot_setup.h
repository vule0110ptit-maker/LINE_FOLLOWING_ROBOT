#ifndef __ROBOT__H
#define __ROBOT__H
#include <stdint.h>
#pragma once
// ===== Cảm biến =====
constexpr uint8_t SENSOR_COUNT  = 8;
constexpr bool    LINE_ADC_HIGH = false;   // true: ADC cao khi cảm biến nằm trên line

// ===== ADC DMA (continuous) =====
// ===== ADC DMA (continuous) =====
constexpr uint32_t ADC_SAMPLE_FREQ_HZ  = 40000;  // tổng cho mọi kênh (611..83333) -> 5 kHz/kênh
constexpr uint32_t ADC_CONV_PER_PIN    = 10;     // số lần đọc mỗi chân trong 1 khung, driver tự trung bình
constexpr uint8_t  ADC_RING_SIZE       = 16;     // mẫu/kênh, PHẢI là lũy thừa của 2
constexpr uint8_t  ADC_AVG_SAMPLES     = 8;      // trung bình N mẫu mới nhất trong ring
constexpr uint8_t  ADC_MAX_GPIO        = 48;     // GPIO cao nhất trên ESP32-S3, dùng để tra bảng
// ===== PWM motor =====
constexpr uint32_t PWM_FREQ_HZ  = 20000;
constexpr uint8_t  PWM_RES_BITS = 10;
constexpr uint32_t PWM_MAX_DUTY = (1u << PWM_RES_BITS) - 1u;

// ===== Vòng điều khiển =====
constexpr uint32_t CONTROL_FREQ_HZ   = 500;
constexpr uint32_t CONTROL_PERIOD_US = 1000000UL / CONTROL_FREQ_HZ;

#endif