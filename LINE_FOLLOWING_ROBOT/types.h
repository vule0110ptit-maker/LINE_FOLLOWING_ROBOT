#pragma once
// Vai trò: gom dữ liệu hiện tại của robot để các module điều khiển dùng chung.

#include <stdint.h>
#include <stdbool.h>
#include "robot_setup.h"
#include "lowpassfilter.h"
#include "pid.h"
#include "line_process.h"
#include "fsm.h"
#include "marker_tracker.h"

// Dữ liệu dùng chung giữa control_task và các state
struct RobotContext {
    LineData line;
    PD       pd;
    LowPass  posFilter;
    Fsm      fsm;
    bool     btnPressed;     // true đúng 1 tick khi có cạnh nhấn
    bool     learnPressed;   // nút học line, active LOW
    uint32_t lastFoundMs;
    float    lastGoodPos;
    MarkerTracker marker;
};
