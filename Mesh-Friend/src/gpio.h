#ifndef		__gpio_h__
#define 	__gpio_h__

//***********************************************************************************
// Include files
//***********************************************************************************
#include "main.h"
#include "em_gpio.h"

//***********************************************************************************
// defined files
//***********************************************************************************

//// LED0 pin is
//#define	LED0_port  gpioPortF
//#define LED0_pin   4
//#define LED0_default	false 	// off
//// LED1 pin is
//#define LED1_port  gpioPortF
//#define LED1_pin   5
//#define LED1_default	false	// off

#define Button0_port	gpioPortF
#define Button0_pin		6
#define Button0_Interrupt_Mask	0x40

#define Button1_port	gpioPortF
#define Button1_pin		7
#define Button1_Interrupt_Mask	0x80

#define Alarm_port			gpioPortD
#define Alarm_pin				10


//***********************************************************************************
// global variables
//***********************************************************************************

extern volatile bool button0_read;
extern volatile bool button1_read;
extern volatile bool valid_button_press;

//***********************************************************************************
// function prototypes
//***********************************************************************************
void Button0_Init(void);
void Button1_Init(void);
void gpio_init(void);
#endif
