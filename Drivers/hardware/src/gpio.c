#include "gpio.h"


void NVICx_Init(IRQn_Type IRQn, uint32_t PreemptPriority, uint32_t SubPriority)
{ 
  uint32_t prioritygroup = 0x00U;
  
  prioritygroup = NVIC_GetPriorityGrouping();
  
  NVIC_SetPriority(IRQn, NVIC_EncodePriority(prioritygroup, PreemptPriority, SubPriority));
	
	 /* Enable interrupt */
  NVIC_EnableIRQ(IRQn);
}
void GPIOx_Init(GPIO_TypeDef *GPIOx ,uint8_t Pin ,uint8_t Mode ,uint8_t Pull,uint8_t Speed)
{ 
if(GPIOx == GPIOA ) RCC ->APB2ENR |= 1<<2 ;
if(GPIOx == GPIOB ) RCC ->APB2ENR |= 1<<3 ;
if(GPIOx == GPIOC ) RCC ->APB2ENR |= 1<<4 ;
if(GPIOx == GPIOD ) RCC ->APB2ENR |= 1<<5 ;
if(GPIOx == GPIOE ) RCC ->APB2ENR |= 1<<6 ;
	if(Pin<8){
	GPIOx->CRL &=~(0xF<<(Pin*4)) ;
	GPIOx->CRL |=(((Mode<<2)+ Speed) <<(Pin*4)) ; 
	}
	else{
	GPIOx->CRH &=~(0xF<<((Pin-8)*4)) ;
	GPIOx->CRH |=(((Mode<<2)+ Speed) <<((Pin-8)*4)) ;
	}
	if(Pull ==PU)
  GPIOx->ODR |= 1<<Pin ;
  else GPIOx->ODR &= ~(1<<Pin); 	
}
void GPIOx_WritePin(GPIO_TypeDef *GPIOx ,uint8_t Pin ,uint8_t bit)
{
	if (bit==1) GPIOx->BSRR |= 1 << Pin ;
	else GPIOx->BSRR |= 1 << (Pin + 16);
}
uint8_t GPIOx_ReadPin(GPIO_TypeDef *GPIOx ,uint8_t Pin )
{
	return ((GPIOx->IDR)&(1<<Pin))==0 ? 0:1 ; 
}
void GPIOx_TogglePin(GPIO_TypeDef *GPIOx ,uint8_t Pin )
{
	GPIOx->ODR^=(1<<Pin);

}