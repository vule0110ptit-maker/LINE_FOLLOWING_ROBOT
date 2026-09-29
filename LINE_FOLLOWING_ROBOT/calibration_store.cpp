#include <Preferences.h>
#include <stddef.h>
#include "calibration_store.h"
#include "parameters.h"
#include "pin.h"

// Lưu hiệu chuẩn vào NVS (flash) dưới một khóa duy nhất. Không ghi mỗi lần đọc ADC.
namespace {
constexpr uint32_t MAGIC = 0x4C43414C; // "LCAL"
constexpr uint16_t VERSION = 1;

struct Record {
    // Metadata giúp từ chối dữ liệu của phiên bản code hoặc sơ đồ chân khác.
    uint32_t magic;
    uint16_t version;
    uint8_t count;
    uint8_t pins[SENSOR_COUNT];
    CalibrationData data;
    uint32_t checksum;
};

uint32_t checksum(const Record &r) {
    // FNV-1a phát hiện blob bị lỗi; bỏ qua chính trường checksum khi tính.
    const uint8_t *bytes = reinterpret_cast<const uint8_t *>(&r);
    uint32_t hash = 2166136261u;
    for (size_t i = 0; i < offsetof(Record, checksum); ++i)
        hash = (hash ^ bytes[i]) * 16777619u;
    return hash;
}

bool valid(const Record &r) {
    // Ngoài checksum, từng cảm biến phải có dải ADC đủ rộng để hiệu chuẩn.
    if (r.magic != MAGIC || r.version != VERSION || r.count != SENSOR_COUNT ||
        r.checksum != checksum(r)) return false;
    for (uint8_t i = 0; i < SENSOR_COUNT; ++i) {
        if (r.pins[i] != IR_PINS[i] || r.data.min[i] > 4095 || r.data.max[i] > 4095 ||
            r.data.max[i] - r.data.min[i] < CALIB_MIN_RANGE) return false;
    }
    return true;
}
} // namespace

bool calibration_store_load(CalibrationData *out) {
    Preferences prefs;
    // true = chỉ đọc, tránh ghi flash lúc khởi động.
    if (!prefs.begin("linecal", true)) return false;
    Record record{};
    const bool read = prefs.getBytesLength("sensor") == sizeof(record) &&
                      prefs.getBytes("sensor", &record, sizeof(record)) == sizeof(record);
    prefs.end();
    if (!read || !valid(record)) return false;
    *out = record.data;
    return true;
}

bool calibration_store_save(const CalibrationData &data) {
    // Tạo toàn bộ record rồi ghi một lần sau khi lượt học hoàn tất.
    Record record{};
    record.magic = MAGIC;
    record.version = VERSION;
    record.count = SENSOR_COUNT;
    for (uint8_t i = 0; i < SENSOR_COUNT; ++i) record.pins[i] = IR_PINS[i];
    record.data = data;
    record.checksum = checksum(record);
    if (!valid(record)) return false;

    Preferences prefs;
    if (!prefs.begin("linecal", false)) return false;
    const bool saved = prefs.putBytes("sensor", &record, sizeof(record)) == sizeof(record);
    prefs.end();
    return saved;
}
