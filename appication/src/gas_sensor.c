/*
 * gas_sensor.c
 *
 *  Created on: Jun 30, 2025
 *      Author: FPTSHOP
 */
#include "gas_sensor.h"
void InitMQ137Sensor(GasSensor_t* sensor) {
    // Cấu hình base sensor
    sensor->base.sensor_id = 0;
    sensor->base.sensor_type = SENSOR_TYPE_MQ137;
    strcpy(sensor->base.sensor_name, "MQ137");
    sensor->base.is_enabled = 1;
    sensor->base.adc_channel = MQ137_ADC_CHANNEL;
    sensor->base.digital_port = MQ137_DIGITAL_PORT;
    sensor->base.digital_pin = MQ137_DIGITAL_PIN;
    sensor->base.r0_value = 10000.0f; // Giá trị mặc định, cần hiệu chuẩn
    
    // Cấu hình gas sensor
    sensor->gas_type = GAS_TYPE_NH3;
    strcpy(sensor->gas_name, "NH3");
    strcpy(sensor->unit, "ppm");
    
    // Ngưỡng cảnh báo NH3
    sensor->threshold_low = 15.0f;
    sensor->threshold_high = 25.0f;
    sensor->threshold_danger = 50.0f;
    
    // Thông số đặc tuyến MQ137 cho NH3
    sensor->curve_a = -0.263f;
    sensor->curve_b = 0.42f;
    sensor->min_ppm = 0.0f;
    sensor->max_ppm = 500.0f;
    
    // Khởi tạo giá trị
    sensor->alarm_level = ALARM_NORMAL;
}

/**
 * @brief Khởi tạo cảm biến MQ136 (H2S)
 */
void InitMQ136Sensor(GasSensor_t* sensor) {
    // Cấu hình base sensor
    sensor->base.sensor_id = 1;
    sensor->base.sensor_type = SENSOR_TYPE_MQ136;
    strcpy(sensor->base.sensor_name, "MQ136");
    sensor->base.is_enabled = 1;
    sensor->base.adc_channel = MQ136_ADC_CHANNEL;
    sensor->base.digital_port = MQ136_DIGITAL_PORT;
    sensor->base.digital_pin = MQ136_DIGITAL_PIN;
    sensor->base.r0_value = 10000.0f; // Giá trị mặc định, cần hiệu chuẩn
    
    // Cấu hình gas sensor
    sensor->gas_type = GAS_TYPE_H2S;
    strcpy(sensor->gas_name, "H2S");
    strcpy(sensor->unit, "ppm");
    
    // Ngưỡng cảnh báo CO2
    sensor->threshold_low = MQ136_THRESHOLD_LOW;
    sensor->threshold_high = MQ136_THRESHOLD_HIGH;
    sensor->threshold_danger = MQ136_THRESHOLD_DANGER;
    
    // Thông số đặc tuyến MQ135 cho CO2
    sensor->curve_a = MQ136_CURVE_A;
    sensor->curve_b = MQ136_CURVE_B;
    sensor->min_ppm = MQ136_MIN_PPM;
    sensor->max_ppm = MQ136_MAX_PPM;
    
    // Khởi tạo giá trị
    sensor->alarm_level = ALARM_NORMAL;
}
void ProcessGasSensor(GasSensor_t* sensor) {

    sensor->base.raw_voltage = ReadADC_Voltage(sensor->base.adc_channel);
    sensor->base.digital_state = ReadDigitalState(sensor->base.digital_port, sensor->base.digital_pin);
    sensor->base.timestamp = HAL_GetTick();

    // Tính toán điện trở và tỷ lệ Rs/R0
    if(sensor->base.raw_voltage > 0.2f) { // Ngưỡng thấp hơn cho ADC 3.3V
        sensor->base.resistance = CalculateResistance(sensor->base.raw_voltage);
        sensor->base.rs_r0_ratio = sensor->base.resistance / sensor->base.r0_value;

        // Tính nồng độ khí
        sensor->gas_ppm = CalculateGasPPM(sensor->base.rs_r0_ratio, sensor->curve_a, sensor->curve_b);

        // Giới hạn giá trị trong khoảng hợp lệ
        if(sensor->gas_ppm < sensor->min_ppm) sensor->gas_ppm = sensor->min_ppm;
        if(sensor->gas_ppm > sensor->max_ppm) sensor->gas_ppm = sensor->max_ppm;

        // Lọc nhiễu đơn giản (moving average)
        sensor->filtered_ppm = 0.8f * sensor->filtered_ppm + 0.2f * sensor->gas_ppm;
        sensor->average_ppm = sensor->filtered_ppm;

    } else {
        sensor->base.resistance = 0.0f;
        sensor->base.rs_r0_ratio = 0.0f;
        sensor->gas_ppm = sensor->min_ppm;
        sensor->filtered_ppm = sensor->min_ppm;
    }

    // Validate dữ liệu
    sensor->base.is_valid = ValidateGasSensor(sensor);

    // Cập nhật mức cảnh báo
    UpdateAlarmLevel(sensor);
}

/**
 * @brief Validate dữ liệu cảm biến (cập nhật cho điện áp 3.3V)
 */
uint8_t ValidateGasSensor(GasSensor_t* sensor) {
    // Kiểm tra điện áp trong khoảng hợp lệ (0.1V - 4.8V cho cảm biến 5V)
    // Sau khi qua mạch chia áp, tín hiệu sẽ từ 0.066V - 3.168V tại ADC
    if(sensor->base.raw_voltage < 0.2f || sensor->base.raw_voltage > 4.8f) {
        return 0;
    }
    
    // Kiểm tra điện trở trong khoảng hợp lệ
    if(sensor->base.resistance < 1000.0f || sensor->base.resistance > 200000.0f) {
        return 0;
    }
    
    // Kiểm tra nồng độ khí trong khoảng hợp lệ
    if(sensor->gas_ppm < sensor->min_ppm || sensor->gas_ppm > sensor->max_ppm) {
        return 0;
    }
    
    return 1; // Dữ liệu hợp lệ
}

/**
 * @brief Cập nhật mức cảnh báo cho cảm biến
 */
void UpdateAlarmLevel(GasSensor_t* sensor) {
    sensor->alarm_level = DetermineAlarmLevel(sensor->gas_ppm, 
                                            sensor->threshold_low,
                                            sensor->threshold_high, 
                                            sensor->threshold_danger);
}
void PrintSensorDetails(GasSensor_t* sensor) {
    printf("\r\n--- %s (%s) ---\r\n", sensor->base.sensor_name, sensor->gas_name);
    printf("ID: %d | TRANG THAI: %s\r\n", 
           sensor->base.sensor_id, 
           sensor->base.is_enabled ? (sensor->base.is_valid ? "HOAT DONG" : "LOI") : "OFF");
    printf("DIEN AP: %.3f V | DIEN TRO: %.1f Ω\r\n", 
           sensor->base.raw_voltage, sensor->base.resistance);
    printf("Rs/R0: %.3f | R0: %.1f Ω\r\n", 
           sensor->base.rs_r0_ratio, sensor->base.r0_value);
    printf("NONG DO: %.1f %s \r\n", 
           sensor->gas_ppm, sensor->unit);
    printf("Digital: %s\r\n", sensor->base.digital_state ? "HIGH" : "LOW");
    
    const char* alarm_text[] = {"BINH THUONG", "THAP", "CAO", "NGUY HIEM"};
    printf("CANH BAO: %s (%.1f/%.1f/%.1f)\r\n", 
           alarm_text[sensor->alarm_level],
           sensor->threshold_low, sensor->threshold_high, sensor->threshold_danger);
}


