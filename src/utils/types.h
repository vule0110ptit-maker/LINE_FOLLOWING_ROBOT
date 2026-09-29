#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "config/robot_setup.h"
#include "control/lowpassfilter.h"
#include "control/pid.h"
#include "control/line_process.h"
#include "statemachine/fsm.h"

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