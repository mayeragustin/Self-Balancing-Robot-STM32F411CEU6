################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Src/ADC/ADC_handler.c 

OBJS += \
./Core/Src/ADC/ADC_handler.o 

C_DEPS += \
./Core/Src/ADC/ADC_handler.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/ADC/%.o Core/Src/ADC/%.su Core/Src/ADC/%.cyclo: ../Core/Src/ADC/%.c Core/Src/ADC/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F411xE -c -I../Core/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -I../USB_DEVICE/App -I../USB_DEVICE/Target -I../Middlewares/ST/STM32_USB_Device_Library/Core/Inc -I../Middlewares/ST/STM32_USB_Device_Library/Class/CDC/Inc -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-Src-2f-ADC

clean-Core-2f-Src-2f-ADC:
	-$(RM) ./Core/Src/ADC/ADC_handler.cyclo ./Core/Src/ADC/ADC_handler.d ./Core/Src/ADC/ADC_handler.o ./Core/Src/ADC/ADC_handler.su

.PHONY: clean-Core-2f-Src-2f-ADC

