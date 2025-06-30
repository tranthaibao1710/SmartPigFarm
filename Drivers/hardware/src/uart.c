/*
 * uart.c
 *
 *  Created on: Jun 28, 2025
 *      Author: FPTSHOP
 */

#include "uart.h"
void USARTx_Init(USART_TypeDef * USARTx,USART_Pin Pin,uint32_t baud)
{
	  IRQn_Type IRQn  ; 
	  uint8_t u = 0 ; 
	  uint32_t PCLKx = 0 ; 
	  RCC->APB2ENR |=1<<0 ; 
    float USARTDIV = 0.0 ;
    uint16_t mantisa = 0 ,fraction= 0 ;  
	
		if(USARTx == USART1 ){PCLKx =(HAL_RCC_GetHCLKFreq() >> APBPrescTable[(RCC->CFGR & RCC_CFGR_PPRE2) >> RCC_CFGR_PPRE1_Pos]); RCC->APB2ENR|=1<<14;IRQn = USART1_IRQn ; u=1;}
		if(USARTx == USART2 ){PCLKx =(HAL_RCC_GetHCLKFreq() >> APBPrescTable[(RCC->CFGR & RCC_CFGR_PPRE1) >> RCC_CFGR_PPRE1_Pos]); RCC->APB1ENR|=1<<17;IRQn = USART2_IRQn ; u=2;} 
		if(USARTx == USART3 ){PCLKx =(HAL_RCC_GetHCLKFreq() >> APBPrescTable[(RCC->CFGR & RCC_CFGR_PPRE1) >> RCC_CFGR_PPRE1_Pos]); RCC->APB1ENR|=1<<18;IRQn = USART3_IRQn ; u=3;}
	if(Pin==PA9PA10){//usart1
			GPIOx_Init(GPIOA,9, OUTPUT_AF_PP,NOPULL, MODE_OUTPUT_50MHZ);//tx
			GPIOx_Init(GPIOA,10, INPUT_FLOATING,NOPULL, 0);//rx
		 }
	if(Pin==PB6PB7){//usart1
			AFIO->MAPR|=1<<2;
			GPIOx_Init(GPIOB,6, OUTPUT_AF_PP,NOPULL, MODE_OUTPUT_50MHZ);//tx
			GPIOx_Init(GPIOB,7, INPUT_FLOATING,NOPULL, 0);//rx
		 }

		if(Pin==PA2PA3){//usart2
		GPIOx_Init(GPIOA,2, OUTPUT_AF_PP,NOPULL, MODE_OUTPUT_50MHZ);//tx
		GPIOx_Init(GPIOA,3, INPUT_FLOATING,NOPULL, 0);//rx
	 }
		if(Pin==PB10PB11){//usart3
		GPIOx_Init(GPIOB,10, OUTPUT_AF_PP,NOPULL, MODE_OUTPUT_50MHZ);//tx
		GPIOx_Init(GPIOB,11, INPUT_FLOATING,NOPULL, 0);//rx
	 }
		USARTDIV = (float)(PCLKx/(16.0*baud)) ;
	  mantisa  = (uint16_t)USARTDIV ; 
    fraction = (USARTDIV - mantisa)*16 ; 	
		USARTx->BRR = (mantisa <<4)+fraction ; 
    USARTx->CR1 |=1<<2;//tx
	  USARTx->CR1 |=1<<3;//rx
	  USARTx->CR1 |=1<<13 ; //enable usart
    USARTx->CR1 |=1<<5; //it
    NVICx_Init(IRQn, 1, u);
}


#define USART1_BUFFER_SIZE 100
#define USART2_BUFFER_SIZE 100
#define USART3_BUFFER_SIZE 100
char USART1_BUFFER[USART1_BUFFER_SIZE];
char USART2_BUFFER[USART2_BUFFER_SIZE];
char USART3_BUFFER[USART3_BUFFER_SIZE];

USART_ST USART1_ST  ={USART1_BUFFER,USART1_BUFFER_SIZE , 0, 0, 0} ;
USART_ST USART2_ST  ={USART2_BUFFER,USART2_BUFFER_SIZE , 0, 0, 0} ;
USART_ST USART3_ST  ={USART3_BUFFER,USART3_BUFFER_SIZE , 0, 0, 0} ;
void USARTtoBUFF(USART_ST *u , char c){
	if(u->in<u->size){
		 u->buffer[u->in] = c;
		 u->in++;
		 u->num++;
		if(u->in==u->size) u->in = 0 ;
	}
}
char USARTx_GetC(USART_TypeDef * USARTx){
	USART_ST *u ;
char c=0 ;	
   if(USARTx==USART1)u = &USART1_ST ; 
   if(USARTx==USART2)u = &USART2_ST ; 
   if(USARTx==USART3)u = &USART3_ST ; 
		if(u->num>0){
		c=u->buffer[u->out]; 
		u->out++;
		u->num--;
		if(u->out==u->size) u->out = 0 ; 
	}
		return c ; 
}

uint16_t USARTx_isEMPTY(USART_TypeDef * USARTx){
	USART_ST *u ;
	
   if(USARTx==USART1) u = &USART1_ST ; 
   if(USARTx==USART2)u = &USART2_ST ; 
   if(USARTx==USART3)u = &USART3_ST ; 
   return u->num ; 
}

uint16_t USARTx_GetS(USART_TypeDef * USARTx, char * str, uint16_t len){
    uint16_t i = 0; 
    char c = 0;
    
    // B? ki?m tra empty ? d?u - d? d?c liên t?c
    while(i < len - 1){  // Ð? ch? cho null terminator
        
        // Ki?m tra có data không
        if(USARTx_isEMPTY(USARTx) == 0) {
            break;  // Không có data thì thoát
        }
        
        c = USARTx_GetC(USARTx); 
        if(c) {
            str[i] = c;
            if(str[i] == '\n') {
                i++; 
                break;
            }
            else {
                i++;
            }
        }
        else {
            break;  // ? THÊM ELSE d? tránh treo
        }
    }
    
    // Thêm null terminator
    if(i < len) str[i] = '\0';
    
    return i; 
}
void USART_PutC(USART_TypeDef * USARTx, char c) {
    while(!(USARTx->SR & (1<<7)));  // Ð?i d?n khi TXE = 1
    USARTx->DR = c;
}
void USART_PutS(USART_TypeDef * USARTx ,char *str ){

 while(*str) USART_PutC(USARTx ,*str ++) ;

}
void USART1_IRQHandler(void)
{
    if(USART1->SR & (1<<5)) {           // ? USART1 check USART1
        char data = USART1->DR;                // ? Ð?c t? USART1
        USARTtoBUFF(&USART1_ST, data);         // ? Luu vào buffer USART1
    }
}

void USART2_IRQHandler(void)
{
    if(USART2->SR & (1<<5)) {           // ? USART2 check USART2  
        char data = USART2->DR;                // ? Ð?c t? USART2
        USARTtoBUFF(&USART2_ST, data);         // ? Luu vào buffer USART2
    }
}

void USART3_IRQHandler(void)
{
    if(USART3->SR & (1<<5)) {           // ? USART3 dã dúng
        char data = USART3->DR;                // ? Ð?c t? USART3
        USARTtoBUFF(&USART3_ST, data);         // ? Luu vào buffer USART3
    }
}