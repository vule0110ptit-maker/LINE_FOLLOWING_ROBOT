#include <Arduino.h>
#include "states.h"
#include "fsm.h"
#include "types.h"
#include "parameters.h"
#include "sensor_ir.h"
#include "pid.h"
#include "lowpassfilter.h"
#include "motor.h"

static void request_follow(RobotContext *c) {
    // Không khởi động motor bằng dữ liệu ADC mặc định hoặc khi xe chưa đặt lên line.
    if (!sensor_calib_valid()) {
        Serial.println("RUN=BLOCKED LEARN_LINE_FIRST");
    } else if (!c->line.found) {
        Serial.println("RUN=BLOCKED PLACE_ON_LINE");
    } else {
        marker_reset(&c->marker, millis());
        fsm_request(&c->fsm, STATE_FOLLOW);
    }
}

// ---------------- IDLE: chờ HỌC LINE hoặc CHẠY ----------------
static void idle_enter(RobotContext *) { motor_stop(); }
static void idle_update(RobotContext *c) {
    if (c->learnPressed) fsm_request(&c->fsm, STATE_CALIBRATE);
    else if (c->btnPressed) request_follow(c);
}

// ---------------- CALIBRATE: quét min/max cảm biến ----------------
static void calib_enter(RobotContext *) { sensor_calib_begin(); }
static void calib_update(RobotContext *c) {
    if (c->btnPressed) { fsm_request(&c->fsm, STATE_STOP); return; }
    // Xe xoay tại chỗ qua hai phía để từng mắt cảm biến thấy cả nền và line.
    static const int8_t PATTERN[4] = {1, -1, -1, 1};   // phải, trái, trái, phải -> quét 2 phía rồi về giữa
    sensor_calib_step();

    const uint32_t t = fsm_elapsed_ms(&c->fsm);
    const float s = (float)PATTERN[(t / CALIB_SWEEP_MS) & 3] * CALIB_SPIN_SPEED;
    motor_set(s, -s);

    if (t >= CALIB_TIME_MS) fsm_request(&c->fsm, STATE_READY);
}
static void calib_exit(RobotContext *c) {
    motor_stop();
    // Nút CHẠY trong lúc học là lệnh hủy; không ghi lượt học dở dang vào NVS.
    if (c->fsm.next == STATE_STOP) {
        sensor_calib_cancel();
        Serial.println("CAL=CANCELLED");
        return;
    }
    switch (sensor_calib_end()) {
        case CalibResult::Saved: Serial.println("CAL=SAVED_NVS"); break;
        case CalibResult::RamOnly: Serial.println("CAL=RAM_ONLY NVS_WRITE_FAILED"); break;
        case CalibResult::Invalid: Serial.println("CAL=INVALID KEEP_PREVIOUS"); break;
    }
}

// ---------------- READY: đặt xe lên line, bấm CHẠY hoặc HỌC lại ----------------
static void ready_enter(RobotContext *) { motor_stop(); }
static void ready_update(RobotContext *c) {
    if (c->learnPressed) fsm_request(&c->fsm, STATE_CALIBRATE);
    else if (c->btnPressed) request_follow(c);
}

// ---------------- FOLLOW: bám line bằng PD ----------------
static void follow_enter(RobotContext *c) {
    // Xóa đạo hàm/vị trí cũ để xe không giật khi bắt đầu hoặc vừa tìm lại line.
    pd_reset(&c->pd);
    lp_reset(&c->posFilter);
    c->lastFoundMs  = millis();
    c->lastGoodPos  = c->line.position;
}
static void follow_update(RobotContext *c) {
    if (c->btnPressed) { fsm_request(&c->fsm, STATE_STOP); return; }

    const LineData &ln = c->line;
    const uint32_t now = millis();

    if (marker_update(&c->marker, ln.found && ln.crossing, now, LAP_MARKER_PATTERN)) {
        // Chỉ dừng theo số vòng khi đã bật mẫu mốc phù hợp trong parameters.h.
        Serial.print("LAP=");
        Serial.println(c->marker.laps);
        if (c->marker.laps >= TARGET_LAPS) {
            motor_stop();
            fsm_request(&c->fsm, STATE_STOP);
            return;
        }
    }

    if (!ln.found) {
        // Nét đứt ngắn: đi chậm thẳng qua nếu line vừa ở gần giữa cảm biến.
        // Mất line khi đang lệch nhiều có thể là cua gắt, nên chuyển sang tìm ngay.
        if (now - c->lastFoundMs < LOST_GRACE_MS &&
            fabsf(c->lastGoodPos) <= GAP_MAX_ENTRY_POS) {
            motor_set(GAP_SPEED, GAP_SPEED);
        } else {
            motor_stop();
            fsm_request(&c->fsm, STATE_LOST);
        }
        return;
    }
    c->lastFoundMs = now;
    c->lastGoodPos = ln.position;

    const float pos   = lp_update(&c->posFilter, ln.position);
    // PD biến độ lệch thành chênh tốc độ hai bánh; vào cua thì giảm tốc nền.
    const float steer = pd_update(&c->pd, pos);                 // >0: rẽ phải
    const float base  = BASE_SPEED - SPEED_DROP * fabsf(pos);   // vào cua thì giảm tốc
    motor_set(base + steer, base - steer);
}

// ---------------- LOST: xoay tại chỗ về phía cuối cùng thấy line ----------------
static void lost_update(RobotContext *c) {
    if (c->btnPressed) {
        fsm_request(&c->fsm, STATE_STOP);
    } else if (c->line.found) {
        fsm_request(&c->fsm, STATE_FOLLOW);
    } else if (fsm_elapsed_ms(&c->fsm) >= LOST_TIMEOUT_MS) {
        fsm_request(&c->fsm, STATE_STOP);
    } else {
        // Quét về phía cuối thấy line trước, rồi đổi hướng định kỳ để tăng vùng tìm kiếm.
        const uint32_t elapsed = fsm_elapsed_ms(&c->fsm);
        const int8_t direction = elapsed < LOST_INITIAL_SWEEP_MS ? c->line.lastSide :
            (((elapsed - LOST_INITIAL_SWEEP_MS) / LOST_SWEEP_MS) & 1) ?
                c->line.lastSide : -c->line.lastSide;
        const float s = (float)direction * LOST_SPIN_SPEED;
        motor_set(s, -s);
    }
}

// ---------------- STOP: phanh rồi thả, bấm nút để về READY ----------------
static void stop_enter(RobotContext *) { motor_brake(); }
static void stop_update(RobotContext *c) {
    // Phanh ngắn rồi thả motor; nút HỌC vẫn cho phép hiệu chuẩn lại.
    if (fsm_elapsed_ms(&c->fsm) >= STOP_BRAKE_MS) motor_stop();
    if (c->learnPressed) fsm_request(&c->fsm, STATE_CALIBRATE);
    else if (c->btnPressed) request_follow(c);
}

// ---------------- Bảng state (theo thứ tự enum StateId) ----------------
const StateDef STATE_TABLE[STATE_COUNT] = {
    /* IDLE      */ {idle_enter,   idle_update,   nullptr},
    /* CALIBRATE */ {calib_enter,  calib_update,  calib_exit},
    /* READY     */ {ready_enter,  ready_update,  nullptr},
    /* FOLLOW    */ {follow_enter, follow_update, nullptr},
    /* LOST      */ {nullptr,      lost_update,   nullptr},
    /* STOP      */ {stop_enter,   stop_update,   nullptr},
};
