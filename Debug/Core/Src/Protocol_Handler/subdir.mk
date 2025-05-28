################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Src/Protocol_Handler/protocol_handler.c 

OBJS += \
./Core/Src/Protocol_Handler/protocol_handler.o 

C_DEPS += \
./Core/Src/Protocol_Handler/protocol_handler.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/Protocol_Handler/%.o Core/Src/Protocol_Handler/%.su Core/Src/Protocol_Handler/%.cyclo: ../Core/Src/Protocol_Handler/%.c Core/Src/Protocol_Handler/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F411xE -c -I../Core/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -I../USB_DEVICE/App -I../USB_DEVICE/Target -I../Middlewares/ST/STM32_USB_Device_Library/Core/Inc -I../Middlewares/ST/STM32_USB_Device_Library/Class/CDC/Inc -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-Src-2f-Protocol_Handler

clean-Core-2f-Src-2f-Protocol_Handler:
	-$(RM) ./Core/Src/Protocol_Handler/protocol_handler.cyclo ./Core/Src/Protocol_Handler/protocol_handler.d ./Core/Src/Protocol_Handler/protocol_handler.o ./Core/Src/Protocol_Handler/protocol_handler.su

.PHONY: clean-Core-2f-Src-2f-Protocol_Handler

