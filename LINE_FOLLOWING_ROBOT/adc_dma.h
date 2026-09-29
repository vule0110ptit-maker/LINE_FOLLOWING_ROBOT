#pragma once
// Vai trò: đọc đồng thời 8 kênh ADC bằng DMA và giữ các mẫu gần nhất trong bộ đệm vòng.
#include <stdint.h>

// Khởi tạo ADC continuous (DMA) cho danh sách chân GPIO, dùng API Arduino-ESP32
bool adc_dma_init(const uint8_t *gpioPins, uint8_t count);
bool adc_dma_start();
void adc_dma_stop();

// Rút khung dữ liệu mới nhất (nếu có) -> đẩy vào ring buffer từng kênh (không chặn)
bool adc_dma_poll(); // true khi nhận được khung ADC hợp lệ

// Trung bình n mẫu mới nhất của kênh idx (idx = thứ tự trong gpioPins)
uint16_t adc_dma_average(uint8_t idx, uint8_t n);
