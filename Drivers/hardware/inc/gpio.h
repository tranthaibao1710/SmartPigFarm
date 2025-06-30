/*
 * gpio.h
 *
 *  Created on: Jun 28, 2025
 *      Author: FPTSHOP
 */

#ifndef HARDWARE_INC_GPIO_H_
#define HARDWARE_INC_GPIO_H_

#include "stm32f103xb.h"
#include <stdint.h>
/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#define INPUT_ANALOG           0x00
#define INPUT_FLOATING         0x01
#define INPUT_PUPD             0x02
#define OUTPUT_PP              0x00
#define OUTPUT_OD              0x01
#define OUTPUT_AF_PP           0x02
#define OUTPUT_AF_OD           0x03
#define MODE_INPUT             0x00
#define MODE_OUTPUT_10MHZ      0x01
#define MODE_OUTPUT_2MHZ       0x02
#define MODE_OUTPUT_50MHZ      0x03
#define NOPULL                 0x00
#define PU                     0x01
#define PD                     0x02
/* GPIO Ports Clock Configuration                       */
#define _HAL_RCC_AFIO_CLK_ENABLE()        (RCC->APB2ENR |=  (1 << 0) )
#define _HAL_RCC_GPIOA_CLK_ENABLE()       (RCC->APB2ENR |=  (1 << 2) )
#define _HAL_RCC_GPIOB_CLK_ENABLE()       (RCC->APB2ENR |=  (1 << 3) )
#define _HAL_RCC_GPIOC_CLK_ENABLE()       (RCC->APB2ENR |=  (1 << 4) )
#define _HAL_RCC_GPIOD_CLK_ENABLE()       (RCC->APB2ENR |=  (1 << 5) )
#define _HAL_RCC_GPIOE_CLK_ENABLE()       (RCC->APB2ENR |=  (1 << 6) )

#define _HAL_RCC_GPIOA_CLK_DISABLE()       (RCC->APB2ENR &=  ~(1 << 2) )
#define _HAL_RCC_GPIOB_CLK_DISABLE()       (RCC->APB2ENR &=  ~(1 << 3) )
#define _HAL_RCC_GPIOC_CLK_DISABLE()       (RCC->APB2ENR &=  ~(1 << 4) )
#define _HAL_RCC_GPIOD_CLK_DISABLE()       (RCC->APB2ENR &=  ~(1 << 5) )
#define _HAL_RCC_GPIOE_CLK_DISABLE()       (RCC->APB2ENR &=  ~(1 << 6) )


void NVICx_Init(IRQn_Type IRQn, uint32_t PreemptPriority, uint32_t SubPriority);
void GPIOx_Init(GPIO_TypeDef *GPIOx ,uint8_t Pin ,uint8_t Mode ,uint8_t Pull,uint8_t Speed);
void GPIOx_WritePin(GPIO_TypeDef *GPIOx ,uint8_t Pin ,uint8_t bit);
uint8_t GPIOx_ReadPin(GPIO_TypeDef *GPIOx ,uint8_t Pin );   
#endif /* HARDWARE_INC_GPIO_H_ */
