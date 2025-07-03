
#ifndef HARDWARE_INC_UART_H_
#define HARDWARE_INC_UART_H_
#include "adc.h"
#include "stm32f103xb.h"
#include <stdint.h>
#include <stdio.h>
typedef enum{
PA9PA10, //usart1
PB6PB7, // uart1
PA2PA3,//uasrt2
PB10PB11,//usart3	
}USART_Pin;

typedef struct
{
char * buffer ; 
uint16_t size ; 
uint16_t in ;
uint16_t out ; 
uint16_t num;
	
}USART_ST ;
void USARTx_Init(USART_TypeDef * USARTx,USART_Pin Pin,uint32_t baud);
void USARTtoBUFF(USART_ST *u , char c);
char USARTx_GetC(USART_TypeDef * USARTx);
uint16_t USARTx_GetS(USART_TypeDef * USARTx, char * str, uint16_t len);
uint16_t USARTx_isEMPTY(USART_TypeDef * USARTx);
void USART_PutC(USART_TypeDef * USARTx, char c);
void USART_PutS(USART_TypeDef * USARTx ,char *str );    
void USART1_IRQHandler(void);
void USART2_IRQHandler(void);   
void USART3_IRQHandler(void);   
int __io_putchar(int ch) ;  




#endif /* HARDWARE_INC_UART_H_ */
