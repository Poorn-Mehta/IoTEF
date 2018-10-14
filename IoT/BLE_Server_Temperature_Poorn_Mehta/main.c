
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
uint8_t boot_to_dfu = 0;

//***********************************************************************************
// Include files
//***********************************************************************************

#include "src/gpio.h"
#include "src/cmu.h"
#include "src/Custom_I2C0.h"
#include "src/Custom_LETIMER0.h"
#include "src/Custom_Sleep.h"
#include "src/Custom_Si7021.h"
#include "src/Custom_BLE_Server.h"


//***********************************************************************************
// defined files
//***********************************************************************************


//***********************************************************************************
// global variables
//***********************************************************************************

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
		gecko_external_signal(Event_Letimer_Period_Reached);
	}
	CORE_AtomicEnableIrq();		// Enable the interrupts again as ISR is ending
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
  initApp();

  // Initialize stack
  gecko_init(&config);

  // Only enabling the clocks of I2C0. Structure and everything else pending.
  Custom_I2C0_Clock_Enable();

  // Setting up enable pin
  Si7021_Power_Setup();

  // LETIMER0 setup. Prescalers, structure, clocks - everything.
  Blue_Gecko_Letimer_Init();

  /* Initialize GPIO */
  gpio_init();

  // Initialize clocks
  cmu_init();

  //gecko_external_signal


  // Setting up DCIS pin so that the LCD won't flicker during power switching
  // of Si7021
  GPIO_PinModeSet(gpioPortD, 14, gpioModePushPull, 0);
  GPIO_PinOutSet(gpioPortD, 14);

  uint32_t Event_Read;	// This variable is used to indicate which external
			// event has occurred
  uint8_t Connection_Handle = 0;
  int8_t RSSI_Value;
  bool Notifications_Status = 0;	// Send notifications only when allowed
 // uint16_t Slave_Latency, Conn_Int, Timeout;	// These variables can be used to get
 // values of connection parameters

  // Scheduler
  while (1)
  {
	    /* Event pointer for handling events */
	    struct gecko_cmd_packet* evt;

	    /* Check for stack event. */
	    evt = gecko_wait_event();

	    /* Handle events */
	    switch (BGLIB_MSG_ID(evt->header))
	    {
	    	/* This boot event is generated when the system boots up after reset.
	    	 * Do not call any stack commands before receiving the boot event.
	    	 * Here the system is set to start advertising immediately after boot procedure. */
	    	case gecko_evt_system_boot_id:
	    	{
	    		Notifications_Status = 0;

	    		// Setting power to be 0 dB on reset
	    		gecko_cmd_system_set_tx_power(0);

	    		/* Set advertising parameters. 250ms advertisement interval.
	    		 * The first parameter is advertising set handle
	    		 * The next two parameters are minimum and maximum advertising interval, both in
	    		 * units of (milliseconds * 1.6). Thus for 250ms, 400 as value.
	    		 * The last two parameters are duration and maxevents left as default. */
	    		gecko_cmd_le_gap_set_advertise_timing(0, 400, 400, 0, 0);

	    		/* Start general advertising and enable connections. */
	    		gecko_cmd_le_gap_start_advertising
				(0, le_gap_general_discoverable, le_gap_connectable_scannable);

	    		break;
	    	}

	    	/*Connected*/
	    	case gecko_evt_le_connection_opened_id:
	    	{
	    		/*To get the connection handle value */
	    		Connection_Handle = evt->data.evt_le_connection_opened.connection;

	    		/*Setting Connection Parameters*/
	    		gecko_cmd_le_connection_set_parameters
				(Connection_Handle, Conn_Int_Min_Val, Conn_Int_Max_Val, Slave_Lat_Val, Timeout_Val);

	    		break;
	    	}

	    	/*This can be used to read parameter values after setting*/
	    	/*	      case gecko_evt_le_connection_parameters_id:

	    	  	  Slave_Latency = evt->data.evt_le_connection_parameters.latency;
	    	  	  Conn_Int = evt->data.evt_le_connection_parameters.interval;
	    	  	  Timeout = evt->data.evt_le_connection_parameters.timeout;
	    	  	  break;*/

	    	/*Disconnected*/
	    	case gecko_evt_le_connection_closed_id:
	    	{
	    		// Reset notification status
	    		Notifications_Status = 0;

	    		// Reset tx power to 0dBm
	    		gecko_cmd_system_set_tx_power(0);

	    		/* Check if need to boot to dfu mode */
	    		if(boot_to_dfu)
	    		{
	    			/* Enter to DFU OTA mode */
	    			gecko_cmd_system_reset(2);
	    		}
	    		else
	    		{
	    			/* Restart advertising after client has disconnected */
	    			gecko_cmd_le_gap_start_advertising(0, le_gap_general_discoverable, le_gap_connectable_scannable);
	    		}

	    		break;
	    	}

	    	/* This event indicates either that a local Client Characteristic Configuration descriptor
	    	 * has been changed by the remote GATT client, or that a confirmation from the remote
	    	 * GATT client was received upon a successful reception of the indication*/
	    	case gecko_evt_gatt_server_characteristic_status_id:
	    	{
	    		/* Checking for both - the characteristic ID and the change in characteristic status*/
	    		if((evt->data.evt_gatt_server_characteristic_status.characteristic == gattdb_temperature_measurement)
	    				&& (evt->data.evt_gatt_server_characteristic_status.status_flags == 0x01))
	    		{
	    			/* If notifications are enabled, then proceed with temp measurement*/
	    			if(evt->data.evt_gatt_server_characteristic_status.client_config_flags == 0x02)
	    			{
	    				Notifications_Status = 1;
	    				LETIMER_Enable(LETIMER0, true);
	    			}

	    			/* If notifications are disabled then stop the timer*/
	    			else if(evt->data.evt_gatt_server_characteristic_status.client_config_flags == 0x00)
	    			{
	    				Notifications_Status = 0;
	    				LETIMER_Enable(LETIMER0, false);
	    			}
	    		}

	    		break;
	    	}

	    	/* External event has occurred - from interrupt*/
	    	case gecko_evt_system_external_signal_id:
	    	{
	    		/* Proceed with sensor powerup and timer only if notifications are enabled*/
	    		if(Notifications_Status == 1)
	    		{
	    			// Reading event ID in case of multiple events
	    			Event_Read = evt->data.evt_system_external_signal.extsignals;
	    			if(Event_Read == Event_Letimer_Period_Reached)
	    			{
	    				Si7021_Power_On(); // Powering on the sensor
	    				LETIMER_Enable(LETIMER0, false);	// Stopping LETIMER
	    				// 2621 / 32768 = 80ms
	    				// Using this timer so that mcu can sleep
	    				gecko_cmd_hardware_set_soft_timer(2621, 0, 0);
	    			}
	    		}
	    		// If notifications are disabled then turn off the timer
	    		else
	    		{
	    			gecko_cmd_hardware_set_soft_timer(0, 0, 0);
	    		}

	    		break;
	    	}

	    	/* Timer triggered - 80ms passed */
	    	case gecko_evt_hardware_soft_timer_id:
	    	{
	    		// Read temperature from Si7021 and send it to client
	    		Custom_BLE_Server_Get_Temperature_Send_Notification();

	    		// Switching off timer so that it doesn't generate event
	    		gecko_cmd_hardware_set_soft_timer(0, 0, 0);

	    		// Reading RSSI Value
	    		gecko_cmd_le_connection_get_rssi(Connection_Handle);

	    		// Starting LETIMER once again, as a new cycle has begun
	    		LETIMER_Enable(LETIMER0, true);

	    		break;
	    	}

	    	case gecko_evt_le_connection_rssi_id:
	    	{
	    		// Get RSSI Value
	    		RSSI_Value = evt->data.evt_le_connection_rssi.rssi;

	    		// Halt the BT module to change the TX power
	    		gecko_cmd_system_halt(1);

	    		// Conditions
	    		if(RSSI_Value > -35)
	    		{
	    			gecko_cmd_system_set_tx_power(BG13_Min_Power);
	    		}
	    		else if(RSSI_Value > -45)
	    		{
	    			gecko_cmd_system_set_tx_power(-200);
	    		}
	    		else if(RSSI_Value > -55)
	    		{
	    			gecko_cmd_system_set_tx_power(-150);
	    		}
	    		else if(RSSI_Value > -65)
	    		{
	    			gecko_cmd_system_set_tx_power(-50);
	    		}
	    		else if(RSSI_Value > -75)
	    		{
	    			gecko_cmd_system_set_tx_power(0);
	    		}
	    		else if(RSSI_Value > -85)
	    		{
	    			gecko_cmd_system_set_tx_power(50);
	    		}
	    		else
	    		{
	    			gecko_cmd_system_set_tx_power(BG13_Max_Power);
	    		}

	    		// Resume BT Module
	    		gecko_cmd_system_halt(0);

	    		break;
	    	}

	    	/* Events related to OTA upgrading
	         ----------------------------------------------------------------------------- */

	    	/* Check if the user-type OTA Control Characteristic was written.
	    	 * If ota_control was written, boot the device into Device Firmware Upgrade (DFU) mode. */
	    	case gecko_evt_gatt_server_user_write_request_id:
	    	{
		        if (evt->data.evt_gatt_server_user_write_request.characteristic == gattdb_ota_control)
		        {
		        	/* Set flag to enter to OTA mode */
		        	boot_to_dfu = 1;
		        	/* Send response to Write Request */
		        	gecko_cmd_gatt_server_send_user_write_response(
		            evt->data.evt_gatt_server_user_write_request.connection,
		            gattdb_ota_control,
		            bg_err_success);

		        	/* Close connection to enter to DFU OTA mode */
		        	gecko_cmd_le_connection_close(evt->data.evt_gatt_server_user_write_request.connection);
		        }
		        break;
	    	}

	    	default:
	    	{
	    		break;
	    	}
	    }
  	}
}
