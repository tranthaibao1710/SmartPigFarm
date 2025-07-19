#include "adc.h"

void ADCx_Init(ADC_TypeDef *ADCx, uint8_t Channel) {
    // Enable ADC clock
    if(ADCx == ADC1) RCC->APB2ENR |= 1<<9;
    if(ADCx == ADC2) RCC->APB2ENR |= 1<<10;
    
    // Configure GPIO pins for analog input
    if(Channel == ADC_Channel_0) GPIOx_Init(GPIOA, 0, INPUT_ANALOG, NOPULL, 0);
    if(Channel == ADC_Channel_1) GPIOx_Init(GPIOA, 1, INPUT_ANALOG, NOPULL, 0);
    if(Channel == ADC_Channel_2) GPIOx_Init(GPIOA, 2, INPUT_ANALOG, NOPULL, 0);
    if(Channel == ADC_Channel_3) GPIOx_Init(GPIOA, 3, INPUT_ANALOG, NOPULL, 0);
    if(Channel == ADC_Channel_4) GPIOx_Init(GPIOA, 4, INPUT_ANALOG, NOPULL, 0);
    if(Channel == ADC_Channel_5) GPIOx_Init(GPIOA, 5, INPUT_ANALOG, NOPULL, 0);
    if(Channel == ADC_Channel_6) GPIOx_Init(GPIOA, 6, INPUT_ANALOG, NOPULL, 0);
    if(Channel == ADC_Channel_7) GPIOx_Init(GPIOA, 7, INPUT_ANALOG, NOPULL, 0);
    
    // ADC Configuration
    ADCx->CR1 |= 1<<8;              // SCAN MODE enable
    ADCx->CR2 |= (1<<1) | (1<<0);   // CONT=1, ADON=1 (Continuous mode)
    ADCx->CR2 |= 1<<20;             // EXTEN[1:0] = 00 (External trigger disabled)
    ADCx->CR2 &= ~(1<<11);          // ALIGN=0 (Right alignment)
    ADCx->CR2 &= ~(0x7<<17);        // Clear EXTSEL bits
    ADCx->CR2 |= (0x7<<17);         // EXTSEL = 111 (SWSTART)
    
    // Set sequence length to 1 conversion
    ADCx->SQR1 &= ~(0xF<<20);       // Clear L[3:0]
    ADCx->SQR1 |= (0x0<<20);        // L=0 (1 conversion)
    
    // Set sampling time (longest for better accuracy)
    if(Channel > 9) {
        ADCx->SMPR1 &= ~(7<<((Channel-10)*3)); 
        ADCx->SMPR1 |= (7<<((Channel-10)*3));   // 480 cycles
    } else {
        ADCx->SMPR2 &= ~(7<<(Channel*3));
        ADCx->SMPR2 |= (7<<(Channel*3));        // 480 cycles
    }
    
    // Set first conversion in regular sequence
    ADCx->SQR3 &= 0xFFFFFFE0;       // Clear SQ1[4:0]
    ADCx->SQR3 |= Channel;          // Set channel for SQ1
    
    // ADC Calibration
    ADCx->CR2 |= 1<<3;              // RSTCAL=1 (Reset calibration)
    while(ADCx->CR2 & (1<<3));      // Wait for reset calibration to complete
    
    ADCx->CR2 |= 1<<2;              // CAL=1 (Start calibration)  
    while(ADCx->CR2 & (1<<2));      // Wait for calibration to complete
        
    // Enable ADC
    ADCx->CR2 |= (1<<22) | (1<<0);  // SWSTART=1, ADON=1
}

uint16_t ADCx_Read(ADC_TypeDef *ADCx, uint8_t Channel) {
    // Set sampling time for the channel
    if(Channel > 9) {
        ADCx->SMPR1 &= ~(7<<((Channel-10)*3)); 
        ADCx->SMPR1 |= (7<<((Channel-10)*3));
    } else {
        ADCx->SMPR2 &= ~(7<<(Channel*3));
        ADCx->SMPR2 |= (7<<(Channel*3));
    }
    
    // Set channel in regular sequence
    ADCx->SQR3 &= 0xFFFFFFE0;       
    ADCx->SQR3 |= Channel;
    
    // Start conversion
    ADCx->CR2 |= (1<<22) | (1<<0);  // SWSTART=1, ADON=1
    
    // Wait for conversion to complete
    while((ADCx->SR & (1<<1)) == 0);
    
    // Clear EOC flag by reading DR
    return ADCx->DR;
}

uint16_t ADCx_Read_TB(ADC_TypeDef *ADCx, uint8_t Channel, uint8_t n) {
    if(n == 0) return 0;            // Validation
    
    uint32_t sum = 0;               // Dùng uint32_t d? tránh overflow
    
    // ? Lo?i b? m?t s? m?u d?u d? ADC ?n d?nh
    for(int i = 0; i < 3; i++) {
        ADCx_Read(ADCx, Channel);
        for(volatile int d = 0; d < 500; d++); // Delay
    }
    
    // Ð?c n m?u
    for (int i = 0; i < n; i++) {
        sum += ADCx_Read(ADCx, Channel);
        for (volatile int d = 0; d < 1000; d++); // Delay gi?a các l?n d?c
    }
    
    return (uint16_t)(sum / n);     // Tr? v? trung bình
}

// ✅ Hàm đọc điện áp đã sửa
