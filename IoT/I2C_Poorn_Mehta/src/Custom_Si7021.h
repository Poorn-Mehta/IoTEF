#ifndef		__Custom_Si7021_h__
#define 	__Custom_Si7021_h__

//***********************************************************************************
// Include files
//***********************************************************************************

#include "src/main.h"
#include "src/Custom_I2C0.h"

//***********************************************************************************
// defined files
//***********************************************************************************

#define Si7021_Temperature_Threshold	15	// Triggering temp
#define Si7021_Address		0x40
#define Si7021_En_Port		gpioPortD
#define Si7021_En_Pin		15

// Some macros
#define Si7021_Power_Setup() GPIO_PinModeSet(Si7021_En_Port, Si7021_En_Pin, gpioModePushPull, 0)
#define Si7021_Power_On()	GPIO_PinOutSet(Si7021_En_Port, Si7021_En_Pin)
#define Si7021_Power_Off()	GPIO_PinOutClear(Si7021_En_Port, Si7021_En_Pin)

// From device datasheet. Useful for verification i2c functioning
#define Si7021_Firmware_Value	0xFF
#define Si7021_Electronic_Serial_Number_Value	0x15

//***********************************************************************************
// global variables
//***********************************************************************************

uint16_t raw_temp;
float Si7021_Temperature_C;

//***********************************************************************************
// function prototypes
//***********************************************************************************
bool Si7021_Verify_Firmware(void);
bool Si7021_Electronic_Serial_Number(void);
void Si7021_Get_Temperature_C(void);

#endif
