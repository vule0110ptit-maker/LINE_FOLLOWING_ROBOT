#include <Arduino.h>
#include "control_task.h"
#include "timer.h"

// Bật khi cần tune. Serial USB có thể chặn nếu không có host -> làm trễ vòng điều khiển.
#define DEBUG_TELEMETRY 0

void setup() {
    Serial.begin(115200);
    if (!control_task_init()) {
        Serial.println("Khoi tao ADC DMA that bai");
        while (true) delay(1000);
    }
}

void loop() {
    control_task_run();

#if DEBUG_TELEMETRY
    static uint32_t lastPrint = 0;
    const uint32_t now = millis();
    if (now - lastPrint >= 100) {
        lastPrint = now;
        const RobotContext *c = control_task_context();
        Serial.printf("st=%u pos=%.3f found=%d ovr=%lu\n",
                      c->fsm.current, c->line.position, c->line.found,
                      (unsigned long)timer_overruns());
    }
#endif
}