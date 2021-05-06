################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (9-2020-q2-update)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Src/lib/cube_f767/Middlewares/ST/STM32_USB_Device_Library/Class/CDC/Src/usbd_cdc.c 

OBJS += \
./Src/lib/cube_f767/Middlewares/ST/STM32_USB_Device_Library/Class/CDC/Src/usbd_cdc.o 

C_DEPS += \
./Src/lib/cube_f767/Middlewares/ST/STM32_USB_Device_Library/Class/CDC/Src/usbd_cdc.d 


# Each subdirectory must supply rules for building sources it contributes
Src/lib/cube_f767/Middlewares/ST/STM32_USB_Device_Library/Class/CDC/Src/usbd_cdc.o: ../Src/lib/cube_f767/Middlewares/ST/STM32_USB_Device_Library/Class/CDC/Src/usbd_cdc.c Src/lib/cube_f767/Middlewares/ST/STM32_USB_Device_Library/Class/CDC/Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m7 -std=gnu11 -g3 -DSTM32F767xx -c -I"C:/Users/YoungSik/STM32CubeIDE/workspace_1.6.0/stm32f767zi_boot/Src" -I"C:/Users/YoungSik/STM32CubeIDE/workspace_1.6.0/stm32f767zi_boot/Src/lib/cube_f767/Middlewares/ST/STM32_USB_Device_Library/Class/CDC/Inc" -I"C:/Users/YoungSik/STM32CubeIDE/workspace_1.6.0/stm32f767zi_boot/Src/lib/cube_f767/Middlewares/ST/STM32_USB_Device_Library/Core/Inc" -I"C:/Users/YoungSik/STM32CubeIDE/workspace_1.6.0/stm32f767zi_boot/Src/lib/cube_f767/USB_DEVICE/App" -I"C:/Users/YoungSik/STM32CubeIDE/workspace_1.6.0/stm32f767zi_boot/Src/lib/cube_f767/USB_DEVICE/Target" -I"C:/Users/YoungSik/STM32CubeIDE/workspace_1.6.0/stm32f767zi_boot/Src/lib/cube_f767/Drivers/CMSIS/Device/ST/STM32F7xx/Include" -I"C:/Users/YoungSik/STM32CubeIDE/workspace_1.6.0/stm32f767zi_boot/Src/lib/cube_f767/Drivers/STM32F7xx_HAL_Driver/Inc" -I"C:/Users/YoungSik/STM32CubeIDE/workspace_1.6.0/stm32f767zi_boot/Src/lib/cube_f767/Drivers/CMSIS/Include" -I"C:/Users/YoungSik/STM32CubeIDE/workspace_1.6.0/stm32f767zi_boot/Src/BSP" -I"C:/Users/YoungSik/STM32CubeIDE/workspace_1.6.0/stm32f767zi_boot/Startup" -I"C:/Users/YoungSik/STM32CubeIDE/workspace_1.6.0/stm32f767zi_boot/Inc" -Os -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"Src/lib/cube_f767/Middlewares/ST/STM32_USB_Device_Library/Class/CDC/Src/usbd_cdc.d" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"

