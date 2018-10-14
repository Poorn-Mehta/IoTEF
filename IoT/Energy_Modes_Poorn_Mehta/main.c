
/***********************************************************************************************//**
 * \file   main.c
 * \brief  Silicon Labs Empty Example Project
 *
 * This example demonstrates the bare minimum needed for a Blue Gecko C application
 * that allows Over-the-Air Device Firmware Upgrading (OTA DFU). The application
 * starts advertising after boot and restarts advertising after a connection is closed.
 ***************************************************************************************************
 * <b> (C) Copyright 2016 Silicon Labs, http://www.silabs.com</b>
 ***************************************************************************************************
 * This file is licensed under the Silabs License Agreement. See the file
 * "Silabs_License_Agreement.txt" for details. Before using this software for
 * any purpose, you must agree to the terms of that agreement.
 **************************************************************************************************/

/* Board headers */
#include "init_mcu.h"
#include "init_board.h"
#include "ble-configuration.h"
#include "board_features.h"

/* Bluetooth stack headers */
#include "bg_types.h"
#include "native_gecko.h"
#include "gatt_db.h"

/* Libraries containing default Gecko configuration values */
#include "em_emu.h"
#include "em_cmu.h"

/* Device initialization header */
#include "hal-config.h"

#if defined(HAL_CONFIG)
#include "bsphalconfig.h"
#else
#include "bspconfig.h"
#endif

/***********************************************************************************************//**
 * @addtogroup Application
 * @{
 **************************************************************************************************/

/***********************************************************************************************//**
 * @addtogroup app
 * @{
 **************************************************************************************************/

#ifndef MAX_CONNECTIONS
#define MAX_CONNECTIONS 4
#endif
uint8_t bluetooth_stack_heap[DEFAULT_BLUETOOTH_HEAP(MAX_CONNECTIONS)];

// Gecko configuration parameters (see gecko_configuration.h)
/*static const gecko_configuration_t config = {
  .config_flags = 0,
  .sleep.flags = SLEEP_FLAGS_DEEP_SLEEP_ENABLE,
  .bluetooth.max_connections = MAX_CONNECTIONS,
  .bluetooth.heap = bluetooth_stack_heap,
  .bluetooth.heap_size = sizeof(bluetooth_stack_heap),
  .bluetooth.sleep_clock_accuracy = 100, // ppm
  .gattdb = &bg_gattdb_data,
  .ota.flags = 0,
  .ota.device_name_len = 3,
  .ota.device_name_ptr = "OTA",
#if (HAL_PA_ENABLE) && defined(FEATURE_PA_HIGH_POWER)
  .pa.config_enable = 1, // Enable high power PA
  .pa.input = GECKO_RADIO_PA_INPUT_VBAT, // Configure PA input to VBAT
#endif // (HAL_PA_ENABLE) && defined(FEATURE_PA_HIGH_POWER)
};*/

// Flag for indicating DFU Reset must be performed
uint8_t boot_to_dfu = 0;
//***********************************************************************************
// Include files
//***********************************************************************************

#include "src/main.h"
#include "src/gpio.h"
#include "src/cmu.h"

#include "em_core.h"
#include "em_letimer.h"

//***********************************************************************************
// defined files
//***********************************************************************************

#define Total_Modes			5	// Number of EM modes in Blue Gecko
#define Lowest_Allowed_Sleep_Mode	3	// Modify to compare avg current consumption
#define LFXO_Frequency		32768	// Took the value from datasheet
#define ULFRCO_Frequency	1000	// Took the value from datasheet
#define LETIMER_Counter_Max		65535		// (2^16) - 1
#define Max_Prescaling_for_LETIMER	15	// Value from reference manual
#define LED_On_Time			175		// Time in Milliseconds, Vary the value for
									// different duty cycles
#define Period		2250				// Time in Milliseconds

//***********************************************************************************
// global variables
//***********************************************************************************

uint8_t EM_Array[Total_Modes];	// Protected by atomic operation
uint32_t Interrupt_Flags = 0;	// No need to protect as it is configured only once,
// and that too before the timer starts (so no possibility of interrupts)

//***********************************************************************************
// function prototypes
//***********************************************************************************

void LETIMER0_IRQHandler(void);
void Set_Letimer_Clock_Prescaler(bool LFXO_or_ULFRCO);
void Setting_Letimer_Clock(void);
void Blue_Gecko_BlockSleepMode(uint8 Required_Mode);
void Blue_Gecko_UnblockSleepMode(uint8 Required_Mode);
void Blue_Gecko_Sleep(void);
void Blue_Gecko_Letimer_Init(void);

//***********************************************************************************
// functions
//***********************************************************************************

void LETIMER0_IRQHandler(void)		// ISR
{
	CORE_AtomicDisableIrq();	// Nested Interrupts mode is not configured for now
	uint32_t Interrupt_Read;
	Interrupt_Read = LETIMER_IntGetEnabled(LETIMER0); // Reading the LETIMER0_IF
	LETIMER_IntClear(LETIMER0, Interrupt_Read);		// Clearing using LETIMER0_IFC

	// LETIMER0 is having 16bit downcounter. So I have set the top value using
	// COMP0 to set the period. And I have used COMP1 and UF interrupts to control
	// the LED state. While downcounting all the way from COMP0 top, when COMP1
	// is reached, the LED0 is turned on, and kept on till the counter underflows.
	if(Interrupt_Read == LETIMER_IF_COMP1)	// If COMP1 is matched, then turn on the LED0
	{
		GPIO_PinOutSet(LED0_port, LED0_pin);
	}
	else if(Interrupt_Read == LETIMER_IF_UF)	// If CNT is 0, Turn off the LED
	{
		GPIO_PinOutClear(LED0_port, LED0_pin);
	}
	CORE_AtomicEnableIrq();		// Enable Interrupts
}

void Blue_Gecko_Letimer_Init(void)
{
	// Set the clocks first
	Setting_Letimer_Clock();

	// Initializing Block Mode array
	uint8_t i;
	for (i = 0; i < Total_Modes; i ++)
	{
		EM_Array[i] = 0;
	}

	// Blocking Sleep Mode
	Blue_Gecko_BlockSleepMode(Lowest_Allowed_Sleep_Mode);

	// Setting Interrupts in LETIMER0
	Interrupt_Flags = (LETIMER_IEN_UF | LETIMER_IEN_COMP1);
	LETIMER_IntClear(LETIMER0, (uint32_t)0x1F);	// Clearing all interrupts before enabling
	LETIMER_IntEnable(LETIMER0, Interrupt_Flags);

	// Configuring NVIC
	NVIC_EnableIRQ(LETIMER0_IRQn);

	// Setting up the LETIMER Init Array as per the em_letimer.h file
	const LETIMER_Init_TypeDef Blue_Gecko_Letimer0 =
	{
			.enable = true,		// Enable the LETIMER when Init is completed
			.debugRun = true,	// Don't run while debugging (could be turned on if needed)
			.comp0Top = true,	// Set COMP0 as the top value, to set the period
			.bufTop = false,	// COMP1 is needed for duty cycle
			.out0Pol = 0,		// Doesn't matter which value is here
			.out1Pol = 0,
			.ufoa0 = letimerUFOANone, // No use of this as out0 can't be routed to PF4 or PF5
			.ufoa1 = letimerUFOANone,
			.repMode = letimerRepeatFree	// Free running mode
	};

	// Using emlib function to initialize LETIMER correctly
	LETIMER_Init(LETIMER0, &Blue_Gecko_Letimer0);
}

void Set_Letimer_Clock_Prescaler(bool LFXO_or_ULFRCO)	// 0 for LFXO, 1 for ULFRCO
{
	 uint32_t New_Period, New_Duty_Cycle;
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
	 New_Duty_Cycle = (New_Period * LED_On_Time) / (Period);

	 // Check and satisfy the need for prescaling
	 // Set the conditions so that highest resolution is achieved, and prescaler value
	 // Stays in range
	 while((i <= Max_Prescaling_for_LETIMER) && (New_Period > LETIMER_Counter_Max))
	 {
		 i += 1;
		 New_Period >>= 1;	// Divide the value by 2, as the prescaler is only in power of 2
	 }

	 // As the period (COMP0) is varied, the duty cycle COMP1 should be varied too accordingly
	 New_Duty_Cycle >>= i;

	 // Set the prescaler value in appropriate register
	 CMU->LFAPRESC0 = i;

	 // Load values into COMP0 and COMP1
	 LETIMER_CompareSet(LETIMER0, 0, New_Period);
	 LETIMER_CompareSet(LETIMER0, 1, New_Duty_Cycle);
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

/*******************************************************************************
 * # License
 * <b>Copyright 2016 Silicon Laboratories, Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 *
 * DISCLAIMER OF WARRANTY/LIMITATION OF REMEDIES: Silicon Labs has no
 * obligation to support this Software. Silicon Labs is providing the
 * Software "AS IS", with no express or implied warranties of any kind,
 * including, but not limited to, any implied warranties of merchantability
 * or fitness for any particular purpose or warranties against infringement
 * of any proprietary rights of a third party.
 *
 * Silicon Labs will not be liable for any consequential, incidental, or
 * special damages, or any other relief, or for any claim by any third party,
 * arising from your use of this Software.
 *
 ******************************************************************************/
/* The above Silicon Laboratories IP has been applied, and adhered to by the
 * developer for the following three functions:
 * Blue_Gecko_BlockSleepMode()
 * Blue_Gecko_UnblockSleepMode()
 * Blue_Gecko_Sleep()
 */

// Following 3 functions ensure that while a peripheral is running, and is having a particular lowest
// Energy mode in which it can operate, and not below it, then the module won't drop below that
// Particular lowest mode. If a peripheral is enabled and can be operated down to EM2 only and not the
// EM3, then block function will increase the value of 3rd element of array, preventing sleep function
// To drop down below EM2. The sleep function will go to the lowest allowed energy mode.
// In this application, LETIEMR is running forever and thus Unblock function hasn't been used.
// However, if needed, it can be called at the end of the peripheral disabling function.
// Also, arrays are used because multiple peripherals can increase the value, and the particular
// EM mode will stay as the lowest allowed EM mode, until all of the peripherals using that mode
// are disabled.
void Blue_Gecko_BlockSleepMode(uint8 Required_Mode)
{
	CORE_AtomicDisableIrq();	// Disabling interrupts so that variable doesn't get corrupted
	EM_Array[Required_Mode] += 1;	// Add one to the indicating that a peripheral is using that mode
	// Has just started running
	CORE_AtomicEnableIrq();
}

void Blue_Gecko_UnblockSleepMode(uint8 Required_Mode)
{
	CORE_AtomicDisableIrq();	// Disabling interrupts so that variable doesn't get corrupted
	if(EM_Array[Required_Mode] > 0)
	{
		EM_Array[Required_Mode] -= 1;	// Deduct 1 for indicating that a peripheral
		// Using that mode has just turned off
	}
	else
	{
		EM_Array[Required_Mode] = 0;	// Just as a precaution
	}
	CORE_AtomicEnableIrq();
}

void Blue_Gecko_Sleep(void)
{
	// Following conditions indicate that some peripheral(s) has/have set the respective modes
	// As their own lowest allowed energy modes. The sleep function will take care of all the
	// Peripherals by stepping down one EM mode at time, from EM0 to EM4(EM3 for now)
	// Only after ensuring that the mode which it has skipped (ex EM1 for the condition of EM_Array[2] > 0)
	// Was not being used by any active peripheral (In other words, all the active peripherals are
	// Able to work in an EM mode which is lower than the skipped one); it will step down further.
	if(EM_Array[0] > 0)
	{
		return;
	}
	else if(EM_Array[1] > 0)
	{
		EMU_EnterEM1();
	}
	else if(EM_Array[2] > 0)
	{
		EMU_EnterEM2(true);
	}
	else	// Not configured for EM4 now
	{
		EMU_EnterEM3(true);
	}
}

//***********************************************************************************
// main
//***********************************************************************************

/**
 * @brief  Main function
 */
int main(void)
{

  // Initialize device
  initMcu();
  // Initialize board
  initBoard();

  // LETIMER Initialization
  Blue_Gecko_Letimer_Init();
  /* Initialize GPIO */
  gpio_init();

  // Initialize clocks
  cmu_init();

  // Initialize stack
//  gecko_init(&config);

  while (1)
  {
		Blue_Gecko_Sleep();
  }
}
