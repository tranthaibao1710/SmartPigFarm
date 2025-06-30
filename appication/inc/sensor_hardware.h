/*
 * sensor_hardware.h
 *
 *  Created on: Jun 30, 2025
 *      Author: FPTSHOP
 */

#ifndef INC_SENSOR_HARDWARE_H_
#define INC_SENSOR_HARDWARE_H_
#include "sensor_config.h"
#include "sensor_types.h"
// Hàm đọc dữ liệu phần cứng
float ReadADC_Voltage(uint32_t channel);
uint8_t ReadDigitalState(GPIO_TypeDef* port, uint16_t pin);

// Hàm tính toán cơ bản
float CalculateResistance(float voltage);
float CalculateGasPPM(float rs_r0_ratio, float curve_a, float curve_b);
AlarmLevel_t DetermineAlarmLevel(float ppm, float low, float high, float danger);

#endif /* INC_SENSOR_HARDWARE_H_ */
