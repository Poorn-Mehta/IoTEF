
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



#include "src/main.h"


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
/*
#ifndef MAX_CONNECTIONS
#define MAX_CONNECTIONS 4
#endif
uint8_t bluetooth_stack_heap[DEFAULT_BLUETOOTH_HEAP(MAX_CONNECTIONS)];

// Gecko configuration parameters (see gecko_configuration.h)
static const gecko_configuration_t config = {
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
};

// Flag for indicating DFU Reset must be performed
uint8_t boot_to_dfu = 0;*/

//***********************************************************************************
// Include files
//***********************************************************************************

#include "src/gpio.h"
#include "src/cmu.h"
#include "src/Custom_I2C0.h"
#include "src/Custom_LETIMER0.h"
#include "src/Custom_Sleep.h"
#include "src/Custom_Si7021.h"
#include "src/Custom_Timer0.h"


//***********************************************************************************
// defined files
//***********************************************************************************




//***********************************************************************************
// global variables
//***********************************************************************************

uint8_t Global_Events = 0;	// Global Variable for events
uint32_t Interrupt_Read;	// To know which interrupt has caused the device
							// to come out of sleep

//***********************************************************************************
// function prototypes
//***********************************************************************************


//***********************************************************************************
// functions
//***********************************************************************************

void LETIMER0_IRQHandler(void)		// Letimer ISR, will execute after every 3 seconds
{
	CORE_AtomicDisableIrq();	// Nested interrupts not supported
	Interrupt_Read = LETIMER_IntGetEnabled(LETIMER0);	// Reading interrupt flag register
	LETIMER_IntClear(LETIMER0, 0x1F);	// I like to clear all the interrupts so
	if(Interrupt_Read == LETIMER_IF_UF)	// Check for the specific interrupt
	{
		Global_Events = Event_Letimer_Period_Reached;	// Mark event as occurred in global variable
	}
	CORE_AtomicEnableIrq();		// Enable the interrupts again as ISR is ending
}

void TIMER0_IRQHandler(void)	// Timer ISR, will execute 80ms after TIMER0 is enabled
{
	CORE_AtomicDisableIrq();	// No nested interrupts, atomic operation
	Interrupt_Read = TIMER_IntGetEnabled(TIMER0);	// Reading interrupt flag register
	TIMER_IntClear(TIMER0, 0x0FFF);		// Clearing all interrupts in register
	if(Interrupt_Read == TIMER_IF_UF)	// Checking for the occurrence of specific interrupt
	{
		Global_Events = Event_Timer0_Period_Reached;	// Set the event variable properly
	}
	CORE_AtomicEnableIrq();		// End of ISR
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
  // Initialize application
//  initApp();
  // Initialize stack
 // gecko_init(&config);

  // Initializing TIMER0 - clocks, structure everything. Not enabling it.
  Custom_Timer0_Init();

  // Only enabling the clocks of I2C0. Structure and everything else pending.
  Custom_I2C0_Clock_Enable();

  // Setting up enable pin
  Si7021_Power_Setup();

  // LETIMER0 setup. Prescalers, structure, clocks - everything. Timer enabled
  // at the end of this function. Also sleep mode 3 is blocked inside it.
  Blue_Gecko_Letimer_Init();

  /* Initialize GPIO */
  gpio_init();

  // Initialize clocks
  cmu_init();

  // Setting up DCIS pin so that the LCD won't flicker during power switching
  // of Si7021
  GPIO_PinModeSet(gpioPortD, 14, gpioModePushPull, 0);
  GPIO_PinOutSet(gpioPortD, 14);

  // Scheduler
  while (1)
  {
	  if(Global_Events == 0)
	  {
		  Blue_Gecko_Sleep();	// Going to sleep (EM3)
	  }
	  if(Global_Events == Event_Letimer_Period_Reached)	// Checking whether letimer ISR has been run
	  {
		  Global_Events = 0;	// If it has, then first clear the global variable
		  Si7021_Power_On();	// Set the enable pin high, power on reset will follow
		  Custom_Timer0_Start();	// Need this to allow mpu to sleep while waiting for power
		  	  	  	  	  	  	  	// on reset to complete
		  Blue_Gecko_BlockSleepMode(1);	// As timer0 runs on HFPERCLK, lowest mode is em1
		  LETIMER_Enable(LETIMER0, false);	// Stop the Letimer, as it is supposed to give timing
		  	  	  	  	  	  	  	  	  	// only during the deep sleep - em3 - when sensor is off
	  	  Blue_Gecko_Sleep();	// Going to sleep while waiting for poweron reset of sensor to complete // should be commented out
		  EMU_EnterEM1();// should be commented out
	  }
	  if(Global_Events == Event_Timer0_Period_Reached)	// Checking whether timer0 ISR has been run
	  {
		  Global_Events = 0;	// If so then first reset the global event variable
		  Custom_Timer0_Stop();	// No need of this timer now. So stopping it until next cycle
		  Custom_I2C0_Init();	// Initializing I2C from scratch. Everything except clocks are
		  	  	  	  	  	    // initialized over here.
		  Si7021_Get_Temperature_C();	// Get the temperature from the Si7021 sensor
		  if(Si7021_Temperature_C < Si7021_Temperature_Threshold)	// Just checking for threshold
		  {
			  GPIO_PinOutSet(LED1_port, LED1_pin);
		  }
		  else
		  {
			  GPIO_PinOutClear(LED1_port, LED1_pin);
		  }

		  Si7021_Power_Off();	// Switch off the sensor.
		  Custom_I2C0_Abort();	// Send the abort command to completely halt the I2C bus
		  Custom_I2C0_Reset();	// Resetting the I2C registers and pin settings
		  LETIMER_Enable(LETIMER0, true);	// Start the letimer, as temp measurement is done
		  	  	  	  	  	  	  	  	  	// and device will go to deep sleep - em3
		  Blue_Gecko_UnblockSleepMode(1);	// No need of em1 mode now, as timer0 is not running
		  Blue_Gecko_Sleep();		// Go to sleep :D // should be commented out
	  }
  }



  // Initialize stack
//  gecko_init(&config);

}
