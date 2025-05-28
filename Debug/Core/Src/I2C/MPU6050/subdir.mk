################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Src/I2C/MPU6050/mpu6050.c 

OBJS += \
./Core/Src/I2C/MPU6050/mpu6050.o 

C_DEPS += \
./Core/Src/I2C/MPU6050/mpu6050.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/I2C/MPU6050/%.o Core/Src/I2C/MPU6050/%.su Core/Src/I2C/MPU6050/%.cyclo: ../Core/Src/I2C/MPU6050/%.c Core/Src/I2C/MPU6050/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F411xE -c -I../Core/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -I../USB_DEVICE/App -I../USB_DEVICE/Target -I../Middlewares/ST/STM32_USB_Device_Library/Core/Inc -I../Middlewares/ST/STM32_USB_Device_Library/Class/CDC/Inc -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-Src-2f-I2C-2f-MPU6050

clean-Core-2f-Src-2f-I2C-2f-MPU6050:
	-$(RM) ./Core/Src/I2C/MPU6050/mpu6050.cyclo ./Core/Src/I2C/MPU6050/mpu6050.d ./Core/Src/I2C/MPU6050/mpu6050.o ./Core/Src/I2C/MPU6050/mpu6050.su

.PHONY: clean-Core-2f-Src-2f-I2C-2f-MPU6050

