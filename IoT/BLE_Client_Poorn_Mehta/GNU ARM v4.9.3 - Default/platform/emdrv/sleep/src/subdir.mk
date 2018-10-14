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
	arm-none-eabi-gcc -g -gdwarf-2 -mcpu=cortex-m4 -mthumb -std=c99 '-D__STACK_SIZE=0x800' '-DHAL_CONFIG=1' '-D__HEAP_SIZE=0xD00' '-D__StackLimit=0x20000000' '-DEFR32BG13P632F512GM48=1' -I"C:\Users\poorn\SimplicityStudio\v4_workspace\BLE_Client_Poorn_Mehta" -I"C:\Users\poorn\SimplicityStudio\v4_workspace\BLE_Client_Poorn_Mehta\platform\Device\SiliconLabs\EFR32BG13P\Include" -I"C:\Users\poorn\SimplicityStudio\v4_workspace\BLE_Client_Poorn_Mehta\platform\CMSIS\Include" -I"C:\Users\poorn\SimplicityStudio\v4_workspace\BLE_Client_Poorn_Mehta\platform\Device\SiliconLabs\EFR32BG13P\Source" -I"C:\Users\poorn\SimplicityStudio\v4_workspace\BLE_Client_Poorn_Mehta\platform\radio\rail_lib\common" -I"C:\Users\poorn\SimplicityStudio\v4_workspace\BLE_Client_Poorn_Mehta\platform\emlib\src" -I"C:\Users\poorn\SimplicityStudio\v4_workspace\BLE_Client_Poorn_Mehta\platform\emlib\inc" -I"C:\Users\poorn\SimplicityStudio\v4_workspace\BLE_Client_Poorn_Mehta\hardware\kit\common\bsp" -I"C:\Users\poorn\SimplicityStudio\v4_workspace\BLE_Client_Poorn_Mehta\platform\Device\SiliconLabs\EFR32BG13P\Source\GCC" -I"C:\Users\poorn\SimplicityStudio\v4_workspace\BLE_Client_Poorn_Mehta\hardware\kit\common\drivers" -I"C:\Users\poorn\SimplicityStudio\v4_workspace\BLE_Client_Poorn_Mehta\platform\bootloader\api" -I"C:\Users\poorn\SimplicityStudio\v4_workspace\BLE_Client_Poorn_Mehta\platform\emdrv\sleep\inc" -I"C:\Users\poorn\SimplicityStudio\v4_workspace\BLE_Client_Poorn_Mehta\protocol\bluetooth\ble_stack\inc\common" -I"C:\Users\poorn\SimplicityStudio\v4_workspace\BLE_Client_Poorn_Mehta\hardware\kit\common\halconfig" -I"C:\Users\poorn\SimplicityStudio\v4_workspace\BLE_Client_Poorn_Mehta\hardware\kit\EFR32BG13_BRD4104A\config" -I"C:\Users\poorn\SimplicityStudio\v4_workspace\BLE_Client_Poorn_Mehta\platform\emdrv\uartdrv\inc" -I"C:\Users\poorn\SimplicityStudio\v4_workspace\BLE_Client_Poorn_Mehta\platform\emdrv\common\inc" -I"C:\Users\poorn\SimplicityStudio\v4_workspace\BLE_Client_Poorn_Mehta\protocol\bluetooth\ble_stack\inc\soc" -I"C:\Users\poorn\SimplicityStudio\v4_workspace\BLE_Client_Poorn_Mehta\platform\halconfig\inc\hal-config" -I"C:\Users\poorn\SimplicityStudio\v4_workspace\BLE_Client_Poorn_Mehta\platform\emdrv\gpiointerrupt\inc" -I"C:\Users\poorn\SimplicityStudio\v4_workspace\BLE_Client_Poorn_Mehta\platform\emdrv\sleep\src" -I"C:\Users\poorn\SimplicityStudio\v4_workspace\BLE_Client_Poorn_Mehta\platform\radio\rail_lib\chip\efr32" -I"C:\Users\poorn\SimplicityStudio\v4_workspace\BLE_Client_Poorn_Mehta\app\bluetooth\common\stack_bridge" -I"C:\Users\poorn\SimplicityStudio\v4_workspace\BLE_Client_Poorn_Mehta\platform\bootloader" -IC:/Users/poorn/SimplicityStudio/v4_workspace/BLE_Server_LCD_MITM_Poorn_Mehta/lcdGraphics -I"C:/SiliconLabs/SimplicityStudio/v4/developer/sdks/gecko_sdk_suite/v2.3//platform/middleware/glib" -I"C:/SiliconLabs/SimplicityStudio/v4/developer/sdks/gecko_sdk_suite/v2.3//hardware/kit/common/drivers" -I"C:/SiliconLabs/SimplicityStudio/v4/developer/sdks/gecko_sdk_suite/v2.3//util/silicon_labs/silabs_core/graphics" -I"C:/SiliconLabs/SimplicityStudio/v4/developer/sdks/gecko_sdk_suite/v2.3//platform/middleware/glib/dmd" -I"C:/SiliconLabs/SimplicityStudio/v4/developer/sdks/gecko_sdk_suite/v2.3//platform/middleware/glib/glib" -O1 -Wall -c -fmessage-length=0 -ffunction-sections -fdata-sections -mfpu=fpv4-sp-d16 -mfloat-abi=softfp -MMD -MP -MF"platform/emdrv/sleep/src/sleep.d" -MT"platform/emdrv/sleep/src/sleep.o" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

