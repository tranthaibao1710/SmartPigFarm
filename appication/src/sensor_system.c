/*
 * sensor_system.c
 *
 *  Created on: Jun 30, 2025
 *      Author: FPTSHOP
 */
#include "sensor_system.h"
SensorSystem_t g_sensor_system;

// Khai báo history buffer và index
#define HISTORY_BUFFER_SIZE 60  // Lưu 60 mẫu (1 giờ nếu đo mỗi phút)
static SensorSystem_t history_buffer[HISTORY_BUFFER_SIZE];
static uint8_t history_index = 0;

// Các define khác cần thiết
#define MAIN_LOOP_DELAY 60000  // 1 phút = 60000ms
void InitSensorSystem(void) {
    printf("\r\n=== KHỞI TẠO HỆ THỐNG CẢM BIẾN ===\r\n");
    
    // Reset toàn bộ hệ thống
    memset(&g_sensor_system, 0, sizeof(SensorSystem_t));
    memset(history_buffer, 0, sizeof(history_buffer));
    
    // Khởi tạo từng cảm biến
    InitMQ137Sensor(&g_sensor_system.mq137);
    InitMQ135Sensor(&g_sensor_system.mq135);
    
    // MQ137 Digital Pin
    GPIOx_Init(MQ137_DIGITAL_PORT ,MQ137_DIGITAL_PIN ,MODE_INPUT ,PU,0)  ; 
    // MQ135 Digital Pin
    GPIOx_Init(MQ135_DIGITAL_PORT ,MQ135_DIGITAL_PIN ,MODE_INPUT ,PU,0)  ;
// Khởi tạo ADC cho các channel analog
    ADCx_Init(ADC1, MQ137_ADC_CHANNEL);
    ADCx_Init(ADC1, MQ135_ADC_CHANNEL);
    
    
    // Cập nhật trạng thái hệ thống
    g_sensor_system.active_sensor_count = 2;
    g_sensor_system.last_update = HAL_GetTick();
    
    printf("✓ Khởi tạo MQ137 (NH3): %s\r\n", g_sensor_system.mq137.base.is_enabled ? "OK" : "FAIL");
    printf("✓ Khởi tạo MQ135 (CO2): %s\r\n", g_sensor_system.mq135.base.is_enabled ? "OK" : "FAIL");
    printf("✓ Cảm biến hoạt động: %d/2\r\n", g_sensor_system.active_sensor_count);
    
    // Thời gian ổn định
    printf("✓ Đang ổn định cảm biến...\r\n");
    for(int i = 30; i > 0; i--) {
        printf("Ổn định: %d giây\r", i);
        HAL_Delay(1000);
    }
    printf("\r\n✓ Hệ thống sẵn sàng!\r\n");
}
void ProcessAllSensors(void) {
    uint32_t current_time = HAL_GetTick();
    
    // Xử lý từng cảm biến
    if(g_sensor_system.mq137.base.is_enabled) {
        ProcessGasSensor(&g_sensor_system.mq137);
    }
    
    if(g_sensor_system.mq135.base.is_enabled) {
        ProcessGasSensor(&g_sensor_system.mq135);
    }
    
    // Cập nhật trạng thái hệ thống
    UpdateSystemStatus();
    
    // Lưu lịch sử
    SaveToHistory();
    
    // Cập nhật thống kê
    g_sensor_system.total_readings++;
    g_sensor_system.last_update = current_time;
    g_sensor_system.uptime_minutes = current_time / 60000.0f;
}
/**
 * @brief Hiển thị trạng thái hệ thống
 */
void DisplaySystemStatus(void) {
    printf("\r\n=== TRẠNG THÁI HỆ THỐNG CẢM BIẾN ===\r\n");
    printf("Thời gian: %lu ms | Uptime: %.1f phút\r\n", 
           HAL_GetTick(), g_sensor_system.uptime_minutes);
    
    // Hiển thị MQ137
    PrintSensorDetails(&g_sensor_system.mq137);
    
    // Hiển thị MQ135
    PrintSensorDetails(&g_sensor_system.mq135);
    
    // Trạng thái tổng thể
    printf("\r\n--- HỆ THỐNG ---\r\n");
    const char* status_text[] = {"LỖI", "Cơ bản", "Tốt", "Tối ưu"};
    const char* alarm_text[] = {"Bình thường", "Thấp", "Cao", "NGUY HIỂM"};
    
    printf("Trạng thái: %s (%d/2 cảm biến hoạt động)\r\n", 
           status_text[g_sensor_system.system_status], g_sensor_system.active_sensor_count);
    printf("Cảnh báo tổng thể: %s\r\n", alarm_text[g_sensor_system.system_alarm]);
    printf("Tổng số đo: %lu | Lỗi: %lu\r\n", 
           g_sensor_system.total_readings, g_sensor_system.error_count);
    
    printf("=====================================\r\n");
}

/**
 * @brief Test hệ thống
 */
void TestSensorSystem(void) {
    printf("\r\n=== TEST HỆ THỐNG CẢM BIẾN ===\r\n");
    
    for(int i = 0; i < 10; i++) {
        printf("Test lần %d:\r\n", i + 1);
        
        ProcessAllSensors();
        
        printf("  MQ137: %.1f ppm NH3 (Alarm: %d)\r\n", 
               GetNH3_PPM(), GetNH3AlarmLevel());
        printf("  MQ135: %.1f ppm CO2 (Alarm: %d)\r\n", 
               GetCO2_PPM(), GetCO2AlarmLevel());
        printf("  Hệ thống: Status=%d, Alarm=%d\r\n\r\n", 
               GetSystemStatus(), GetSystemAlarmLevel());
        
        HAL_Delay(3000);
    }
    
    printf("✓ Test hoàn tất!\r\n");
}
/**
 * @brief Hiệu chuẩn cảm biến
 */
void CalibrateSensors(void) {
    printf("\r\n=== HIỆU CHUẨN CẢM BIẾN ===\r\n");
    printf("Đặt cảm biến trong không khí sạch...\r\n");
    printf("Đang đo R0 trong 30 giây...\r\n");
    
    float mq137_r0_sum = 0;
    float mq135_r0_sum = 0;
    int valid_samples = 0;
    
    for(int i = 0; i < 30; i++) {
        ProcessAllSensors();
        
        if(g_sensor_system.mq137.base.is_valid && g_sensor_system.mq135.base.is_valid) {
            mq137_r0_sum += g_sensor_system.mq137.base.resistance;
            mq135_r0_sum += g_sensor_system.mq135.base.resistance;
            valid_samples++;
        }
        
        printf("Mẫu %d: MQ137=%.1fΩ, MQ135=%.1fΩ\r\n", 
               i+1, g_sensor_system.mq137.base.resistance, g_sensor_system.mq135.base.resistance);
        
        HAL_Delay(1000);
    }
    
    if(valid_samples > 0) {
        float mq137_r0_avg = mq137_r0_sum / valid_samples;
        float mq135_r0_avg = mq135_r0_sum / valid_samples;
        
        // Cập nhật giá trị R0
        g_sensor_system.mq137.base.r0_value = mq137_r0_avg;
        g_sensor_system.mq135.base.r0_value = mq135_r0_avg;
        
        printf("\r\n=== KẾT QUẢ HIỆU CHUẨN ===\r\n");
        printf("MQ137 R0: %.1f Ω\r\n", mq137_r0_avg);
        printf("MQ135 R0: %.1f Ω\r\n", mq135_r0_avg);
        printf("Mẫu hợp lệ: %d/30\r\n", valid_samples);
        printf("\r\nCập nhật trong code:\r\n");
        printf("#define MQ137_R0 %.1ff\r\n", mq137_r0_avg);
        printf("#define MQ135_R0 %.1ff\r\n", mq135_r0_avg);
        printf("✓ Hiệu chuẩn thành công!\r\n");
    }
    else {
        printf("❌ Hiệu chuẩn thất bại - không có mẫu hợp lệ!\r\n");
    }
}

// =============================================================================
// GETTER FUNCTIONS
// =============================================================================

/**
 * @brief Lấy nồng độ NH3 hiện tại
 */
float GetNH3_PPM(void) {
    return g_sensor_system.mq137.base.is_valid ? g_sensor_system.mq137.filtered_ppm : 0.0f;
}

/**
 * @brief Lấy nồng độ CO2 hiện tại
 */
float GetCO2_PPM(void) {
    return g_sensor_system.mq135.base.is_valid ? g_sensor_system.mq135.filtered_ppm : 0.0f;
}

/**
 * @brief Lấy mức cảnh báo NH3
 */
AlarmLevel_t GetNH3AlarmLevel(void) {
    return g_sensor_system.mq137.alarm_level;
}

/**
 * @brief Lấy mức cảnh báo CO2
 */
AlarmLevel_t GetCO2AlarmLevel(void) {
    return g_sensor_system.mq135.alarm_level;
}

/**
 * @brief Lấy mức cảnh báo tổng thể
 */
AlarmLevel_t GetSystemAlarmLevel(void) {
    return g_sensor_system.system_alarm;
}

/**
 * @brief Lấy trạng thái hệ thống
 */
uint8_t GetSystemStatus(void) {
    return g_sensor_system.system_status;
}
/**
 * @brief Lưu dữ liệu vào buffer lịch sử
 */
void SaveToHistory(void) {
    history_buffer[history_index] = g_sensor_system;
    history_index = (history_index + 1) % HISTORY_SIZE;
}
/**
 * @brief Cập nhật trạng thái hệ thống
 */         

void UpdateSystemStatus(void) {
    uint8_t valid_sensors = 0;
    AlarmLevel_t max_alarm = ALARM_NORMAL;
    
    // Đếm cảm biến hợp lệ và tìm mức cảnh báo cao nhất
    if(g_sensor_system.mq137.base.is_enabled && g_sensor_system.mq137.base.is_valid) {
        valid_sensors++;
        if(g_sensor_system.mq137.alarm_level > max_alarm) {
            max_alarm = g_sensor_system.mq137.alarm_level;
        }
    }
    
    if(g_sensor_system.mq135.base.is_enabled && g_sensor_system.mq135.base.is_valid) {
        valid_sensors++;
        if(g_sensor_system.mq135.alarm_level > max_alarm) {
            max_alarm = g_sensor_system.mq135.alarm_level;
        }
    }
    
    // Cập nhật trạng thái hệ thống
    if(valid_sensors == 2) {
        g_sensor_system.system_status = 3; // Tối ưu
    } else if(valid_sensors == 1) {
        g_sensor_system.system_status = 2; // Tốt
    } else {
        g_sensor_system.system_status = 0; // Lỗi
        g_sensor_system.error_count++;
    }
    
    g_sensor_system.system_alarm = max_alarm;
}



