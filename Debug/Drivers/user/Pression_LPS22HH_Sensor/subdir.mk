################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Drivers/user/Pression_LPS22HH_Sensor/lps22hh_reg.c 

OBJS += \
./Drivers/user/Pression_LPS22HH_Sensor/lps22hh_reg.o 

C_DEPS += \
./Drivers/user/Pression_LPS22HH_Sensor/lps22hh_reg.d 


# Each subdirectory must supply rules for building sources it contributes
Drivers/user/Pression_LPS22HH_Sensor/%.o Drivers/user/Pression_LPS22HH_Sensor/%.su Drivers/user/Pression_LPS22HH_Sensor/%.cyclo: ../Drivers/user/Pression_LPS22HH_Sensor/%.c Drivers/user/Pression_LPS22HH_Sensor/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m7 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F746xx -c -I../Core/Inc -I../Utilities/Fonts -I../Drivers/BSP/STM32746G-Discovery -I../Drivers/BSP/Components -I../Drivers/STM32F7xx_HAL_Driver/Inc -I../Drivers/STM32F7xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F7xx/Include -I../Drivers/CMSIS/Include -I../Drivers/user -I../Drivers/user/Temperature_and_Humidity_HTS221_Sensors -I../Drivers/user/Pression_LPS22HH_Sensor -I../Drivers/user/Anenometer_Sensor -I../Drivers/user/Girouette_sensor -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Drivers-2f-user-2f-Pression_LPS22HH_Sensor

clean-Drivers-2f-user-2f-Pression_LPS22HH_Sensor:
	-$(RM) ./Drivers/user/Pression_LPS22HH_Sensor/lps22hh_reg.cyclo ./Drivers/user/Pression_LPS22HH_Sensor/lps22hh_reg.d ./Drivers/user/Pression_LPS22HH_Sensor/lps22hh_reg.o ./Drivers/user/Pression_LPS22HH_Sensor/lps22hh_reg.su

.PHONY: clean-Drivers-2f-user-2f-Pression_LPS22HH_Sensor

