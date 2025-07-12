#include "i2c.h"
/* I2C1 init function */
void I2Cx_Init(I2C_TypeDef * I2Cx, I2C_Pin Pins, uint32_t Speed)
{
	uint32_t PCLKx=(SystemCoreClock >> APBPrescTable[(RCC->CFGR & RCC_CFGR_PPRE1) >> RCC_CFGR_PPRE1_Pos]);
	
  /* I2C clock enable */
		if (I2Cx == I2C1) {
				RCC->APB1ENR |= (1 << 21); // B?t clock cho I2C1
		} else if (I2Cx == I2C2) {
				RCC->APB1ENR |= (1 << 22); // B?t clock cho I2C2
		}
	

		if(Pins==Pin_PB6PB7){		////I2C1
			GPIOx_Init(GPIOB, 6,OUTPUT_AF_OD,NOPULL,MODE_OUTPUT_50MHZ);
			GPIOx_Init(GPIOB, 7,OUTPUT_AF_OD,NOPULL,MODE_OUTPUT_50MHZ);
		}
		else if(Pins==Pin_PB8PB9){//I2C1
			AFIO->MAPR |= 1<<1;// REMAP
			GPIOx_Init(GPIOB, 8,OUTPUT_AF_OD,NOPULL,MODE_OUTPUT_50MHZ);
			GPIOx_Init(GPIOB, 9,OUTPUT_AF_OD,NOPULL,MODE_OUTPUT_50MHZ);
		}
		
		
		else if(Pins==Pin_PB10PB11){//I2C2
			GPIOx_Init(GPIOB, 10,OUTPUT_AF_OD,NOPULL,MODE_OUTPUT_50MHZ);
			GPIOx_Init(GPIOB, 11,OUTPUT_AF_OD,NOPULL,MODE_OUTPUT_50MHZ);
		}
		
		I2Cx->CR2 =  PCLKx/1000000;// Peripheral clock frequency = 36M
		I2Cx->CCR =  PCLKx/(Speed*2); 
		I2Cx->TRISE = (PCLKx/1000000)+1;
		I2Cx->CR1 |=(0<<1)|(1<<0) ; //en 
	}

	void I2Cx_ReadMulti(I2C_TypeDef *I2Cx, uint8_t DevAddress, uint8_t add, uint8_t * data, uint8_t size){
		I2Cx->CR1 |= (1<<8) |(1<<10);// Start generation + ACK
		while(!(I2Cx->SR1&(1<<0)));
		
		I2Cx->DR = (DevAddress<<1)|0;
		while(!(I2Cx->SR1&(1<<1))  |  !(I2Cx->SR2&(1<<1)));
		
		I2Cx->DR = add;
		while(!(I2Cx->SR1&(1<<7)));
		
		I2Cx->CR1 |= (1<<8) ;// reStart generation 
		while(!(I2Cx->SR1&(1<<0)));
		
		I2Cx->DR = (DevAddress<<1)|1;
		while(!(I2Cx->SR1&(1<<1))  |  !(I2Cx->SR2&(1<<1)));
		
		for(int i=0; i<size; i++){
		while(!(I2Cx->SR1&(1<<6)));// RXE
		data[i] = I2Cx->DR;
		}
		
		I2Cx->CR1 &= ~(1<<10);// nACK
		I2Cx->CR1 |= (1<<9) ;// Stop 
		
	}
	void I2Cx_WriteMulti(I2C_TypeDef *I2Cx, uint8_t DevAddress, uint8_t add, uint8_t * data, uint8_t size){

	I2Cx->CR1 |= (1<<8) |(1<<10);// Start generation + ACK
	while(!(I2Cx->SR1&(1<<0)));
	
	I2Cx->DR = (DevAddress<<1)|0;
	while(!(I2Cx->SR1&(1<<1))  |  !(I2Cx->SR2&(1<<1)));
	
	I2Cx->DR = add;
	while(!(I2Cx->SR1&(1<<7)));// TXE
	
	for(int i=0; i<size; i++){
	I2Cx->DR = data[i];
	while(!(I2Cx->SR1&(1<<7)));// TXE
	}
	
	I2Cx->CR1 &= ~(1<<10);// nACK
	I2Cx->CR1 |= (1<<9) ;// Stop 
}
uint8_t BCDtoBIN(uint8_t bcd){
return 10*(bcd>>4) + (bcd&0x0f);

}

uint8_t BINtoBCD(uint8_t bin){
return ((bin/10)<<4) + (bin%10);
}


uint8_t RTC_CheckMinMax(uint8_t val, uint8_t min, uint8_t max) {
	if (val < min) {
		return min;
	} else if (val > max) {
		return max;
	}
	return val;
}

void DS3231Read(uint8_t *HH, uint8_t *MM, uint8_t *SS,uint8_t *Date ,uint8_t *D,uint8_t *M,uint8_t *Y){
uint8_t data[7];
	
I2Cx_ReadMulti(I2C1, 0x68, 0x00, data, 7);
	
	*HH=BCDtoBIN(data[2]); *MM=BCDtoBIN(data[1]); *SS=BCDtoBIN(data[0]); *Date =BCDtoBIN(data[3]);*D=BCDtoBIN(data[4]);*M=BCDtoBIN(data[5]);*Y=BCDtoBIN(data[6]);
	
}
void DS3231Set(uint8_t HH, uint8_t MM, uint8_t SS,uint8_t Date ,uint8_t D,uint8_t M,uint8_t Y){

	uint8_t data[7];
	
	data[0] = BINtoBCD(RTC_CheckMinMax(SS, 0, 59)); 
	data[1] = BINtoBCD(RTC_CheckMinMax(MM, 0, 59));
	data[2] = BINtoBCD(RTC_CheckMinMax(HH, 0, 23));
	data[3] = BINtoBCD(RTC_CheckMinMax(Date, 1, 7));
	data[4] = BINtoBCD(RTC_CheckMinMax(D, 1, 31));
	data[5] = BINtoBCD(RTC_CheckMinMax(M, 1, 12));
	data[6] = BINtoBCD(RTC_CheckMinMax(Y, 0, 99));
	
	I2Cx_WriteMulti(I2C1, 0x68, 0x00, data,7);
}
