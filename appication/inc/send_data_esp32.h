/*
 * send_data_esp32.h
 *
 *  Created on: Jul 29, 2025
 *      Author: FPTSHOP
 */

#ifndef INC_SEND_DATA_ESP32_H_
#define INC_SEND_DATA_ESP32_H_
#include "sensor_system.h"
void CreateJSONString(void);
uint8_t SendJSONData(void); 
void SendCommandToESP32(const char* command);
uint16_t ReadESP32Response(void);
void ProcessESP32Response(char* response);
void SendDataToESP32(void);
void SendAlarmUpdate(void);
void UpdateMainLoopWithESP32(void);
void CheckESP32Connection(void);
void SendHeartbeat(void);
void HandleDeviceControl(void);  

#endif /* INC_SEND_DATA_ESP32_H_ */
