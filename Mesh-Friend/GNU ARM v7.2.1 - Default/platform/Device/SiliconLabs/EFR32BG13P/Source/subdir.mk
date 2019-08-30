################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../platform/Device/SiliconLabs/EFR32BG13P/Source/system_efr32bg13p.c 

OBJS += \
./platform/Device/SiliconLabs/EFR32BG13P/Source/system_efr32bg13p.o 

C_DEPS += \
./platform/Device/SiliconLabs/EFR32BG13P/Source/system_efr32bg13p.d 


# Each subdirectory must supply rules for building sources it contributes
platform/Device/SiliconLabs/EFR32BG13P/Source/system_efr32bg13p.o: ../platform/Device/SiliconLabs/EFR32BG13P/Source/system_efr32bg13p.c
	@echo 'Building file: $<'
	@echo 'Invoking: GNU ARM C Compiler'
	arm-none-eabi-gcc -g -gdwarf-2 -mcpu=cortex-m4 -mthumb -std=c99 '-DEFR32BG13P632F512GM48=1' '-DMESH_LIB_NATIVE=1' '-DHAL_CONFIG=1' '-D__HEAP_SIZE=0x1200' '-D__STACK_SIZE=0x1000' -I"C:\Users\poorn\SimplicityStudio\v4_workspace\Mesh-Friend\protocol\bluetooth\bt_mesh\inc\soc" -I"C:\Users\poorn\SimplicityStudio\v4_workspace\Mesh-Friend\platform\halconfig\inc\hal-config" -I"C:\Users\poorn\SimplicityStudio\v4_workspace\Mesh-Friend\platform\emdrv\uartdrv\inc" -I"C:\Users\poorn\SimplicityStudio\v4_workspace\Mesh-Friend\platform\bootloader\api" -I"C:\Users\poorn\SimplicityStudio\v4_workspace\Mesh-Friend\hardware\kit\common\drivers" -I"C:\Users\poorn\SimplicityStudio\v4_workspace\Mesh-Friend\platform\radio\rail_lib\common" -I"C:/SiliconLabs/SimplicityStudio/v4/developer/sdks/blemesh/v1.3//hardware/kit/common/drivers" -I"C:\Users\poorn\SimplicityStudio\v4_workspace\Mesh-Friend\platform\emdrv\sleep\inc" -I"C:\Users\poorn\SimplicityStudio\v4_workspace\Mesh-Friend\lcdGraphics" -I"C:\Users\poorn\SimplicityStudio\v4_workspace\Mesh-Friend" -I"C:\Users\poorn\SimplicityStudio\v4_workspace\Mesh-Friend\platform\Device\SiliconLabs\EFR32BG13P\Source\GCC" -I"C:\Users\poorn\SimplicityStudio\v4_workspace\Mesh-Friend\protocol\bluetooth\bt_mesh\inc" -I"C:\Users\poorn\SimplicityStudio\v4_workspace\Mesh-Friend\platform\emdrv\common\inc" -I"C:\Users\poorn\SimplicityStudio\v4_workspace\Mesh-Friend\protocol\bluetooth\bt_mesh\inc\common" -I"C:\Users\poorn\SimplicityStudio\v4_workspace\Mesh-Friend\platform\emdrv\sleep\src" -I"C:\Users\poorn\SimplicityStudio\v4_workspace\Mesh-Friend\platform\emlib\src" -I"C:\Users\poorn\SimplicityStudio\v4_workspace\Mesh-Friend\protocol\bluetooth\bt_mesh\src" -I"C:\Users\poorn\SimplicityStudio\v4_workspace\Mesh-Friend\platform\Device\SiliconLabs\EFR32BG13P\Include" -I"C:\Users\poorn\SimplicityStudio\v4_workspace\Mesh-Friend\platform\radio\rail_lib\chip\efr32" -I"C:/SiliconLabs/SimplicityStudio/v4/developer/sdks/blemesh/v1.3//platform/middleware/glib/dmd" -I"C:\Users\poorn\SimplicityStudio\v4_workspace\Mesh-Friend\platform\emlib\inc" -I"C:\Users\poorn\SimplicityStudio\v4_workspace\Mesh-Friend\platform\CMSIS\Include" -I"C:\Users\poorn\SimplicityStudio\v4_workspace\Mesh-Friend\hardware\kit\EFR32BG13_BRD4104A\config" -I"C:/SiliconLabs/SimplicityStudio/v4/developer/sdks/blemesh/v1.3//platform/middleware/glib" -I"C:\Users\poorn\SimplicityStudio\v4_workspace\Mesh-Friend\platform\Device\SiliconLabs\EFR32BG13P\Source" -I"C:\Users\poorn\SimplicityStudio\v4_workspace\Mesh-Friend\platform\emdrv\gpiointerrupt\inc" -I"C:/SiliconLabs/SimplicityStudio/v4/developer/sdks/blemesh/v1.3//platform/middleware/glib/glib" -I"C:\Users\poorn\SimplicityStudio\v4_workspace\Mesh-Friend\hardware\kit\common\bsp" -I"C:\Users\poorn\SimplicityStudio\v4_workspace\Mesh-Friend\hardware\kit\common\halconfig" -Os -fno-builtin -flto -Wall -c -fmessage-length=0 -ffunction-sections -fdata-sections -mfpu=fpv4-sp-d16 -mfloat-abi=softfp -MMD -MP -MF"platform/Device/SiliconLabs/EFR32BG13P/Source/system_efr32bg13p.d" -MT"platform/Device/SiliconLabs/EFR32BG13P/Source/system_efr32bg13p.o" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


