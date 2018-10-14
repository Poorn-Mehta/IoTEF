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

// LED0 pin is
#define	LED0_port  gpioPortF
#define LED0_pin   4
#define LED0_default	false 	// off
// LED1 pin is
#define LED1_port  gpioPortF
#define LED1_pin   5
#define LED1_default	false	// off

#define Button0_port	gpioPortF
#define Button0_pin		6
#define Button1_port	gpioPortF
#define Button1_pin		7

//***********************************************************************************
// global variables
//***********************************************************************************

extern bool button0_read;
extern bool button1_read;
extern bool valid_button_press;


//***********************************************************************************
// function prototypes
//***********************************************************************************
void Button0_Init(void);
void Button1_Init(void);
void gpio_init(void);
#endif
