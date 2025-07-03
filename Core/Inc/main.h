/* =============================================================================
 * MAIN.H - MAIN HEADER FILE
 * Hệ thống giám sát khí MQ137 & MQ135 - Modular Design
 * Phiên bản: 3.0
 * =============================================================================
 */

#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C"
{
#endif

// =============================================================================
// INCLUDES
// =============================================================================
#include "stm32f1xx_hal.h"            // HAL library chính cho STM32F1xx

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>                    // printf, sprintf...
#include <stdlib.h>                   // malloc, free...
#include <string.h>                   // memset, strcpy...
#include <stdint.h>                   // uint8_t, uint16_t...
#include <math.h>                     // pow, log, sqrt...
#include "uart.h"              // UART functions



  // =============================================================================
  // EXPORTED TYPES
  // =============================================================================

  // =============================================================================
  // EXPORTED CONSTANTS
  // =============================================================================

  // =============================================================================
  // EXPORTED MACRO
  // =============================================================================


  // =============================================================================
  // EXPORTED VARIABLES
  // =============================================================================


  // =============================================================================
  // EXPORTED FUNCTIONS PROTOTYPES
  // =============================================================================

  // System Functions
  void Error_Handler(void);
  // Hardware Initialization Functions
  void SystemClock_Config(void);
  // Application Functions
  int main(void);

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
