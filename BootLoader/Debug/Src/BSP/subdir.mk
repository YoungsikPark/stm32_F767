################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (9-2020-q2-update)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Src/BSP/stm32f7xx_hal_msp.c \
../Src/BSP/stm32f7xx_it.c \
../Src/BSP/system_stm32f7xx.c 

OBJS += \
./Src/BSP/stm32f7xx_hal_msp.o \
./Src/BSP/stm32f7xx_it.o \
./Src/BSP/system_stm32f7xx.o 

C_DEPS += \
./Src/BSP/stm32f7xx_hal_msp.d \
./Src/BSP/stm32f7xx_it.d \
./Src/BSP/system_stm32f7xx.d 


# Each subdirectory must supply rules for building sources it contributes
Src/BSP/stm32f7xx_hal_msp.o: ../Src/BSP/stm32f7xx_hal_msp.c Src/BSP/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m7 -std=gnu11 -g3 -DSTM32F767xx -c -I"C:/Users/YoungSik/STM32CubeIDE/workspace_1.6.0/stm32f767zi_boot/Src" -I"C:/Users/YoungSik/STM32CubeIDE/workspace_1.6.0/stm32f767zi_boot/Src/lib/cube_f767/Drivers/CMSIS/Device/ST/STM32F7xx/Include" -I"C:/Users/YoungSik/STM32CubeIDE/workspace_1.6.0/stm32f767zi_boot/Src/lib/cube_f767/Drivers/STM32F7xx_HAL_Driver/Inc" -I"C:/Users/YoungSik/STM32CubeIDE/workspace_1.6.0/stm32f767zi_boot/Src/lib/cube_f767/Drivers/CMSIS/Include" -I"C:/Users/YoungSik/STM32CubeIDE/workspace_1.6.0/stm32f767zi_boot/Src/BSP" -I"C:/Users/YoungSik/STM32CubeIDE/workspace_1.6.0/stm32f767zi_boot/Startup" -I"C:/Users/YoungSik/STM32CubeIDE/workspace_1.6.0/stm32f767zi_boot/Inc" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"Src/BSP/stm32f7xx_hal_msp.d" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"
Src/BSP/stm32f7xx_it.o: ../Src/BSP/stm32f7xx_it.c Src/BSP/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m7 -std=gnu11 -g3 -DSTM32F767xx -c -I"C:/Users/YoungSik/STM32CubeIDE/workspace_1.6.0/stm32f767zi_boot/Src" -I"C:/Users/YoungSik/STM32CubeIDE/workspace_1.6.0/stm32f767zi_boot/Src/lib/cube_f767/Drivers/CMSIS/Device/ST/STM32F7xx/Include" -I"C:/Users/YoungSik/STM32CubeIDE/workspace_1.6.0/stm32f767zi_boot/Src/lib/cube_f767/Drivers/STM32F7xx_HAL_Driver/Inc" -I"C:/Users/YoungSik/STM32CubeIDE/workspace_1.6.0/stm32f767zi_boot/Src/lib/cube_f767/Drivers/CMSIS/Include" -I"C:/Users/YoungSik/STM32CubeIDE/workspace_1.6.0/stm32f767zi_boot/Src/BSP" -I"C:/Users/YoungSik/STM32CubeIDE/workspace_1.6.0/stm32f767zi_boot/Startup" -I"C:/Users/YoungSik/STM32CubeIDE/workspace_1.6.0/stm32f767zi_boot/Inc" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"Src/BSP/stm32f7xx_it.d" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"
Src/BSP/system_stm32f7xx.o: ../Src/BSP/system_stm32f7xx.c Src/BSP/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m7 -std=gnu11 -g3 -DSTM32F767xx -c -I"C:/Users/YoungSik/STM32CubeIDE/workspace_1.6.0/stm32f767zi_boot/Src" -I"C:/Users/YoungSik/STM32CubeIDE/workspace_1.6.0/stm32f767zi_boot/Src/lib/cube_f767/Drivers/CMSIS/Device/ST/STM32F7xx/Include" -I"C:/Users/YoungSik/STM32CubeIDE/workspace_1.6.0/stm32f767zi_boot/Src/lib/cube_f767/Drivers/STM32F7xx_HAL_Driver/Inc" -I"C:/Users/YoungSik/STM32CubeIDE/workspace_1.6.0/stm32f767zi_boot/Src/lib/cube_f767/Drivers/CMSIS/Include" -I"C:/Users/YoungSik/STM32CubeIDE/workspace_1.6.0/stm32f767zi_boot/Src/BSP" -I"C:/Users/YoungSik/STM32CubeIDE/workspace_1.6.0/stm32f767zi_boot/Startup" -I"C:/Users/YoungSik/STM32CubeIDE/workspace_1.6.0/stm32f767zi_boot/Inc" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"Src/BSP/system_stm32f7xx.d" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"

