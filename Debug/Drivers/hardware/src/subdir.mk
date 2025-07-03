################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (12.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Drivers/hardware/src/adc.c \
../Drivers/hardware/src/gpio.c \
../Drivers/hardware/src/uart.c 

OBJS += \
./Drivers/hardware/src/adc.o \
./Drivers/hardware/src/gpio.o \
./Drivers/hardware/src/uart.o 

C_DEPS += \
./Drivers/hardware/src/adc.d \
./Drivers/hardware/src/gpio.d \
./Drivers/hardware/src/uart.d 


# Each subdirectory must supply rules for building sources it contributes
Drivers/hardware/src/%.o Drivers/hardware/src/%.su Drivers/hardware/src/%.cyclo: ../Drivers/hardware/src/%.c Drivers/hardware/src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m3 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F103xB -c -I../Drivers/STM32F1xx_HAL_Driver/Inc/Legacy -I"C:/BaooHCMUT/TTNT/SmartPigFarm/Drivers/hardware/inc" -I"C:/BaooHCMUT/TTNT/SmartPigFarm/appication/inc" -I"C:/BaooHCMUT/TTNT/SmartPigFarm/Drivers/hardware/inc" -I../Drivers/CMSIS/Device/ST/STM32F1xx/Include -I../Drivers/CMSIS/Include -I"C:/BaooHCMUT/TTNT/SmartPigFarm/Core/Inc" -I"C:/BaooHCMUT/TTNT/SmartPigFarm/Drivers/STM32F1xx_HAL_Driver/Inc" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"

clean: clean-Drivers-2f-hardware-2f-src

clean-Drivers-2f-hardware-2f-src:
	-$(RM) ./Drivers/hardware/src/adc.cyclo ./Drivers/hardware/src/adc.d ./Drivers/hardware/src/adc.o ./Drivers/hardware/src/adc.su ./Drivers/hardware/src/gpio.cyclo ./Drivers/hardware/src/gpio.d ./Drivers/hardware/src/gpio.o ./Drivers/hardware/src/gpio.su ./Drivers/hardware/src/uart.cyclo ./Drivers/hardware/src/uart.d ./Drivers/hardware/src/uart.o ./Drivers/hardware/src/uart.su

.PHONY: clean-Drivers-2f-hardware-2f-src

