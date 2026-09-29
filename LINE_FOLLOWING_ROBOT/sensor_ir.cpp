#include <Arduino.h>
#include "sensor_ir.h"
#include "adc_dma.h"
#include "pin.h"
#include "robot_setup.h"
#include "parameters.h"
#include "math_utils.h"
#include "calibration_store.h"

namespace {
uint16_t s_raw[SENSOR_COUNT];
uint16_t s_norm[SENSOR_COUNT];
uint16_t s_min[SENSOR_COUNT];
uint16_t s_max[SENSOR_COUNT];
float    s_scale[SENSOR_COUNT];   // 1000 / (max - min), tính sẵn để tránh chia mỗi tick
// Giá trị thử được tách khỏi hiệu chuẩn đang dùng: học thất bại không làm mất bản cũ.
uint16_t s_trialMin[SENSOR_COUNT];
uint16_t s_trialMax[SENSOR_COUNT];
bool s_loaded = false;
bool s_calibrated = false;
bool s_learning = false;
bool s_haveFrame = false;
uint32_t s_lastFrameMs = 0;

void resetCalib() {
    for (uint8_t i = 0; i < SENSOR_COUNT; i++) {
        s_min[i]   = 0;
        s_max[i]   = 4095;
        s_scale[i] = 1000.0f / 4095.0f;
    }
}
}  // namespace

bool sensor_init() {
    resetCalib();
    s_haveFrame = false;
    // Có bản NVS hợp lệ thì dùng lại ngay; nếu không, chờ người dùng học line.
    CalibrationData saved;
    s_loaded = calibration_store_load(&saved);
    s_calibrated = s_loaded;
    if (s_loaded) {
        for (uint8_t i = 0; i < SENSOR_COUNT; ++i) {
            s_min[i] = saved.min[i];
            s_max[i] = saved.max[i];
            s_scale[i] = 1000.0f / (float)(s_max[i] - s_min[i]);
        }
    }
    if (!adc_dma_init(IR_PINS, SENSOR_COUNT)) return false;
    return adc_dma_start();
}

void sensor_update() {
    if (adc_dma_poll()) {
        s_haveFrame = true;
        s_lastFrameMs = millis();
    }
    for (uint8_t i = 0; i < SENSOR_COUNT; i++) {
        const uint16_t raw = adc_dma_average(i, ADC_AVG_SAMPLES);
        s_raw[i] = raw;

        // Đưa mỗi cảm biến về 0..1000; 1000 luôn có nghĩa là đang trên line.
        float n = (float)((int32_t)raw - (int32_t)s_min[i]) * s_scale[i];
        n = clampf(n, 0.0f, 1000.0f);
        if (!LINE_ADC_HIGH) n = 1000.0f - n;
        s_norm[i] = (uint16_t)n;
    }
}

bool sensor_data_fresh() {
    return s_haveFrame && millis() - s_lastFrameMs <= SENSOR_STALE_TIMEOUT_MS;
}

const uint16_t *sensor_raw()  { return s_raw; }
const uint16_t *sensor_norm() { return s_norm; }

void sensor_calib_begin() {
    s_learning = true;
    for (uint8_t i = 0; i < SENSOR_COUNT; i++) {
        s_trialMin[i] = 4095;
        s_trialMax[i] = 0;
    }
}

void sensor_calib_step() {
    if (!s_learning) return;
    // Mỗi tick lấy min/max của cả 8 cảm biến trong khi xe xoay quét map.
    for (uint8_t i = 0; i < SENSOR_COUNT; i++) {
        const uint16_t r = s_raw[i];
        if (r < s_trialMin[i]) s_trialMin[i] = r;
        if (r > s_trialMax[i]) s_trialMax[i] = r;
    }
}

CalibResult sensor_calib_end() {
    if (!s_learning) return CalibResult::Invalid;
    s_learning = false;
    bool valid = true;
    for (uint8_t i = 0; i < SENSOR_COUNT; i++) {
        const int32_t range = (int32_t)s_trialMax[i] - (int32_t)s_trialMin[i];
        if (range < CALIB_MIN_RANGE) valid = false;
    }
    if (!valid) return CalibResult::Invalid; // giữ bản đã học trước đó

    // Chỉ sau khi cả 8 cảm biến đạt yêu cầu mới áp dụng vào RAM và ghi NVS.
    CalibrationData data;
    for (uint8_t i = 0; i < SENSOR_COUNT; ++i) {
        s_min[i] = s_trialMin[i];
        s_max[i] = s_trialMax[i];
        s_scale[i] = 1000.0f / (float)(s_max[i] - s_min[i]);
        data.min[i] = s_min[i];
        data.max[i] = s_max[i];
    }
    s_calibrated = true;
    return calibration_store_save(data) ? CalibResult::Saved : CalibResult::RamOnly;
}

void sensor_calib_cancel() { s_learning = false; }

bool sensor_calib_loaded() { return s_loaded; }
bool sensor_calib_valid() { return s_calibrated; }
