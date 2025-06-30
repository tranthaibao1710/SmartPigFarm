#ifndef SENSOR_SYSTEM_H
#define SENSOR_SYSTEM_H

#include "gas_sensor.h"
#define HISTORY_BUFFER_SIZE 60
#define MAIN_LOOP_DELAY 60000
// Function prototypes
void InitSensorSystem(void);
void ProcessAllSensors(void);
void DisplaySystemStatus(void);
void TestSensorSystem(void);
void CalibrateSensors(void);

// API functions
float GetNH3_PPM(void);
float GetCO2_PPM(void);
AlarmLevel_t GetNH3AlarmLevel(void);
AlarmLevel_t GetCO2AlarmLevel(void);
AlarmLevel_t GetSystemAlarmLevel(void);
uint8_t GetSystemStatus(void);

void UpdateSystemStatus(void);
void SaveToHistory(void);
#endif
