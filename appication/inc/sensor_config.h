/**
 * =============================================================================
 * FILE: sensor_config.h
 * DESCRIPTION: Cấu hình hardware và constants cho hệ thống cảm biến MQ137 & MQ135
 * VERSION: 1.0
 * =============================================================================
 */

#ifndef SENSOR_CONFIG_H
#define SENSOR_CONFIG_H

#include "main.h"
#include "adc.h"
// =============================================================================
// CẤU HÌNH CHÂN KẾT NỐI
// =============================================================================

/*
 * Sơ đồ kết nối:
 * MQ137 (NH3)          →    STM32F103        MQ135 (CO2)        →    STM32F103
 * -----------               ---------        -----------------        ---------
 * VCC                  →    5V              VCC                 →    5V
 * GND                  →    GND             GND                 →    GND
 * AOUT                 →    PA0 (ADC1_CH0)  AOUT                →    PA1 (ADC1_CH1)
 * DOUT                 →    PA3 (GPIO)      DOUT                →    PA4 (GPIO)
 */

// Cấu hình chân MQ137 (NH3)
#define MQ137_ADC_CHANNEL   ADC_Channel_0
#define MQ137_DIGITAL_PORT  GPIOA
#define MQ137_DIGITAL_PIN   GPIO_PIN_3

// Cấu hình chân MQ135 (CO2)
#define MQ135_ADC_CHANNEL   ADC_Channel_1
#define MQ135_DIGITAL_PORT  GPIOA
#define MQ135_DIGITAL_PIN   GPIO_PIN_4

// =============================================================================
// CẤU HÌNH ADC VÀ MẠCH CHIA ÁP
// =============================================================================

/*
 * LƯU Ý: Cần mạch chia áp cho AOUT vì STM32F103 chỉ đo được 0-3.3V
 * Mạch chia áp: AOUT → R1(2.7kΩ) → ADC_PIN → R2(5.1kΩ) → GND
 * Tỷ lệ chia: 3.3V / 5V = 0.66
 */

#define ADC_VREF            3.3f    // Điện áp tham chiếu ADC (V)
#define ADC_RESOLUTION      4095    // 12-bit ADC
#define SENSOR_VCC          5.0f    // Điện áp cấp cho cảm biến (V)
#define VOLTAGE_DIVIDER_RATIO 0.66f // Tỷ lệ chia áp (3.3V/5V)
#define LOAD_RESISTANCE     10000.0f // Điện trở tải (Ω)

// =============================================================================
// CẤU HÌNH CẢM BIẾN MQ137 (NH3)
// =============================================================================

// Ngưỡng cảnh báo NH3 (ppm)
#define MQ137_THRESHOLD_LOW     15.0f
#define MQ137_THRESHOLD_HIGH    25.0f
#define MQ137_THRESHOLD_DANGER  50.0f

// Thông số đặc tuyến MQ137 cho NH3
#define MQ137_CURVE_A           102.2f
#define MQ137_CURVE_B           -2.473f
#define MQ137_MIN_PPM           0.0f
#define MQ137_MAX_PPM           500.0f

// Giá trị R0 mặc định (cần hiệu chuẩn)
#define MQ137_R0_DEFAULT        10000.0f

// =============================================================================
// CẤU HÌNH CẢM BIẾN MQ135 (CO2)
// =============================================================================

// Ngưỡng cảnh báo CO2 (ppm)
#define MQ135_THRESHOLD_LOW     800.0f
#define MQ135_THRESHOLD_HIGH    1200.0f
#define MQ135_THRESHOLD_DANGER  2000.0f

// Thông số đặc tuyến MQ135 cho CO2
#define MQ135_CURVE_A           116.6f
#define MQ135_CURVE_B           -2.769f
#define MQ135_MIN_PPM           300.0f
#define MQ135_MAX_PPM           5000.0f

// Giá trị R0 mặc định (cần hiệu chuẩn)
#define MQ135_R0_DEFAULT        10000.0f

// =============================================================================
// CẤU HÌNH HỆ THỐNG
// =============================================================================

// Buffer lưu lịch sử
#define HISTORY_SIZE            60

// Thời gian ổn định cảm biến (giây)
#define SENSOR_WARMUP_TIME      30

// Số lần đọc ADC để lấy trung bình
#define ADC_SAMPLE_COUNT        10

// Độ trễ giữa các lần đọc ADC (ms)
#define ADC_SAMPLE_DELAY        1

// Hệ số lọc nhiễu (0-1, càng nhỏ càng mượt)
#define NOISE_FILTER_FACTOR     0.2f

// =============================================================================
// VALIDATION LIMITS
// =============================================================================

// Giới hạn điện áp hợp lệ sau mạch chia áp (V)
#define MIN_VALID_VOLTAGE       0.2f
#define MAX_VALID_VOLTAGE       4.8f

// Giới hạn điện trở cảm biến hợp lệ (Ω)
#define MIN_VALID_RESISTANCE    1000.0f
#define MAX_VALID_RESISTANCE    200000.0f

#endif /* SENSOR_CONFIG_H */
