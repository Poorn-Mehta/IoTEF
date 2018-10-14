################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../platform/emdrv/sleep/src/sleep.c 

OBJS += \
./platform/emdrv/sleep/src/sleep.o 

C_DEPS += \
./platform/emdrv/sleep/src/sleep.d 


# Each subdirectory must supply rules for building sources it contributes
platform/emdrv/sleep/src/sleep.o: ../platform/emdrv/sleep/src/sleep.c
	@echo 'Building file: $<'
	@echo 'Invoking: GNU ARM C Compiler'
	arm-none-eabi-gcc -g -gdwarf-2 -mcpu=cortex-m4 -mthumb -std=c99 '-DEFR32BG13P632F512GM48=1' '-D__STACK_SIZE=0x800' '-D__StackLimit=0x20000000' '-DHAL_CONFIG=1' '-D__HEAP_SIZE=0xD00' -I"C:\Users\poorn\SimplicityStudio\v4_workspace\soc-thermometer" -I"C:\Users\poorn\SimplicityStudio\v4_workspace\soc-thermometer\platform\Device\SiliconLabs\EFR32BG13P\Include" -I"C:\Users\poorn\SimplicityStudio\v4_workspace\soc-thermometer\platform\CMSIS\Include" -I"C:\Users\poorn\SimplicityStudio\v4_workspace\soc-thermometer\platform\emlib\inc" -I"C:\Users\poorn\SimplicityStudio\v4_workspace\soc-thermometer\protocol\bluetooth\ble_stack\inc\common" -I"C:\Users\poorn\SimplicityStudio\v4_workspace\soc-thermometer\platform\radio\rail_lib\chip\efr32" -I"C:\Users\poorn\SimplicityStudio\v4_workspace\soc-thermometer\platform\emdrv\uartdrv\inc" -I"C:\Users\poorn\SimplicityStudio\v4_workspace\soc-thermometer\hardware\kit\common\drivers" -I"C:\Users\poorn\SimplicityStudio\v4_workspace\soc-thermometer\platform\bootloader\api" -I"C:\Users\poorn\SimplicityStudio\v4_workspace\soc-thermometer\platform\emdrv\sleep\src" -I"C:\Users\poorn\SimplicityStudio\v4_workspace\soc-thermometer\platform\emlib\src" -I"C:\Users\poorn\SimplicityStudio\v4_workspace\soc-thermometer\hardware\kit\common\halconfig" -I"C:\Users\poorn\SimplicityStudio\v4_workspace\soc-thermometer\hardware\kit\common\bsp" -I"C:\Users\poorn\SimplicityStudio\v4_workspace\soc-thermometer\platform\Device\SiliconLabs\EFR32BG13P\Source" -I"C:\Users\poorn\SimplicityStudio\v4_workspace\soc-thermometer\hardware\kit\EFR32BG13_BRD4104A\config" -I"C:\Users\poorn\SimplicityStudio\v4_workspace\soc-thermometer\protocol\bluetooth\ble_stack\inc\soc" -I"C:\Users\poorn\SimplicityStudio\v4_workspace\soc-thermometer\platform\radio\rail_lib\common" -I"C:\Users\poorn\SimplicityStudio\v4_workspace\soc-thermometer\platform\emdrv\gpiointerrupt\inc" -I"C:\Users\poorn\SimplicityStudio\v4_workspace\soc-thermometer\platform\emdrv\common\inc" -I"C:\Users\poorn\SimplicityStudio\v4_workspace\soc-thermometer\platform\emdrv\sleep\inc" -I"C:\Users\poorn\SimplicityStudio\v4_workspace\soc-thermometer\platform\Device\SiliconLabs\EFR32BG13P\Source\GCC" -I"C:\Users\poorn\SimplicityStudio\v4_workspace\soc-thermometer\platform\halconfig\inc\hal-config" -I"C:\Users\poorn\SimplicityStudio\v4_workspace\soc-thermometer\app\bluetooth\common\stack_bridge" -I"C:\Users\poorn\SimplicityStudio\v4_workspace\soc-thermometer\platform\bootloader" -O2 -Wall -c -fmessage-length=0 -ffunction-sections -fdata-sections -mfpu=fpv4-sp-d16 -mfloat-abi=softfp -MMD -MP -MF"platform/emdrv/sleep/src/sleep.d" -MT"platform/emdrv/sleep/src/sleep.o" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


