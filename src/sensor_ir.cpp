#include <Arduino.h>
#include "sensor_ir.h"
#include "adc_dma.h"
#include "pin.h"
#include "robot_setup.h"
#include "parameters.h"
#include "math_utils.h"

namespace {
uint16_t s_raw[SENSOR_COUNT];
uint16_t s_norm[SENSOR_COUNT];
uint16_t s_min[SENSOR_COUNT];
uint16_t s_max[SENSOR_COUNT];
float    s_scale[SENSOR_COUNT];   // 1000 / (max - min), tính sẵn để tránh chia mỗi tick

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
    if (!adc_dma_init(IR_PINS, SENSOR_COUNT)) return false;
    return adc_dma_start();
}

void sensor_update() {
    adc_dma_poll();
    for (uint8_t i = 0; i < SENSOR_COUNT; i++) {
        const uint16_t raw = adc_dma_average(i, ADC_AVG_SAMPLES);
        s_raw[i] = raw;

        float n = (float)((int32_t)raw - (int32_t)s_min[i]) * s_scale[i];
        n = clampf(n, 0.0f, 1000.0f);
        if (!LINE_ADC_HIGH) n = 1000.0f - n;
        s_norm[i] = (uint16_t)n;
    }
}

const uint16_t *sensor_raw()  { return s_raw; }
const uint16_t *sensor_norm() { return s_norm; }

void sensor_calib_begin() {
    for (uint8_t i = 0; i < SENSOR_COUNT; i++) {
        s_min[i] = 4095;
        s_max[i] = 0;
    }
}

void sensor_calib_step() {
    for (uint8_t i = 0; i < SENSOR_COUNT; i++) {
        const uint16_t r = s_raw[i];
        if (r < s_min[i]) s_min[i] = r;
        if (r > s_max[i]) s_max[i] = r;
    }
}

void sensor_calib_end() {
    for (uint8_t i = 0; i < SENSOR_COUNT; i++) {
        const int32_t range = (int32_t)s_max[i] - (int32_t)s_min[i];
        if (range < CALIB_MIN_RANGE) {          // cảm biến không quét được line -> dùng mặc định
            s_min[i] = 0;
            s_max[i] = 4095;
            s_scale[i] = 1000.0f / 4095.0f;
        } else {
            s_scale[i] = 1000.0f / (float)range;
        }
    }
}