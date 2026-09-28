#ifndef __PARA__H
#define __PARA__H
#pragma once
#include <stdint.h>

// ===== PD (sai số = vị trí line, chuẩn hóa -1..1) =====
constexpr float PD_KP          = 0.55f;
constexpr float PD_KD          = 0.035f;
constexpr float PD_D_FILTER_HZ = 40.0f;   // lowpass cho thành phần D
constexpr float PD_OUT_LIMIT   = 1.0f;

// ===== Lowpass vị trí =====
constexpr float POS_FILTER_HZ  = 60.0f;

// ===== Tốc độ (-1..1) =====
constexpr float BASE_SPEED     = 0.45f;
constexpr float SPEED_DROP     = 0.15f;   // giảm tốc theo |vị trí| khi vào cua

// ===== Xử lý line (giá trị chuẩn hóa 0..1000) =====
constexpr uint16_t SENSOR_NOISE_FLOOR = 80;
constexpr uint16_t LINE_ON_THRESHOLD  = 500;
constexpr uint32_t LINE_FOUND_SUM_MIN = 250;
constexpr uint8_t  CROSS_MIN_ACTIVE   = 7;      // >= số này là vạch ngang/đích (đặt = SENSOR_COUNT+1 để tắt)
constexpr float    LAST_SIDE_MIN      = 0.3f;

// ===== Calibration =====
constexpr uint32_t CALIB_TIME_MS    = 3000;
constexpr uint32_t CALIB_SWEEP_MS   = 300;
constexpr float    CALIB_SPIN_SPEED = 0.30f;    // 0 nếu muốn quét cảm biến bằng tay
constexpr int32_t  CALIB_MIN_RANGE  = 300;      // biên độ ADC tối thiểu để coi là hợp lệ

// ===== Mất line / dừng =====
constexpr uint32_t LOST_GRACE_MS    = 80;
constexpr uint32_t LOST_TIMEOUT_MS  = 1500;
constexpr float    LOST_SPIN_SPEED  = 0.35f;
constexpr uint32_t FINISH_HOLD_MS   = 40;
constexpr uint32_t STOP_BRAKE_MS    = 300;

// ===== Nút bấm =====
constexpr uint32_t BTN_DEBOUNCE_MS  = 30;

#endif