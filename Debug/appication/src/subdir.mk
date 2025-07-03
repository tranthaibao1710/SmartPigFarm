################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (12.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../appication/src/gas_sensor.c \
../appication/src/sensor_hardware.c \
../appication/src/sensor_system.c 

OBJS += \
./appication/src/gas_sensor.o \
./appication/src/sensor_hardware.o \
./appication/src/sensor_system.o 

C_DEPS += \
./appication/src/gas_sensor.d \
./appication/src/sensor_hardware.d \
./appication/src/sensor_system.d 


# Each subdirectory must supply rules for building sources it contributes
appication/src/%.o appication/src/%.su appication/src/%.cyclo: ../appication/src/%.c appication/src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m3 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F103xB -c -I../Drivers/STM32F1xx_HAL_Driver/Inc/Legacy -I"C:/BaooHCMUT/TTNT/SmartPigFarm/Drivers/hardware/inc" -I"C:/BaooHCMUT/TTNT/SmartPigFarm/appication/inc" -I"C:/BaooHCMUT/TTNT/SmartPigFarm/Drivers/hardware/inc" -I../Drivers/CMSIS/Device/ST/STM32F1xx/Include -I../Drivers/CMSIS/Include -I"C:/BaooHCMUT/TTNT/SmartPigFarm/Core/Inc" -I"C:/BaooHCMUT/TTNT/SmartPigFarm/Drivers/STM32F1xx_HAL_Driver/Inc" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"

clean: clean-appication-2f-src

clean-appication-2f-src:
	-$(RM) ./appication/src/gas_sensor.cyclo ./appication/src/gas_sensor.d ./appication/src/gas_sensor.o ./appication/src/gas_sensor.su ./appication/src/sensor_hardware.cyclo ./appication/src/sensor_hardware.d ./appication/src/sensor_hardware.o ./appication/src/sensor_hardware.su ./appication/src/sensor_system.cyclo ./appication/src/sensor_system.d ./appication/src/sensor_system.o ./appication/src/sensor_system.su

.PHONY: clean-appication-2f-src

