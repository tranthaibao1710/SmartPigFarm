################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (12.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/application/src/sensor_core.c \
../Core/application/src/sensor_devices.c \
../Core/application/src/sensor_system.c 

OBJS += \
./Core/application/src/sensor_core.o \
./Core/application/src/sensor_devices.o \
./Core/application/src/sensor_system.o 

C_DEPS += \
./Core/application/src/sensor_core.d \
./Core/application/src/sensor_devices.d \
./Core/application/src/sensor_system.d 


# Each subdirectory must supply rules for building sources it contributes
Core/application/src/%.o Core/application/src/%.su Core/application/src/%.cyclo: ../Core/application/src/%.c Core/application/src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m3 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F103xB -c -I../Drivers/STM32F1xx_HAL_Driver/Inc/Legacy -I"C:/BaooHCMUT/TTNT/SmartPigFarm/Drivers/hardware/inc" -I../Drivers/CMSIS/Device/ST/STM32F1xx/Include -I../Drivers/CMSIS/Include -I"C:/BaooHCMUT/TTNT/SmartPigFarm/Core/Inc" -I"C:/BaooHCMUT/TTNT/SmartPigFarm/Drivers/STM32F1xx_HAL_Driver/Inc" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"

clean: clean-Core-2f-application-2f-src

clean-Core-2f-application-2f-src:
	-$(RM) ./Core/application/src/sensor_core.cyclo ./Core/application/src/sensor_core.d ./Core/application/src/sensor_core.o ./Core/application/src/sensor_core.su ./Core/application/src/sensor_devices.cyclo ./Core/application/src/sensor_devices.d ./Core/application/src/sensor_devices.o ./Core/application/src/sensor_devices.su ./Core/application/src/sensor_system.cyclo ./Core/application/src/sensor_system.d ./Core/application/src/sensor_system.o ./Core/application/src/sensor_system.su

.PHONY: clean-Core-2f-application-2f-src

