/*
 * sensor_hardware.c
 *
 *  Created on: Jun 30, 2025
 *      Author: FPTSHOP
 */
#include "sensor_hardware.h"
float ReadADC_Voltage(uint32_t channel) {
    /*ADC_ChannelConfTypeDef sConfig = {0};
    sConfig.Channel = channel;
    sConfig.Rank = ADC_REGULAR_RANK_1;
    sConfig.SamplingTime = ADC_SAMPLETIME_55CYCLES_5;
    
    if(HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK) {
        return 0.0f;
    }
    
    // Đọc nhiều lần và lấy trung bình để giảm nhiễu
    uint32_t adc_sum = 0;
    for(int i = 0; i < 10; i++) {
        HAL_ADC_Start(&hadc1);
        HAL_ADC_PollForConversion(&hadc1, 100);
        adc_sum += HAL_ADC_GetValue(&hadc1);
        HAL_ADC_Stop(&hadc1);
        HAL_Delay(1); // Delay nhỏ giữa các lần đo
    }
    
    uint32_t adc_avg = adc_sum / 10;
    
    // Chuyển đổi giá trị ADC sang điện áp thực tế
    float adc_voltage = (float)adc_avg * ADC_VREF / ADC_RESOLUTION;
    
    // Hiệu chỉnh cho mạch chia áp để có điện áp thực của cảm biến
    float sensor_voltage = adc_voltage / VOLTAGE_DIVIDER_RATIO;
    
    return sensor_voltage;*/
}

/**
 * @brief Đọc trạng thái digital
 */
uint8_t ReadDigitalState(GPIO_TypeDef* port, uint16_t pin) {
    return HAL_GPIO_ReadPin(port, pin);
}

/**
 * @brief Tính điện trở cảm biến (hiệu chỉnh cho nguồn 5V)
 */
float CalculateResistance(float voltage) {
    if(voltage <= 0.1f) return 0.0f;
    
    // Công thức tính điện trở cảm biến với nguồn cấp 5V
    // Rs = (Vcc - Vs) / Vs * RL
    // Trong đó: Vcc = 5V, Vs = điện áp đo được, RL = điện trở tải
    return ((SENSOR_VCC - voltage) / voltage) * LOAD_RESISTANCE;
}

/**
 * @brief Tính nồng độ khí từ tỷ lệ Rs/R0
 */
float CalculateGasPPM(float rs_r0_ratio, float curve_a, float curve_b) {
    if(rs_r0_ratio <= 0) return 0.0f;
    return curve_a * powf(rs_r0_ratio, curve_b);
}

/**
 * @brief Xác định mức cảnh báo
 */
AlarmLevel_t DetermineAlarmLevel(float ppm, float low, float high, float danger) {
    if(ppm >= danger) return ALARM_DANGER;
    if(ppm >= high) return ALARM_HIGH;
    if(ppm >= low) return ALARM_LOW;
    return ALARM_NORMAL;
}

