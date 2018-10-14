//***********************************************************************************
// Include files
//***********************************************************************************
#include "src/Custom_LETIMER0.h"
#include "em_cmu.h"

//***********************************************************************************
// Functions
//***********************************************************************************
uint32_t Interrupt_Flags = 0;

void Blue_Gecko_Letimer_Init(void)
{
	// Set the clocks first
	Setting_Letimer_Clock();

	// Initializing Block Mode array
	void Blue_Gecko_Clear_EM_Array(void);

	// Blocking Sleep Mode
//	Blue_Gecko_BlockSleepMode(Lowest_Allowed_Sleep_Mode);


	// Setting up the LETIMER Init Array as per the em_letimer.h file
	const LETIMER_Init_TypeDef Blue_Gecko_Letimer0 =
	{
			.enable = false,		// Don't enable the LETIMER automatically
			.debugRun = true,	// Run while debugging
			.comp0Top = true,	// Set COMP0 as the top value, to set the period
			.bufTop = false,	// No need
			.out0Pol = 0,		// Doesn't matter which value is here
			.out1Pol = 0,
			.ufoa0 = letimerUFOANone, // No use of this
			.ufoa1 = letimerUFOANone,
			.repMode = letimerRepeatFree	// Free running mode
	};

	// Using emlib function to initialize LETIMER correctly
	LETIMER_Init(LETIMER0, &Blue_Gecko_Letimer0);

	// Setting Interrupts in LETIMER0
	LETIMER_IntClear(LETIMER0, (uint32_t)0x1F);	// Clearing all interrupts before enabling
	LETIMER_IntEnable(LETIMER0, LETIMER_IEN_UF);

	// Configuring NVIC
	NVIC_EnableIRQ(LETIMER0_IRQn);
}

void Set_Letimer_Clock_Prescaler(bool LFXO_or_ULFRCO)	// 0 for LFXO, 1 for ULFRCO
{
	 uint32_t New_Period;
	 uint8_t i = 0;
	 if(LFXO_or_ULFRCO)	// Test for clock source
	 {
		 New_Period = ULFRCO_Frequency * Period;
	 }
	 else
	 {
		 New_Period = LFXO_Frequency * Period;
	 }
	 New_Period /= 1000;	// As period is in milliseconds

	 // Check and satisfy the need for prescaling
	 // Set the conditions so that highest resolution is achieved, and prescaler value
	 // Stays in range
	 while((i <= Max_Prescaling_for_LETIMER) && (New_Period > LETIMER_Counter_Max))
	 {
		 i += 1;
		 New_Period >>= 1;	// Divide the value by 2, as the prescaler is only in power of 2
	 }

	 // Set the prescaler value in appropriate register
	 CMU->LFAPRESC0 = i;

	 // Load values into COMP0
	 LETIMER_CompareSet(LETIMER0, 0, New_Period);
}

void Setting_Letimer_Clock(void)	// Function to set source of LFA
{
	// Enabling CORELE
	CMU_ClockSelectSet(cmuClock_CORELE, true);
	CMU_ClockEnable(cmuClock_CORELE, true);

	// Not configured for EM4
	if((Lowest_Allowed_Sleep_Mode >= 0) && (Lowest_Allowed_Sleep_Mode <= 3))
	{
		if(Lowest_Allowed_Sleep_Mode == 3)		// Go for ULFRCO if in EM3
		{
			CMU_ClockSelectSet(cmuClock_LFA, cmuSelect_ULFRCO);
			Set_Letimer_Clock_Prescaler(1);
		}
		else							// Go for LFXO if in any mode above EM3
		{
			CMU_ClockSelectSet(cmuClock_LFA, cmuSelect_LFXO);
			Set_Letimer_Clock_Prescaler(0);
		}
		CMU_ClockEnable(cmuClock_LETIMER0, true);
	}
}

