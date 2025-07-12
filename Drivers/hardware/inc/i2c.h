/*
 * i2c.h
 *
 *  Created on: Jul 11, 2025
 *      Author: FPTSHOP
 */

#ifndef HARDWARE_INC_I2C_H_
#define HARDWARE_INC_I2C_H_
#include "adc.h"
typedef enum{
Pin_PB6PB7, //i2c1
Pin_PB8PB9,//i2c1
Pin_PB10PB11,//i2c2	
}I2C_Pin;

void I2Cx_Init(I2C_TypeDef * I2Cx, I2C_Pin Pins, uint32_t Speed);
void I2Cx_ReadMulti(I2C_TypeDef *I2Cx, uint8_t DevAddress, uint8_t add, uint8_t * data, uint8_t size);
void I2Cx_WriteMulti(I2C_TypeDef *I2Cx, uint8_t DevAddress, uint8_t add, uint8_t * data, uint8_t size);
uint8_t BCDtoBIN(uint8_t bcd);
uint8_t BINtoBCD(uint8_t bin);
uint8_t RTC_CheckMinMax(uint8_t val, uint8_t min, uint8_t max) ;
void DS3231Read(uint8_t *HH, uint8_t *MM, uint8_t *SS,uint8_t *Date ,uint8_t *D,uint8_t *M,uint8_t *Y)  ;
void DS3231Set(uint8_t HH, uint8_t MM, uint8_t SS,uint8_t Date ,uint8_t D,uint8_t M,uint8_t Y);
#endif /* HARDWARE_INC_I2C_H_ */
