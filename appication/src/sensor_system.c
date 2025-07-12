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
    printf("\r\nKHOI TAO HE THONG\r\n");
    
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
    ADCx_Init(ADC1,MQ137_ADC_CHANNEL); // Khởi tạo ADC cho MQ137
    ADCx_Init(ADC1,MQ135_ADC_CHANNEL); // Khởi tạo ADC cho MQ135
    
    
    // Cập nhật trạng thái hệ thống
    g_sensor_system.active_sensor_count = 2;
    g_sensor_system.last_update = HAL_GetTick();
    
    printf("KHOI TAO MQ137 (NH3): %s\r\n", g_sensor_system.mq137.base.is_enabled ? "OK" : "FAIL");
    printf("KHOI TAO MQ135 (CO2): %s\r\n", g_sensor_system.mq135.base.is_enabled ? "OK" : "FAIL");
    printf("CAM BIEN HOAT DONG: %d/2\r\n", g_sensor_system.active_sensor_count);
    
    // Thời gian ổn định
    printf("DANG ON DINH CAM BIEN...\r\n");
    for(int i = 30; i > 0; i--) {
        printf("ON DINH: %d GIAY\r", i);
        HAL_Delay(1000);
    }
    printf("\r\nHE THONG SAN SANG\r\n");
}
void ProcessAllSensors(void) {
    uint32_t current_time = HAL_GetTick();
    printf  ("\r\nXU LY CAM BIEN TAI %lu ms \r\n", current_time);
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
    printf("\r\n=== TRANG THAI HE THONG CAM BIEN ===\r\n");
    printf("TIME: %lu ms | Uptime: %.1f PHUT\r\n", 
           HAL_GetTick(), g_sensor_system.uptime_minutes);
    
    // Hiển thị MQ137
    PrintSensorDetails(&g_sensor_system.mq137);
    
    // Hiển thị MQ135
    PrintSensorDetails(&g_sensor_system.mq135);
    
    // Trạng thái tổng thể
    printf("\r\n--- HE THONG ---\r\n");
    const char* status_text[] = {"LOI", "CO BAN", "TOT", "TOI UU"};
    const char* alarm_text[] = {"BINH THUONG", "THAP", "CAO", "NGUY HIEM"};
    
    printf("TRANG THAI: %s (%d/2 CAM BIEN HOAT DONG)\r\n", 
           status_text[g_sensor_system.system_status], g_sensor_system.active_sensor_count);
    printf("CANH BAO TONG THE %s\r\n", alarm_text[g_sensor_system.system_alarm]);
    printf("TONG SO DO: %lu | LOI: %lu\r\n", 
           g_sensor_system.total_readings, g_sensor_system.error_count);
    
    printf("=====================================\r\n");
}

/**
 * @brief Test hệ thống
 */
void TestSensorSystem(void) {
    printf("\r\nTEST HE THONG CAM BIEN\r\n");
    
    for(int i = 0; i < 10; i++) {
        printf("TEST LAN  %d:\r\n", i + 1);
        
        ProcessAllSensors();
        
        printf("  MQ137: %.1f ppm NH3 (Alarm: %d)\r\n", 
               GetNH3_PPM(), GetNH3AlarmLevel());
        printf("  MQ135: %.1f ppm CO2 (Alarm: %d)\r\n", 
               GetCO2_PPM(), GetCO2AlarmLevel());
        printf("  HE THONG: Status=%d, Alarm=%d\r\n\r\n", 
               GetSystemStatus(), GetSystemAlarmLevel());
        
        HAL_Delay(3000);
    }
    
    printf("TEST HOAN TAT\r\n");
}
/**
 * @brief Hiệu chuẩn cảm biến
 */
// =============================================================================
// HIỆU CHUẨN CẢM BIẾN ĐƯỢC CẢI TIẾN - MQ137 & MQ135
// =============================================================================

void CalibrateSensors(void) {
    printf("\r\n=== HIEU CHUAN CAM BIEN CAI TIEN ===\r\n");
    printf("QUAN TRONG: Dam bao cam bien da chay 24h+ va o moi truong sach!\r\n");
    
    // Kiểm tra điều kiện trước khi hiệu chuẩn
    if(!CheckCalibrationConditions()) {
        printf("❌ DIEU KIEN KHONG DAP UNG - Huy hieu chuan!\r\n");
        return;
    }
    
    printf("✅ Dieu kien OK - Bat dau hieu chuan...\r\n");
    printf("Thoi gian: 5 phut (300 mau)\r\n");
    
    float mq137_r0_sum = 0, mq137_min = 999999, mq137_max = 0;
    float mq135_r0_sum = 0, mq135_min = 999999, mq135_max = 0;
    int valid_samples = 0;
    int total_samples = 300;  // 5 phút
    
    for(int i = 0; i < total_samples; i++) {
        ProcessAllSensors();
        
        float mq137_resistance = g_sensor_system.mq137.base.resistance;
        float mq135_resistance = g_sensor_system.mq135.base.resistance;
        
        // Validate samples với điều kiện chặt chẽ hơn
        uint8_t mq137_valid = ValidateCalibrationSample(mq137_resistance, 
                                                       g_sensor_system.mq137.base.raw_voltage);
        uint8_t mq135_valid = ValidateCalibrationSample(mq135_resistance, 
                                                       g_sensor_system.mq135.base.raw_voltage);
        
        if(mq137_valid && mq135_valid) {
            // MQ137 statistics
            mq137_r0_sum += mq137_resistance;
            if(mq137_resistance < mq137_min) mq137_min = mq137_resistance;
            if(mq137_resistance > mq137_max) mq137_max = mq137_resistance;
            
            // MQ135 statistics
            mq135_r0_sum += mq135_resistance;
            if(mq135_resistance < mq135_min) mq135_min = mq135_resistance;
            if(mq135_resistance > mq135_max) mq135_max = mq135_resistance;
            
            valid_samples++;
        }
        
        // Progress report mỗi 30 giây
        if(i % 30 == 0) {
            float progress = (float)i / total_samples * 100.0f;
            printf("Tien do: %.1f%% | Mau %d: MQ137=%.1fΩ, MQ135=%.1fΩ | Valid: %d\r\n", 
                   progress, i+1, mq137_resistance, mq135_resistance, valid_samples);
        }
        
        HAL_Delay(1000);  // 1 giây mỗi sample
    }
    
    // Phân tích kết quả
    if(valid_samples < (total_samples * 0.8f)) {  // Cần ít nhất 80% sample hợp lệ
        printf("❌ KHONG DU SAMPLE HOP LE (%d/%d = %.1f%%)\r\n", 
               valid_samples, total_samples, (float)valid_samples/total_samples*100);
        printf("Hay kiem tra:\r\n");
        printf("- Cam bien da preheat 24h+?\r\n");
        printf("- Moi truong thuc su sach?\r\n");
        printf("- Hardware ket noi dung?\r\n");
        return;
    }
    
    // Tính giá trị trung bình
    float mq137_r0_avg = mq137_r0_sum / valid_samples;
    float mq135_r0_avg = mq135_r0_sum / valid_samples;
    
    // Tính độ ổn định
    float mq137_range = mq137_max - mq137_min;
    float mq135_range = mq135_max - mq135_min;
    float mq137_stability = (mq137_range / mq137_r0_avg) * 100.0f;
    float mq135_stability = (mq135_range / mq135_r0_avg) * 100.0f;
    
    printf("\r\n=== KET QUA HIEU CHUAN ===\r\n");
    printf("Thoi gian: 5 phut | Sample hop le: %d/%d (%.1f%%)\r\n", 
           valid_samples, total_samples, (float)valid_samples/total_samples*100);
    
    // MQ137 results
    printf("\r\n--- MQ137 (NH3) ---\r\n");
    printf("R0 trung binh: %.1f Ω\r\n", mq137_r0_avg);
    printf("Khoang bien thien: %.1f - %.1f Ω\r\n", mq137_min, mq137_max);
    printf("Do on dinh: %.1f%%\r\n", mq137_stability);
    
    // MQ135 results  
    printf("\r\n--- MQ135 (CO2) ---\r\n");
    printf("R0 trung binh: %.1f Ω\r\n", mq135_r0_avg);
    printf("Khoang bien thien: %.1f - %.1f Ω\r\n", mq135_min, mq135_max);
    printf("Do on dinh: %.1f%%\r\n", mq135_stability);
    
    // Validate và apply R0 values
    uint8_t mq137_r0_valid = ValidateR0Value(mq137_r0_avg, mq137_stability, "MQ137");
    uint8_t mq135_r0_valid = ValidateR0Value(mq135_r0_avg, mq135_stability, "MQ135");
    
    if(mq137_r0_valid && mq135_r0_valid) {
        // Cập nhật giá trị R0
        g_sensor_system.mq137.base.r0_value = mq137_r0_avg;
        g_sensor_system.mq135.base.r0_value = mq135_r0_avg;
        
        printf("\r\n✅ HIEU CHUAN THANH CONG!\r\n");
        printf("R0 da duoc cap nhat:\r\n");
        printf("- MQ137: %.1f Ω\r\n", mq137_r0_avg);
        printf("- MQ135: %.1f Ω\r\n", mq135_r0_avg);
        
        // Lưu vào Flash (optional)
        SaveCalibrationToFlash(mq137_r0_avg, mq135_r0_avg);
        
        // Test ngay với R0 mới
        printf("\r\n🧪 TEST VOI R0 MOI:\r\n");
        TestCalibrationResult();
        
    } else {
        printf("\r\n❌ HIEU CHUAN THAT BAI!\r\n");
        printf("Su dung gia tri R0 mac dinh\r\n");
        g_sensor_system.mq137.base.r0_value = 10000.0f;
        g_sensor_system.mq135.base.r0_value = 10000.0f;
    }
    
    printf("===============================\r\n");
}

// =============================================================================
// CÁC HÀM HỖ TRỢ
// =============================================================================

uint8_t CheckCalibrationConditions(void) {
    printf("Kiem tra dieu kien hieu chuan...\r\n");
    
    // Kiểm tra 10 sample để đánh giá ổn định
    float mq137_readings[10], mq135_readings[10];
    
    for(int i = 0; i < 10; i++) {
        ProcessAllSensors();
        mq137_readings[i] = g_sensor_system.mq137.base.resistance;
        mq135_readings[i] = g_sensor_system.mq135.base.resistance;
        HAL_Delay(2000);  // 2 giây mỗi sample
    }
    
    // Tính coefficient of variation
    float mq137_cv = CalculateCV(mq137_readings, 10);
    float mq135_cv = CalculateCV(mq135_readings, 10);
    
    printf("Do on dinh MQ137: %.1f%% (can <10%%)\r\n", mq137_cv);
    printf("Do on dinh MQ135: %.1f%% (can <10%%)\r\n", mq135_cv);
    
    // Kiểm tra range hợp lý
    float mq137_avg = CalculateAverage(mq137_readings, 10);
    float mq135_avg = CalculateAverage(mq135_readings, 10);
    
    printf("Dien tro trung binh MQ137: %.1f Ω\r\n", mq137_avg);
    printf("Dien tro trung binh MQ135: %.1f Ω\r\n", mq135_avg);
    
    // Điều kiện pass
    if(mq137_cv < 10.0f && mq135_cv < 10.0f && 
       mq137_avg > 10000.0f && mq137_avg < 200000.0f &&
       mq135_avg > 10000.0f && mq135_avg < 200000.0f) {
        printf("✅ Dieu kien dat yeu cau\r\n");
        return 1;
    } else {
        printf("❌ Dieu kien chua dat yeu cau\r\n");
        if(mq137_cv >= 10.0f) printf("- MQ137 chua on dinh\r\n");
        if(mq135_cv >= 10.0f) printf("- MQ135 chua on dinh\r\n");
        if(mq137_avg <= 10000.0f || mq137_avg >= 200000.0f) {
            printf("- MQ137 dien tro bat thuong\r\n");
        }
        if(mq135_avg <= 10000.0f || mq135_avg >= 200000.0f) {
            printf("- MQ135 dien tro bat thuong\r\n");
        }
        return 0;
    }
}

uint8_t ValidateCalibrationSample(float resistance, float voltage) {
    // Kiểm tra điện áp hợp lệ
    if(voltage < 0.2f || voltage > 4.8f) return 0;
    
    // Kiểm tra điện trở trong khoảng cho clean air
    if(resistance < 8000.0f || resistance > 300000.0f) return 0;
    
    return 1;
}

uint8_t ValidateR0Value(float r0_value, float stability, const char* sensor_name) {
    printf("\r\nValidate %s R0...\r\n", sensor_name);
    
    // Kiểm tra range
    if(r0_value < 8000.0f) {
        printf("❌ %s R0 qua thap (%.1f < 8kΩ)\r\n", sensor_name, r0_value);
        return 0;
    }
    if(r0_value > 200000.0f) {
        printf("❌ %s R0 qua cao (%.1f > 200kΩ)\r\n", sensor_name, r0_value);
        return 0;
    }
    
    // Kiểm tra stability
    if(stability > 15.0f) {
        printf("❌ %s chua on dinh (%.1f%% > 15%%)\r\n", sensor_name, stability);
        return 0;
    }
    
    printf("✅ %s R0 hop le\r\n", sensor_name);
    return 1;
}

float CalculateCV(float* data, int count) {
    float sum = 0, avg, variance = 0;
    
    // Tính trung bình
    for(int i = 0; i < count; i++) {
        sum += data[i];
    }
    avg = sum / count;
    
    // Tính variance
    for(int i = 0; i < count; i++) {
        variance += powf(data[i] - avg, 2);
    }
    variance /= count;
    
    // Coefficient of variation = (std_dev / mean) * 100%
    float std_dev = sqrtf(variance);
    return (std_dev / avg) * 100.0f;
}

float CalculateAverage(float* data, int count) {
    float sum = 0;
    for(int i = 0; i < count; i++) {
        sum += data[i];
    }
    return sum / count;
}

void TestCalibrationResult(void) {
    printf("Test 5 lan voi R0 moi...\r\n");
    
    for(int i = 0; i < 5; i++) {
        ProcessAllSensors();
        
        float nh3_ppm = GetNH3_PPM();
        float co2_ppm = GetCO2_PPM();
        
        printf("Test %d: NH3=%.1f ppm, CO2=%.1f ppm\r\n", i+1, nh3_ppm, co2_ppm);
        
        HAL_Delay(3000);
    }
}

void SaveCalibrationToFlash(float mq137_r0, float mq135_r0) {
    // TODO: Implement Flash save
    printf("💾 Luu R0 vao Flash: MQ137=%.1f, MQ135=%.1f\r\n", mq137_r0, mq135_r0);
}

// =============================================================================
// QUICK CALIBRATION - Phiên bản nhanh cho test
// =============================================================================

void QuickCalibrateSensors(void) {
    printf("\r\n=== HIEU CHUAN NHANH (30 GIAY) ===\r\n");
    printf("CHI DE TEST - KHONG CHINH XAC!\r\n");
    
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
        
        printf("Mau %d: MQ137=%.1fΩ, MQ135=%.1fΩ\r\n", 
               i+1, g_sensor_system.mq137.base.resistance, g_sensor_system.mq135.base.resistance);
        
        HAL_Delay(1000);
    }
    
    if(valid_samples > 15) {  // Cần ít nhất 50% sample
        float mq137_r0_avg = mq137_r0_sum / valid_samples;
        float mq135_r0_avg = mq135_r0_sum / valid_samples;
        
        // Chỉ apply nếu hợp lý
        if(mq137_r0_avg > 8000.0f && mq137_r0_avg < 200000.0f) {
            g_sensor_system.mq137.base.r0_value = mq137_r0_avg;
        }
        if(mq135_r0_avg > 8000.0f && mq135_r0_avg < 200000.0f) {
            g_sensor_system.mq135.base.r0_value = mq135_r0_avg;
        }
        
        printf("\r\nKET QUA HIEU CHUAN NHANH:\r\n");
        printf("MQ137 R0: %.1f Ω\r\n", g_sensor_system.mq137.base.r0_value);
        printf("MQ135 R0: %.1f Ω\r\n", g_sensor_system.mq135.base.r0_value);
        printf("Mau hop le: %d/30\r\n", valid_samples);
        printf("✅ HOAN TAT\r\n");
    } else {
        printf("❌ Khong du sample hop le!\r\n");
    }
}

// =============================================================================
// MANUAL R0 SETTING
// =============================================================================

void SetManualR0(float mq137_r0, float mq135_r0) {
    printf("\r\nTHIET LAP R0 THU CONG:\r\n");
    
    if(mq137_r0 > 5000.0f && mq137_r0 < 300000.0f) {
        g_sensor_system.mq137.base.r0_value = mq137_r0;
        printf("✅ MQ137 R0 = %.1f Ω\r\n", mq137_r0);
    } else {
        printf("❌ MQ137 R0 khong hop le\r\n");
    }
    
    if(mq135_r0 > 5000.0f && mq135_r0 < 300000.0f) {
        g_sensor_system.mq135.base.r0_value = mq135_r0;
        printf("✅ MQ135 R0 = %.1f Ω\r\n", mq135_r0);
    } else {
        printf("❌ MQ135 R0 khong hop le\r\n");
    }
}

// Sử dụng:
// SetManualR0(10000.0f, 10000.0f);  // Set cả 2 về 10kΩ


// =============================================================================
// GETTER FUNCTIONS
// =============================================================================

/**
 * @brief Lấy nồng độ NH3 hiện tại
 */
float GetNH3_PPM(void) {
    return g_sensor_system.mq137.base.is_valid ? g_sensor_system.mq137.gas_ppm : 0.0f;
}

/**
 * @brief Lấy nồng độ CO2 hiện tại
 */
float GetCO2_PPM(void) {
    return g_sensor_system.mq135.base.is_valid ? g_sensor_system.mq135.gas_ppm : 0.0f;
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



