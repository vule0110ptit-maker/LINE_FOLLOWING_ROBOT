#ifndef __PARA__H
#define __PARA__H

#pragma once
// Vai trò: các ngưỡng và tốc độ có thể chỉnh khi thử xe trên map thật.
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
constexpr uint16_t LINE_OFF_THRESHOLD = 350; // hysteresis: giảm nhấp nháy khi line mờ
constexpr uint32_t LINE_FOUND_SUM_MIN = 250;
constexpr uint8_t  CROSS_MIN_ACTIVE   = 7;      // vùng rộng: giao cắt hoặc vạch ngang
constexpr float    LAST_SIDE_MIN      = 0.3f;

// Khi có hai vùng line rời nhau, KEEP bám vùng gần vị trí line trước đó.
// Nếu hai nhánh đối xứng hoàn toàn, KEEP chọn nhánh trái.
enum class BranchChoice : uint8_t { Keep, Left, Right };
constexpr BranchChoice BRANCH_CHOICE = BranchChoice::Keep;
// Chỉ bật đếm vòng nếu map có mốc rộng đặc trưng, không trùng giao lộ.
enum class MarkerPattern : uint8_t { Disabled, SingleBar, DoubleBar };
constexpr MarkerPattern LAP_MARKER_PATTERN = MarkerPattern::Disabled;
constexpr uint8_t TARGET_LAPS = 1;
constexpr uint32_t MIN_LAP_MS = 3000;    // tránh đếm lại cùng một mốc sau khi vừa xuất phát
constexpr uint32_t DOUBLE_BAR_MIN_MS = 100;
constexpr uint32_t DOUBLE_BAR_MAX_MS = 800;

// ===== Calibration =====
constexpr uint32_t CALIB_TIME_MS    = 3000;
constexpr uint32_t CALIB_SWEEP_MS   = 300;
constexpr float    CALIB_SPIN_SPEED = 0.30f;    // 0 nếu muốn quét cảm biến bằng tay
constexpr int32_t  CALIB_MIN_RANGE  = 300;      // biên độ ADC tối thiểu để coi là hợp lệ

// ===== Mất line / dừng =====
constexpr uint32_t LOST_GRACE_MS    = 80;
constexpr uint32_t SENSOR_STALE_TIMEOUT_MS = 100; // quá hạn này phải dừng motor
constexpr uint32_t LOST_TIMEOUT_MS  = 1500;
constexpr float    LOST_SPIN_SPEED  = 0.35f;
constexpr float    GAP_SPEED        = 0.20f;  // chạy thẳng chậm qua nét đứt ngắn
constexpr float    GAP_MAX_ENTRY_POS = 0.35f; // chỉ băng qua khi vừa thấy line gần giữa
constexpr uint32_t LOST_INITIAL_SWEEP_MS = 500;
constexpr uint32_t LOST_SWEEP_MS = 350;
constexpr uint32_t FINISH_HOLD_MS   = 40;     // lọc nhiễu trước khi đếm vùng rộng
constexpr uint32_t MARKER_CLEAR_MS = 60;    // phải rời mốc trước khi nhận mốc kế
constexpr uint32_t STOP_BRAKE_MS    = 300;

// ===== Nút bấm =====
constexpr uint32_t BTN_DEBOUNCE_MS  = 30;

#endif
