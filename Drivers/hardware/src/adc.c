#include "adc.h"

void ADCx_Init(ADC_TypeDef *ADCx  ,uint8_t Channel){
if(ADCx==ADC1) RCC->APB2ENR |=1<<9;
if(ADCx==ADC2) RCC->APB2ENR |=1<<10;
	if(Channel == ADC_Channel_0) GPIOx_Init(GPIOA ,0 ,INPUT_ANALOG,NOPULL,0);
	if(Channel == ADC_Channel_1) GPIOx_Init(GPIOA ,1 ,INPUT_ANALOG,NOPULL,0);
	if(Channel == ADC_Channel_2) GPIOx_Init(GPIOA ,2 ,INPUT_ANALOG,NOPULL,0);
	if(Channel == ADC_Channel_3) GPIOx_Init(GPIOA ,3 ,INPUT_ANALOG,NOPULL,0);
	if(Channel == ADC_Channel_4) GPIOx_Init(GPIOA ,4 ,INPUT_ANALOG,NOPULL,0);
	if(Channel == ADC_Channel_5) GPIOx_Init(GPIOA ,5 ,INPUT_ANALOG,NOPULL,0);
	if(Channel == ADC_Channel_6) GPIOx_Init(GPIOA ,6 ,INPUT_ANALOG,NOPULL,0);
	if(Channel == ADC_Channel_7) GPIOx_Init(GPIOA ,7 ,INPUT_ANALOG,NOPULL,0);
	
	ADCx->CR1 |= 1<<8 ;  //SCAN MODE 
	ADCx->CR2 |= (1<<1) | (1<<0) ;  //CONTINUE MODE
	ADCx->CR2 |= 1<<20 ;
	ADCx->CR2 |= 0<<11 ;
	ADCx->CR2 &= ~(0x7<<17);
	ADCx->CR2 |=  (0x7<<17) ;//swstart 
	ADCx->SQR1 &=~(0xF<<20);
	ADCx->SQR1 |=(0x0<<20);
	if(Channel>9) {
		ADCx->SMPR1 &=~(7<<(Channel-10)*3); 
		ADCx->SMPR1 |=(7<<(Channel-10)*3) ; 
	}
  else
	{
	  ADCx->SMPR2 &=~(7<<(Channel)*3);
	  ADCx->SMPR2 |=(7<<(Channel)*3);
	}
	  ADCx->SQR3 &= 0xFFFFFFE0 ; 
	  ADCx->SQR3 |= Channel ;
	
	while(ADCx->CR2 & (1<<3 ))
	ADCx->CR2 |= 1<<2 ;
	while(ADCx->CR2 & (1<<2 ))
		
  ADCx->CR2 |= (1<<22)|(1<<0) ; //ENABLE

}
uint16_t ADCx_Read(ADC_TypeDef *ADCx , uint8_t Channel )
{
if(Channel>9) {
		ADCx->SMPR1 &=~(7<<(Channel-10)*3); 
		ADCx->SMPR1 |=(7<<(Channel-10)*3) ; 
	}
  else
	{
		ADCx->SMPR2 &=~(7<<(Channel)*3);
	    ADCx->SMPR2 |=(7<<(Channel)*3);
	}
	   ADCx->SQR3 &= 0xFFFFFFE0 ; 
	   ADCx->SQR3 |= Channel ;
       ADCx->CR2  |= (1<<22)|(1<<0) ;
		while ((ADCx->SR & (1<<1)) == 0);
		return ADCx->DR ;
}
uint16_t ADCx_Read_TB(ADC_TypeDef *ADCx , uint8_t Channel , uint8_t n ){
uint8_t sum = 0;
	for (int i=0;i<n;i++) sum += ADCx_Read(ADCx ,  Channel ) ;
	return sum / n  ;

}
