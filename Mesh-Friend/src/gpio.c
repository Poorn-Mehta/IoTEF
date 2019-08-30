//***********************************************************************************
// Include files
//***********************************************************************************
#include "src/gpio.h"

//***********************************************************************************
// defined files
//***********************************************************************************


//***********************************************************************************
// global variables
//***********************************************************************************


//***********************************************************************************
// function prototypes
//***********************************************************************************


//***********************************************************************************
// functions
//***********************************************************************************
void Button0_Init(void)
{
	uint32_t int_clear;
	int_clear = GPIO_IntGet();
	GPIO_IntClear(int_clear);
	GPIO_IntConfig(Button0_port, Button0_pin, false, true, true);
//	NVIC_EnableIRQ(GPIO_EVEN_IRQn);
}

void Button1_Init(void)
{
	uint32_t int_clear;
	int_clear = GPIO_IntGet();
	GPIO_IntClear(int_clear);
	GPIO_IntConfig(Button1_port, Button1_pin, false, true, true);
//	NVIC_EnableIRQ(GPIO_ODD_IRQn);
}

void gpio_init(void)
{

	// Set LED ports to be standard output drive with default off (cleared)
//	GPIO_DriveStrengthSet(LED0_port, gpioDriveStrengthStrongAlternateStrong);
//	GPIO_DriveStrengthSet(LED0_port, gpioDriveStrengthWeakAlternateWeak);
//	GPIO_PinModeSet(LED0_port, LED0_pin, gpioModePushPull, LED0_default);

//	GPIO_DriveStrengthSet(LED1_port, gpioDriveStrengthStrongAlternateStrong);
//	GPIO_DriveStrengthSet(LED1_port, gpioDriveStrengthWeakAlternateWeak);
//	GPIO_PinModeSet(LED1_port, LED1_pin, gpioModePushPull, LED1_default);

	GPIO_DriveStrengthSet(Alarm_port, gpioDriveStrengthStrongAlternateStrong);
	GPIO_PinModeSet(Alarm_port, Alarm_pin, gpioModePushPull, false);
//	GPIO_PinOutSet(LED0_port, LED0_pin);
//	GPIO_PinOutSet(LED1_port, LED1_pin);

  	GPIO_PinModeSet(Button0_port, Button0_pin, gpioModeInput, false);
  	GPIO_PinModeSet(Button1_port, Button1_pin, gpioModeInput, false);
  	Button0_Init();
  	Button1_Init();
}
