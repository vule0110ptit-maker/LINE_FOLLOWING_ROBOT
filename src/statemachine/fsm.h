#pragma once
#include <stdint.h>
#include <stdbool.h>

struct RobotContext;

// Mỗi state là 3 con trỏ hàm (có thể nullptr)
typedef void (*StateFunc)(RobotContext *ctx);

typedef struct {
    StateFunc onEnter;
    StateFunc onUpdate;
    StateFunc onExit;
} StateDef;

typedef struct {
    const StateDef *table;
    uint8_t  current;
    uint8_t  next;
    bool     pending;
    uint32_t enterMs;
} Fsm;

void     fsm_init(Fsm *f, const StateDef *table, uint8_t initial, RobotContext *ctx);
void     fsm_request(Fsm *f, uint8_t next);        // chuyển state ở đầu lần update kế tiếp
void     fsm_update(Fsm *f, RobotContext *ctx);
uint32_t fsm_elapsed_ms(const Fsm *f);