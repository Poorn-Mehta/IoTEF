

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
#include "em_system.h"


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
volatile bool button0_read = 0;
volatile bool button1_read = 0;
uint8_t Connection_Handle = 0;

//***********************************************************************************
// function prototypes
//***********************************************************************************


//***********************************************************************************
// functions
//***********************************************************************************

void GPIO_EVEN_IRQHandler(void)
{
	CORE_AtomicDisableIrq();
	Interrupt_Read = GPIO_IntGet();
	GPIO_IntClear(Interrupt_Read);
	if(Interrupt_Read == Button0_Interrupt_Mask)
	{
		button0_read = true;
	}
	gecko_external_signal(Event_Button0_Pressed);
	NVIC_DisableIRQ(GPIO_EVEN_IRQn);
	CORE_AtomicEnableIrq();
}

void GPIO_ODD_IRQHandler(void)
{
	CORE_AtomicDisableIrq();
	Interrupt_Read = GPIO_IntGet();
	GPIO_IntClear(Interrupt_Read);
	if(Interrupt_Read == Button1_Interrupt_Mask)
	{
		button1_read = true;
	}
	gecko_external_signal(Event_Button1_Pressed);
	NVIC_DisableIRQ(GPIO_ODD_IRQn);
	CORE_AtomicEnableIrq();
}

void LETIMER0_IRQHandler(void)		// Letimer ISR, will execute after every 3 seconds
{
	CORE_AtomicDisableIrq();	// Nested interrupts not supported
	Interrupt_Read = LETIMER_IntGetEnabled(LETIMER0);	// Reading interrupt flag register
	LETIMER_IntClear(LETIMER0, Interrupt_Read);	// I like to clear all the interrupts so
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

/*
 *
 * temp
 * button intrp
 * troubleshoot after bonding reconnection*/

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

  uint32_t Event_Read;	// This variable is used to indicate which external
			// event has occurred
  int8_t RSSI_Value;
  uint8_t bonding_state = No_Bonding;
  bool passkey_event_entered = false;
  bool bonding_failure_flag = false;
  bool Notifications_Status = 0;	// Send notifications only when allowed
 // uint16_t Slave_Latency, Conn_Int, Timeout;	// These variables can be used to get
 // values of connection parameters

  Si7021_Power_On(); // Powering on the sensor
  LCD_init("BLE Server");
  LCD_write("BT ADDR", LCD_ROW_BTADDR1);
  LCD_write("Not Connected", LCD_ROW_CONNECTION);


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

	    		bonding_failure_flag = false;


	    		Notifications_Status = 0;

	    		Server_Address();

	    		// temporary for demo
	    		Custom_BLE_Server_Delete_Bondings();

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

	    		LCD_write("Advertising", LCD_ROW_ACTION);

	    		gecko_cmd_sm_set_passkey(-1);	// random passkeys
	    		gecko_cmd_sm_configure(SM_Config, sm_io_capability_displayyesno);	// config for bondable with MITM

	    		break;
	    	}

	    	case gecko_evt_sm_confirm_passkey_id:
	    	{
	    		passkey_event_entered = true;
	    		uint32_t key;
	    		key = evt->data.evt_sm_confirm_passkey.passkey; // reading passkey
	    		char Passkey_String[7];
	    		snprintf(Passkey_String, sizeof(Passkey_String), "%0.6ld",
	    		  			key);	// printing the passkey
	    		LCD_write(Passkey_String, LCD_ROW_PASSKEY);
	    		LCD_write("PB0-Accept PB1-Reject", LCD_ROW_ACTION);
	    	    NVIC_EnableIRQ(GPIO_EVEN_IRQn);
	    	    NVIC_EnableIRQ(GPIO_ODD_IRQn);

	    		break;
	    	}

	    	case gecko_evt_sm_bonded_id:
	    	{
	    		LCD_write("Bonded Successfully", LCD_ROW_ACTION);
	    		LCD_write("Connected", LCD_ROW_CONNECTION);
	    		bonding_failure_flag = false;
	    		break;
	    	}

	    	case gecko_evt_sm_bonding_failed_id:
	    	{
	    		LCD_write("Bonding Failed", LCD_ROW_ACTION);
	    		bonding_failure_flag = true;
	    		gecko_cmd_le_connection_close(Connection_Handle);
	    		break;
	    	}

	    	/*Connected*/
	    	case gecko_evt_le_connection_opened_id:
	    	{
	    		LCD_write("Connection Opened", LCD_ROW_CONNECTION);

	    		/*To get the connection handle value */
	    		Connection_Handle = evt->data.evt_le_connection_opened.connection;//address, 6 bytes

	    		bd_addr client_adr = evt->data.evt_le_connection_opened.address; // Reading client address
	    		char client_addr_string[6];
	    		snprintf(client_addr_string, sizeof(client_addr_string), "%0.2X:%0.2X", client_adr.addr[1], client_adr.addr[0]);
	    		LCD_write(client_addr_string, LCD_ROW_CLIENTADDR);

	    		/*Setting Connection Parameters*/
	    		gecko_cmd_le_connection_set_parameters
				(Connection_Handle, Conn_Int_Min_Val, Conn_Int_Max_Val, Slave_Lat_Val, Timeout_Val);

	    		bonding_state = evt->data.evt_le_connection_opened.bonding;

	    		if(bonding_state != No_Bonding)
	    		{
	    			LCD_write("Already Bonded", LCD_ROW_ACTION);
	    			LCD_write("Connected", LCD_ROW_CONNECTION);
	    			gecko_cmd_sm_increase_security(Connection_Handle);	//Encrypting
	    		}
	    		else
	    		{
	    			gecko_cmd_sm_increase_security(Connection_Handle);	//Encrypting
	    		}

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

	    		LCD_write("Disconnected", LCD_ROW_CONNECTION);
	    		LCD_write(" ", LCD_ROW_CLIENTADDR);
	    		LCD_write(" ", LCD_ROW_PASSKEY);
	    		LCD_write(" ", LCD_ROW_ACTION);
	    		LCD_write(" ", LCD_ROW_TEMPVALUE);


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
	    			LCD_write("Advertising", LCD_ROW_ACTION);
	    		}

	    		break;
	    	}

	    	/* This event indicates either that a local Client Characteristic Configuration descriptor
	    	 * has been changed by the remote GATT client, or that a confirmation from the remote
	    	 * GATT client was received upon a successful reception of the indication*/
	    	case gecko_evt_gatt_server_characteristic_status_id:
	    	{
	    		if(bonding_failure_flag == false)
	    		{
		    		/* Checking for both - the characteristic ID and the change in characteristic status*/
		    		if((evt->data.evt_gatt_server_characteristic_status.characteristic == gattdb_temperature_measurement)
		    				&& (evt->data.evt_gatt_server_characteristic_status.status_flags == Change_in_Characteristic_Status))
		    		{
		    			/* If notifications are enabled, then proceed with temp measurement*/
		    			if(evt->data.evt_gatt_server_characteristic_status.client_config_flags == Indications_Enabled)
		    			{
		    				Notifications_Status = 1;
		    				LETIMER_Enable(LETIMER0, true);
		    			}

		    			/* If notifications are disabled then stop the timer*/
		    			else if(evt->data.evt_gatt_server_characteristic_status.client_config_flags == Indications_Enabled)
		    			{
		    				Notifications_Status = 0;
		    				LETIMER_Enable(LETIMER0, false);
		    			}
		    		}
	    		}

	    		break;
	    	}

	    	/* External event has occurred - from interrupt*/
	    	case gecko_evt_system_external_signal_id:
	    	{
    			// Reading event ID in case of multiple events
	//    		if(passkey_event_entered == true)
	  //  		{
	    			Event_Read = evt->data.evt_system_external_signal.extsignals;
	    			if(Event_Read == Event_Button0_Pressed) // Searching for button0 ISR
	    			{
	    	    		if(button0_read == true)	//Button is pressed
	    	    		{
	    	    			if(passkey_event_entered == true)
	    	    			{
		    	    			gecko_cmd_sm_passkey_confirm(Connection_Handle, true);
		    	    			LCD_write("Passkey Confirmed", LCD_ROW_ACTION);
		    	    			button0_read = false;
	    	    			}
	    	    		}
	    	    		LCD_write("            ", LCD_ROW_PASSKEY);
	    			}
	    			else if(Event_Read == Event_Button1_Pressed)
	    			{
	    	    		if(button1_read == true)
	    	    		{
	    	    			if(passkey_event_entered == true)
	    	    			{
		    	    			gecko_cmd_sm_passkey_confirm(Connection_Handle, false);
		    	    			LCD_write("Passkey Rejected", LCD_ROW_PASSKEY);
		    	    			button1_read = false;
	    	    			}
	    	    		}
	    	    		LCD_write("            ", LCD_ROW_TEMPVALUE);
	    			}
//	    		}
	    		/* Proceed with sensor interface only if notifications are enabled*/
	    		if(Notifications_Status == true)
	    		{
	    			if(Event_Read == Event_Letimer_Period_Reached)
	    			{
	    				LETIMER_Enable(LETIMER0, false);	// Stopping LETIMER
	    				// 2621 / 32768 = 80ms
	    				// Using this timer so that mcu can sleep
	    				// No need of this now as sensor is always on but kept
	    				// for future reference
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
	    		uint32_t temperature1, temperature2;
	    		temperature1 = Si7021_Temperature_C / 1000;
	    		temperature2 = Si7021_Temperature_C - (temperature1 * 1000);
	    		char temperature_print[Temperature_Print_Length];
	    		snprintf(temperature_print, Temperature_Print_Length, "%0.2ld.%0.3ldC", temperature1, temperature2);
	    		LCD_write(temperature_print, LCD_ROW_TEMPVALUE);

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
	    		int16_t tmp_rssi;


	    		// Conditions
	    		if(RSSI_Value > -35)
	    		{
	    			tmp_rssi = BG13_Min_Power;
	    		}
	    		else if(RSSI_Value > -45)
	    		{
	    			tmp_rssi = -200;
	    		}
	    		else if(RSSI_Value > -55)
	    		{
	    			tmp_rssi = -150;
	    		}
	    		else if(RSSI_Value > -65)
	    		{
	    			tmp_rssi = -50;
	    		}
	    		else if(RSSI_Value > -75)
	    		{
	    			tmp_rssi = 0;
	    		}
	    		else if(RSSI_Value > -85)
	    		{
	    			tmp_rssi = 50;
	    		}
	    		else
	    		{
	    			tmp_rssi = BG13_Max_Power;
	    		}

	    		// Resume BT Module

    			gecko_cmd_system_halt(1);
    			gecko_cmd_system_set_tx_power(tmp_rssi);
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
