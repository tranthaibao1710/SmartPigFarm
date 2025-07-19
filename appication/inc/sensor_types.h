/**
 * =============================================================================
 * FILE: sensor_types.h
 * DESCRIPTION: Định nghĩa các kiểu dữ liệu, enum và struct cho hệ thống cảm biến
 * VERSION: 1.0
 * =============================================================================
 */

#ifndef SENSOR_TYPES_H
#define SENSOR_TYPES_H
#include "main.h"  // Để có GPIO_TypeDef

// =============================================================================
// ENUM VÀ ĐỊNH NGHĨA KIỂU DỮ LIỆU
// =============================================================================

/**
 * @brief Loại cảm biến
 */
typedef enum {
    SENSOR_TYPE_MQ137 = 0,  // NH3 Sensor
    SENSOR_TYPE_MQ136       // Multi-gas Sensor (h2s)
} SensorType_t;

/**
 * @brief Loại khí đo
 */
typedef enum {
    GAS_TYPE_NH3 = 0,
    GAS_TYPE_H2S,  // Hydrogen Sulfide
} GasType_t;

/**
 * @brief Mức cảnh báo
 */
typedef enum {
    ALARM_NORMAL = 0,   // Bình thường
    ALARM_LOW,          // Thấp
    ALARM_HIGH,         // Cao
    ALARM_DANGER        // Nguy hiểm
} AlarmLevel_t;

// =============================================================================
// CẤU TRÚC CẢM BIẾN CƠ BẢN
// =============================================================================

/**
 * @brief Cấu trúc cảm biến cơ bản - chứa thông tin hardware và dữ liệu thô
 */
typedef struct {
    // Thông tin định danh
    uint8_t sensor_id;              // ID cảm biến (0, 1, 2...)
    SensorType_t sensor_type;       // Loại cảm biến
    char sensor_name[16];           // Tên cảm biến
    uint8_t is_enabled;             // Cảm biến có hoạt động không
    uint8_t is_valid;               // Dữ liệu có hợp lệ không

    // Cấu hình phần cứng
    uint32_t adc_channel;           // Kênh ADC
    GPIO_TypeDef* digital_port;     // Port GPIO digital
    uint16_t digital_pin;           // Pin GPIO digital

    // Dữ liệu đo thô
    float raw_voltage;              // Điện áp ADC (V)
    float resistance;               // Điện trở cảm biến (Ω)
    float rs_r0_ratio;             // Tỷ lệ Rs/R0
    uint8_t digital_state;         // Trạng thái digital output
    uint32_t timestamp;            // Thời gian đo (ms)

    // Thông số hiệu chuẩn
    float r0_value;                // Giá trị R0 (hiệu chuẩn)
} BaseSensor_t;

// =============================================================================
// CẤU TRÚC CẢM BIẾN KHÍ (KẾ THỪA TỪ BASE SENSOR)
// =============================================================================

/**
 * @brief Cấu trúc cảm biến khí - kế thừa từ BaseSensor_t và thêm logic xử lý khí
 */
typedef struct {
    BaseSensor_t base;              // Cảm biến cơ bản

    // Thông tin khí đo
    GasType_t gas_type;            // Loại khí (NH3, CO2...)
    char gas_name[8];              // Tên khí ("NH3", "CO2")
    char unit[8];                  // Đơn vị ("ppm")

    // Giá trị nồng độ
    float gas_ppm;                 // Nồng độ khí hiện tại (ppm)
    float filtered_ppm;            // Giá trị sau lọc nhiễu
    float average_ppm;             // Giá trị trung bình

    // Ngưỡng cảnh báo
    float threshold_low;           // Ngưỡng cảnh báo thấp
    float threshold_high;          // Ngưỡng cảnh báo cao
    float threshold_danger;        // Ngưỡng nguy hiểm
    AlarmLevel_t alarm_level;      // Mức cảnh báo hiện tại

    // Thông số đặc tuyến cảm biến
    float curve_a;                 // Hệ số A trong công thức ppm = A * (Rs/R0)^B
    float curve_b;                 // Hệ số B
    float min_ppm;                 // Giá trị ppm tối thiểu
    float max_ppm;                 // Giá trị ppm tối đa
} GasSensor_t;

// =============================================================================
// HỆ THỐNG QUẢN LÝ CẢM BIẾN
// =============================================================================

/**
 * @brief Cấu trúc hệ thống quản lý nhiều cảm biến
 */
typedef struct {
    // Cảm biến cụ thể
    GasSensor_t mq137;             // Cảm biến MQ137 (NH3)
    GasSensor_t mq136;             // Cảm biến MQ136 (h2s)

    // Trạng thái hệ thống
    uint8_t active_sensor_count;    // Số cảm biến đang hoạt động
    uint8_t system_status;          // Trạng thái hệ thống (0=lỗi, 1=cơ bản, 2=tốt, 3=tối ưu)
    AlarmLevel_t system_alarm;      // Mức cảnh báo tổng thể
    uint32_t last_update;           // Thời gian cập nhật cuối

    // Thống kê
    uint32_t total_readings;        // Tổng số lần đo
    uint32_t error_count;           // Số lần đo lỗi
    float uptime_minutes;           // Thời gian hoạt động (phút)
} SensorSystem_t;
extern SensorSystem_t g_sensor_system;
#endif /* SENSOR_TYPES_H */
