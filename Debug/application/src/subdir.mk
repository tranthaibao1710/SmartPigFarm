################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (12.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../application/src/sensor_devices.c \
../application/src/sensor_hardware.c \
../application/src/sensor_system.c 

OBJS += \
./application/src/sensor_devices.o \
./application/src/sensor_hardware.o \
./application/src/sensor_system.o 

C_DEPS += \
./application/src/sensor_devices.d \
./application/src/sensor_hardware.d \
./application/src/sensor_system.d 


# Each subdirectory must supply rules for building sources it contributes
application/src/%.o application/src/%.su application/src/%.cyclo: ../application/src/%.c application/src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m3 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F103xB -c -I../Drivers/STM32F1xx_HAL_Driver/Inc/Legacy -I"C:/BaooHCMUT/TTNT/SmartPigFarm/Drivers/hardware/inc" -I"C:/BaooHCMUT/TTNT/SmartPigFarm/application/inc" -I../Drivers/CMSIS/Device/ST/STM32F1xx/Include -I../Drivers/CMSIS/Include -I"C:/BaooHCMUT/TTNT/SmartPigFarm/Core/Inc" -I"C:/BaooHCMUT/TTNT/SmartPigFarm/Drivers/STM32F1xx_HAL_Driver/Inc" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"

clean: clean-application-2f-src

clean-application-2f-src:
	-$(RM) ./application/src/sensor_devices.cyclo ./application/src/sensor_devices.d ./application/src/sensor_devices.o ./application/src/sensor_devices.su ./application/src/sensor_hardware.cyclo ./application/src/sensor_hardware.d ./application/src/sensor_hardware.o ./application/src/sensor_hardware.su ./application/src/sensor_system.cyclo ./application/src/sensor_system.d ./application/src/sensor_system.o ./application/src/sensor_system.su

.PHONY: clean-application-2f-src

