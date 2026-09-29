#pragma once
#include <stdint.h>
#include "statemachine/fsm.h"

enum StateId : uint8_t {
    STATE_IDLE = 0,
    STATE_CALIBRATE,
    STATE_READY,
    STATE_FOLLOW,
    STATE_LOST,
    STATE_STOP,
    STATE_COUNT
};

extern const StateDef STATE_TABLE[STATE_COUNT];