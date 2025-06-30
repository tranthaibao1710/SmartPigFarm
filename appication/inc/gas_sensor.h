/*
 * gas_sensor.h
 *
 *  Created on: Jun 30, 2025
 *      Author: FPTSHOP
 */

#ifndef INC_GAS_SENSOR_H_
#define INC_GAS_SENSOR_H_

#ifndef GAS_SENSOR_H
#define GAS_SENSOR_H

#include "sensor_hardware.h"

// Struct definitions

// Function prototypes
void InitMQ137Sensor(GasSensor_t* sensor);
void InitMQ135Sensor(GasSensor_t* sensor);
void ProcessGasSensor(GasSensor_t* sensor);
uint8_t ValidateGasSensor(GasSensor_t* sensor);
void UpdateAlarmLevel(GasSensor_t* sensor);
void PrintSensorDetails(GasSensor_t* sensor);

#endif

#endif /* INC_GAS_SENSOR_H_ */
