#include "stm32f103xb.h"
#include <stdint.h>
#include "gpio.h"
#ifndef HARDWARE_INC_ADC_H_
#define HARDWARE_INC_ADC_H_

#define ADC_Channel_0                               ((uint8_t)0x00)
#define ADC_Channel_1                               ((uint8_t)0x01)
#define ADC_Channel_2                               ((uint8_t)0x02)
#define ADC_Channel_3                               ((uint8_t)0x03)
#define ADC_Channel_4                               ((uint8_t)0x04)
#define ADC_Channel_5                               ((uint8_t)0x05)
#define ADC_Channel_6                               ((uint8_t)0x06)
#define ADC_Channel_7                               ((uint8_t)0x07)
#define ADC_Channel_8                               ((uint8_t)0x08)
#define ADC_Channel_9                               ((uint8_t)0x09)
#define ADC_Channel_10                              ((uint8_t)0x0A)
#define ADC_Channel_11                              ((uint8_t)0x0B)
#define ADC_Channel_12                              ((uint8_t)0x0C)
#define ADC_Channel_13                              ((uint8_t)0x0D)
#define ADC_Channel_14                              ((uint8_t)0x0E)
#define ADC_Channel_15                              ((uint8_t)0x0F)
#define ADC_Channel_16                              ((uint8_t)0x10)
#define ADC_Channel_17                              ((uint8_t)0x11)
/*	
 * Pinout
 *	CHANNEL				ADC1	ADC2	ADC3
 *	0					PA0		PA0		PA0
 *	1					PA1		PA1		PA1
 *	2					PA2		PA2		PA2
 *	3					PA3		PA3		PA3
 *	4					PA4		PA4		PF6
 *	5					PA5		PA5		PF7
 *	6					PA6		PA6		PF8
 *	7					PA7		PA7		PF9
 *	8					PB0		PB0		PF10
 *	9					pB1		PB1		PF3
 *	10				    PC0		PC0		PC0
 *	11					PC1		PC1		PC1
 *	12					PC2		PC2		PC2
 *	13					PC3		PC3		PC3
 *	14					PC4		PC4		PF4
 *	15					PC5		PC5		PF5
 */
void ADCx_Init(ADC_TypeDef *ADCx  ,uint8_t Channel);
uint16_t ADCx_Read(ADC_TypeDef *ADCx , uint8_t Channel );       
uint16_t ADCx_Read_TB(ADC_TypeDef *ADCx , uint8_t Channel , uint8_t n );

#endif /* HARDWARE_INC_ADC_H_ */
