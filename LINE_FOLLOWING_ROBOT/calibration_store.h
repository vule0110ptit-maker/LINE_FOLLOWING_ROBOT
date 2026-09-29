#pragma once
// Vai trò: lưu và nạp min/max của 8 cảm biến vào NVS để nhớ sau khi tắt nguồn.

#include <stdint.h>
#include "robot_setup.h"

struct CalibrationData {
    uint16_t min[SENSOR_COUNT];
    uint16_t max[SENSOR_COUNT];
};

// Lưu một lần sau khi học xong; không ghi flash trong vòng điều khiển.
bool calibration_store_load(CalibrationData *out);
bool calibration_store_save(const CalibrationData &data);
