
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
//// Gecko configuration parameters (see gecko_configuration.h)
///*static const gecko_configuration_t config = {
//  .config_flags = 0,
//  .sleep.flags = SLEEP_FLAGS_DEEP_SLEEP_ENABLE,
//  .bluetooth.max_connections = MAX_CONNECTIONS,
//  .bluetooth.heap = bluetooth_stack_heap,
//  .bluetooth.heap_size = sizeof(bluetooth_stack_heap),
//  .bluetooth.sleep_clock_accuracy = 100, // ppm
//  .gattdb = &bg_gattdb_data,
//  .ota.flags = 0,
//  .ota.device_name_len = 3,
//  .ota.device_name_ptr = "OTA",
//#if (HAL_PA_ENABLE) && defined(FEATURE_PA_HIGH_POWER)
//  .pa.config_enable = 1, // Enable high power PA
//  .pa.input = GECKO_RADIO_PA_INPUT_VBAT, // Configure PA input to VBAT
//#endif // (HAL_PA_ENABLE) && defined(FEATURE_PA_HIGH_POWER)
//};*/

// Flag for indicating DFU Reset must be performed

//***********************************************************************************
// Include files
//***********************************************************************************

#include "src/main.h"
#include "src/cmu.h"
#include "src/letimer.h"

uint8_t boot_to_dfu = 0;

//***********************************************************************************
// global variables
//***********************************************************************************

uint8_t EM_Array[Total_Modes];	// Protected by atomic operation
uint32_t Interrupt_Flags = 0;	// No need to protect as it is configured only once,
// and that too before the timer starts (so no possibility of interrupts)




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
