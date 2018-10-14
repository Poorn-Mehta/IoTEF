#ifndef		__Custom_LETIMER0_h__
#define 	__Custom_LETIMER0_h__

//***********************************************************************************
// Include files
//***********************************************************************************
#include "src/main.h"
#include "src/gpio.h"
#include "em_letimer.h"
#include "src/Custom_Sleep.h"
#include "src/Custom_Si7021.h"
#include "src/Custom_Timer0.h"

//***********************************************************************************
// defined files
//***********************************************************************************

#define LFXO_Frequency		32768	// Took the value from datasheet
#define ULFRCO_Frequency	1000	// Took the value from datasheet
#define LETIMER_Counter_Max		65535		// (2^16) - 1
#define Max_Prescaling_for_LETIMER	15	// Value from reference manual
#define Period		3000				// Time in Milliseconds

//***********************************************************************************
// global variables
//***********************************************************************************



//***********************************************************************************
// function prototypes
//***********************************************************************************
void LETIMER0_IRQHandler(void);
void Set_Letimer_Clock_Prescaler(bool LFXO_or_ULFRCO);
void Setting_Letimer_Clock(void);
void Blue_Gecko_Letimer_Init(void);
#endif
