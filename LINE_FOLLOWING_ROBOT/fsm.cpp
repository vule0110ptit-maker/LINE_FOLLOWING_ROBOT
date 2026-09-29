#include <Arduino.h>
#include "fsm.h"

void fsm_init(Fsm *f, const StateDef *table, uint8_t initial, RobotContext *ctx) {
    f->table   = table;
    f->current = initial;
    f->next    = initial;
    f->pending = false;
    f->enterMs = millis();
    if (table[initial].onEnter) table[initial].onEnter(ctx);
}

void fsm_request(Fsm *f, uint8_t next) {
    if (next == f->current) return;
    f->next    = next;
    f->pending = true;
}

void fsm_update(Fsm *f, RobotContext *ctx) {
    if (f->pending) {
        f->pending = false;
        const StateFunc onExit = f->table[f->current].onExit;
        if (onExit) onExit(ctx);

        f->current = f->next;
        f->enterMs = millis();

        const StateFunc onEnter = f->table[f->current].onEnter;
        if (onEnter) onEnter(ctx);
    }
    const StateFunc onUpdate = f->table[f->current].onUpdate;
    if (onUpdate) onUpdate(ctx);
}

uint32_t fsm_elapsed_ms(const Fsm *f) {
    return millis() - f->enterMs;
}