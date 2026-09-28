#include <Arduino.h>
#include <string.h>
#include "drivers/adc_dma.h"
#include "config/robot_config.h"

static_assert((ADC_RING_SIZE & (ADC_RING_SIZE - 1)) == 0, "ADC_RING_SIZE phai la luy thua cua 2");

namespace {

constexpr uint8_t RING_MASK = ADC_RING_SIZE - 1;
constexpr uint8_t NO_CH     = 0xFF;

struct Ring {
    uint16_t buf[ADC_RING_SIZE];
    uint8_t  head;
    uint8_t  count;
};

Ring    s_ring[ADC_MAX_GPIO + 1] ;   // cấp dư, chỉ các chân trong s_pins mới được dùng thật
uint8_t s_gpioToIdx[ADC_MAX_GPIO + 1];
uint8_t s_pins[16];
uint8_t s_count = 0;

volatile bool s_dataReady = false;

// Callback chạy khi driver có 1 khung mới; chỉ đặt cờ, không xử lý nặng ở đây
void ARDUINO_ISR_ATTR onAdcConvDone() {
    s_dataReady = true;
}

}  // namespace

bool adc_dma_init(const uint8_t *pins, uint8_t count) {
    if (count == 0 || count > sizeof(s_pins)) return false;

    memset(s_ring, 0, sizeof(s_ring));
    memset(s_gpioToIdx, NO_CH, sizeof(s_gpioToIdx));
    memcpy(s_pins, pins, count);
    s_count = count;

    for (uint8_t i = 0; i < count; i++) {
        if (pins[i] > ADC_MAX_GPIO) return false;
        s_gpioToIdx[pins[i]] = i;
    }

    analogContinuousSetWidth(12);
    analogContinuousSetAtten(ADC_11db);
    return analogContinuous(s_pins, s_count, ADC_CONV_PER_PIN, ADC_SAMPLE_FREQ_HZ, &onAdcConvDone);
}

bool adc_dma_start() {
    return analogContinuousStart();
}

void adc_dma_stop() {
    analogContinuousStop();
}

void adc_dma_poll() {
    if (!s_dataReady) return;
    s_dataReady = false;

    adc_continuous_data_t *result = nullptr;
    if (!analogContinuousRead(&result, 0)) return;

    for (uint8_t k = 0; k < s_count; k++) {
        const uint8_t idx = s_gpioToIdx[result[k].pin];
        if (idx == NO_CH) continue;

        Ring &r = s_ring[idx];
        r.buf[r.head] = result[k].avg_read_raw;
        r.head = (r.head + 1) & RING_MASK;
        if (r.count < ADC_RING_SIZE) r.count++;
    }
}

uint16_t adc_dma_average(uint8_t idx, uint8_t n) {
    const Ring &r = s_ring[idx];
    if (n > r.count) n = r.count;
    if (n == 0) return 0;

    uint32_t sum = 0;
    uint8_t  p   = r.head;
    for (uint8_t k = 0; k < n; k++) {
        p = (p - 1) & RING_MASK;
        sum += r.buf[p];
    }
    return (uint16_t)(sum / n);
}