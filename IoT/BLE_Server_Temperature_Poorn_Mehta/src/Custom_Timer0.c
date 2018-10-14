//***********************************************************************************
// Include files
//***********************************************************************************
#include "src/Custom_TIMER0.h"
#include "src/gpio.h"

//***********************************************************************************
// Functions
//***********************************************************************************

void Custom_Timer0_Clock_Enable(void)
{
	CMU_ClockSelectSet(cmuClock_HFPER, cmuSelect_HFXO);	// Timer0 runs on HFPERCLK
	CMU_ClockEnable(cmuClock_HFPER, true);
	CMU_ClockEnable(cmuClock_TIMER0, true);	// Enabling proper branch
}

void Custom_Timer0_Init(void)
{
	Custom_Timer0_Clock_Enable();
	uint32_t New_Period;
	uint8_t i = 0;
	// to stay in range, also period is in ms
	New_Period = (Custom_HFXO_Freq / 1000) * Custom_Timer0_Period;
	while((i <= Custom_Timer0_Max_Prescaler) && (New_Period > Custom_Timer0_Topvalue_Max))
	{
		i += 1;
		New_Period >>= 1;	// Divide the value by 2, as the prescaler is only in power of 2
	}

	const TIMER_Init_TypeDef Custom_Timer0_Init_Struct =
	{
		false,// don't enable automatically
		true,	//debug run
		i,	// prescaler
		timerClkSelHFPerClk,	// clock source
		false,
		false,
		timerInputActionNone,
		timerInputActionNone,
		timerModeDown,	// set up as down counter
		false,
		false,
		false,
		false
	};

	// Calling emlib function to properly initiate timer0
	TIMER_Init(TIMER0, &Custom_Timer0_Init_Struct);

	// Setting top value to have specific period
	TIMER_TopSet(TIMER0, New_Period);

	// Clearing all interrupts
	TIMER_IntClear(TIMER0, 0x0FFF);

	// Enabling interrupts
	TIMER_IntEnable(TIMER0, TIMER_IEN_UF);
	NVIC_EnableIRQ(TIMER0_IRQn);

	// Locking config, just as a precaution
	TIMER_Lock(TIMER0);
}
