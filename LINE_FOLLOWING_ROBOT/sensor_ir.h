#pragma once
// Vai trò: lọc và chuẩn hóa ADC về 0..1000, học line và kiểm tra dữ liệu còn mới.
#include <stdint.h>

bool sensor_init();
void sensor_update();                 // poll DMA + lấy trung bình + chuẩn hóa
bool sensor_data_fresh();             // false nếu chưa có ADC hoặc ADC ngừng cập nhật

const uint16_t *sensor_raw();         // 0..4095
const uint16_t *sensor_norm();        // 0..1000, 1000 = đang trên line

void sensor_calib_begin();
void sensor_calib_step();             // gọi sau sensor_update()
enum class CalibResult : uint8_t { Invalid, Saved, RamOnly };
CalibResult sensor_calib_end();       // lưu NVS nếu đủ 8 cảm biến; giữ bản cũ nếu không đạt
void sensor_calib_cancel();           // bỏ lượt học đang chạy, giữ dữ liệu cũ
bool sensor_calib_loaded();           // true nếu đã nạp dữ liệu học từ NVS khi khởi động
bool sensor_calib_valid();            // true nếu có hiệu chuẩn hợp lệ trong RAM
