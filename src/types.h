#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "robot_setup.h"
#include "lowpassfilter.h"
#include "pid.h"
#include "line_process.h"
#include "fsm.h"

// Dữ liệu dùng chung giữa control_task và các state
struct RobotContext {
    LineData line;
    PD       pd;
    LowPass  posFilter;
    Fsm      fsm;
    bool     btnPressed;     // true đúng 1 tick khi có cạnh nhấn
    uint32_t lastFoundMs;
    uint32_t crossSinceMs;
};