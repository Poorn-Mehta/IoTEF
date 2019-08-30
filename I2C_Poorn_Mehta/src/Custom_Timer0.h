//***********************************************************************************
// Include files
//***********************************************************************************
#ifndef		__Custom_TIMER0_h__
#define 	__Custom_TIMER0_h__

#include "src/main.h"
#include "em_cmu.h"
#include "em_timer.h"
#include "init_mcu.h"
#include "src/Custom_Sleep.h"
#include "src/Custom_I2C0.h"
#include "src/Custom_Si7021.h"
#include "em_letimer.h"

//***********************************************************************************
// defined files
//***********************************************************************************

#define Custom_HFXO_Freq	BSP_CLK_HFXO_FREQ / 2	// For delay calculations, obtained this value
													// using application note (which says 14MHz,
													// which is actually lower)
#define Custom_Timer0_Period	80 //in ms, power on reset time for Si7021
#define Custom_Timer0_Max_Prescaler		10
#define Custom_Timer0_Topvalue_Max		65535
#define Custom_Timer0_Start()	TIMER_Enable(TIMER0, true)
#define Custom_Timer0_Stop()	TIMER_Enable(TIMER0, false)


//***********************************************************************************
// global variables
//***********************************************************************************

//***********************************************************************************
// function prototypes
//***********************************************************************************

void Custom_Timer0_Init(void);
void Custom_Timer0_Clock_Enable(void);
void TIMER0_IRQHandler(void);


#endif
