#ifndef SENSOR_SYSTEM_H
#define SENSOR_SYSTEM_H

#include "gas_sensor.h"
#define HISTORY_BUFFER_SIZE 60
#define MAIN_LOOP_DELAY 20000
// Function prototypes
void InitSensorSystem(void);
void ProcessAllSensors(void);
void DisplaySystemStatus(void);
void TestSensorSystem(void);
void CalibrateSensors(void);
uint8_t CheckCalibrationConditions(void);
uint8_t ValidateCalibrationSample(float resistance, float voltage); 
void SetManualR0(float mq137_r0, float mq135_r0);
uint8_t ValidateR0Value(float r0_value, float stability, const char* sensor_name) ;
float CalculateCV(float* data, int count);
float CalculateAverage(float* data, int count) ;
void TestCalibrationResult(void) ;
void SaveCalibrationToFlash(float mq137_r0, float mq135_r0);
void QuickCalibrateSensors(void) ;
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
