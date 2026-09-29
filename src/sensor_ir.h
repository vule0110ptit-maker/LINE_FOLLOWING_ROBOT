#pragma once
#include <stdint.h>

bool sensor_init();
void sensor_update();                 // poll DMA + lấy trung bình + chuẩn hóa

const uint16_t *sensor_raw();         // 0..4095
const uint16_t *sensor_norm();        // 0..1000, 1000 = đang trên line

void sensor_calib_begin();
void sensor_calib_step();             // gọi sau sensor_update()
void sensor_calib_end();