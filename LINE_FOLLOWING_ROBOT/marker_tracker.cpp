#include "marker_tracker.h"
#include "parameters.h"

void marker_reset(MarkerTracker *m, uint32_t now) {
    // Gọi khi bắt đầu chuyến mới; mốc đang nằm dưới xe lúc xuất phát sẽ bị bỏ qua.
    *m = {};
    m->startMs = now;
    m->lastLapMs = now;
    m->clearSinceMs = now;
}

bool marker_update(MarkerTracker *m, bool wide, uint32_t now, MarkerPattern pattern) {
    if (pattern == MarkerPattern::Disabled) return false;

    if (!wide) {
        // Phải rời vùng rộng đủ lâu mới cho phép nhận một vạch mới.
        if (m->inWide) {
            m->inWide = false;
            m->clearSinceMs = now;
        }
        if (!m->armed && now - m->clearSinceMs >= MARKER_CLEAR_MS) m->armed = true;
        if (m->hasFirstBar && now - m->firstBarMs > DOUBLE_BAR_MAX_MS)
            m->hasFirstBar = false;
        return false;
    }

    if (!m->inWide) {
        m->inWide = true;
        m->confirmed = false;
        m->wideSinceMs = now;
    }
    if (!m->armed || m->confirmed || now - m->wideSinceMs < FINISH_HOLD_MS)
        return false;

    // Một vùng rộng chỉ tạo một sự kiện sau khi tín hiệu giữ ổn định đủ lâu.
    m->confirmed = true;
    m->armed = false;
    if (pattern == MarkerPattern::DoubleBar) {
        // Vạch thứ hai phải xuất hiện trong cửa sổ thời gian đã cấu hình.
        const uint32_t separation = now - m->firstBarMs;
        if (!m->hasFirstBar || separation < DOUBLE_BAR_MIN_MS || separation > DOUBLE_BAR_MAX_MS) {
            m->firstBarMs = now;
            m->hasFirstBar = true;
            return false;
        }
        m->hasFirstBar = false;
    }

    // Chống đếm lại mốc xuất phát hoặc cùng một mốc khi xe còn ở gần đó.
    if (now - m->startMs < MIN_LAP_MS || now - m->lastLapMs < MIN_LAP_MS)
        return false;
    if (m->laps < 255) ++m->laps;
    m->lastLapMs = now;
    return true;
}
