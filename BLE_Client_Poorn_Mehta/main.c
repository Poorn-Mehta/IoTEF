

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
#include "src/Custom_BLE_Client.h"


//***********************************************************************************
// defined files
//***********************************************************************************


//***********************************************************************************
// global variables
//***********************************************************************************

//This has made global so that it can be used in separate .c file
struct gecko_cmd_packet* evt;

//Hardcoded server address
bd_addr Server_Addr = { .addr = {0xF9, 0x30, 0xEF, 0x57, 0x0B, 0x00} };

uint32_t Interrupt_Read;	// To know which interrupt has caused the device
							// to come out of sleep

//The variable to check for bonding failed condition
bool bonding_failure_flag = false;

//Variables shared between ISR and normal code
volatile bool button0_read = false;
volatile bool button1_read = false;

uint8_t Connection_Handle = 0;
//uint8_t Services_Count = 0;
//uint8_t Characteristics_Count = 0;
uint32_t Service_Handle = 0;
uint16_t Characteristic_Handle = 0;
//uint16_t Characteristic_Handle2 = 0;

//Fixed values for use in service and characteristic discovery by uuid
const uint8_t Health_Thermometer_Service[2] = {0x09, 0x18};
const uint8_t Temperature_Measurement_Characteristic[2] = {0x1C, 0x2A};
//const uint8_t Tx_Power_Service[2] = {0x04, 0x18};
//const uint8_t Tx_Power_Characteristic[2] = {0x07, 0x2A};


//Enum state variable
Global_Device_States Dev_State;

//***********************************************************************************
// function prototypes
//***********************************************************************************


//***********************************************************************************
// functions
//***********************************************************************************

//PB0 ISR
void GPIO_EVEN_IRQHandler(void)
{
	//basic interrupt handling flow
	CORE_AtomicDisableIrq();
	Interrupt_Read = GPIO_IntGet();
	GPIO_IntClear(Interrupt_Read);
	if(Interrupt_Read == Button0_Interrupt_Mask)
	{
		button0_read = true;
	}
	//interrupts are handled using external signal event
	gecko_external_signal(Event_Button0_Pressed);

	//used instead of timer for debouncing
	NVIC_DisableIRQ(GPIO_EVEN_IRQn);

	CORE_AtomicEnableIrq();
}

//same as above, except it is for PB1
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

  /* Initialize GPIO */
  gpio_init();

  // Initialize clocks
  cmu_init();

  //gecko_external_signal
  uint32_t Event_Read;	// This variable is used to indicate which external
			// event has occurred

  //initializing bonding with no bonding handle value
  uint8_t bonding_state = No_Bonding;

 // uint16_t Slave_Latency, Conn_Int, Timeout;	// These variables can be used to get
 // values of connection parameters

  //displaying basic information on lcd
  LCD_init("BLE Client");
  LCD_write("BT ADDR", LCD_ROW_BTADDR1);
  LCD_write("Not Connected", LCD_ROW_CONNECTION);


  // Scheduler
  while (1)
  {
	    /* Event pointer for handling events */
//	    struct gecko_cmd_packet* evt;

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
	    		//resetting everytime the device goes through power cycle
	    		bonding_failure_flag = false;

	            // Set passive scanning on 1Mb PHY
	            gecko_cmd_le_gap_set_discovery_type(le_gap_phy_1m, Scan_Passive);

	            // Set scan interval and scan window
	            gecko_cmd_le_gap_set_discovery_timing(le_gap_phy_1m, Scan_Interval, Scan_Window);

	            //display own address
	            Client_Address();

	            //scan for button input, to either delete bondings or keep it
	            Custom_BLE_Client_Delete_Bondings();

	    		// config for bondable with MITM
	    		gecko_cmd_sm_configure(SM_Config, sm_io_capability_displayyesno);

	            // Start scanning - looking for thermometer devices
	            gecko_cmd_le_gap_start_discovery(le_gap_phy_1m, le_gap_discover_generic);
	            Dev_State = scanning;
	            LCD_write(" ", LCD_ROW_PASSKEY);
	            LCD_write("Scanning", LCD_ROW_ACTION);

	    		break;
	    	}

	    	//some advertiser has been found
	    	case gecko_evt_le_gap_scan_response_id:
	    	{
	    		//check whether the advertiser found is the targeted device
	    		Custom_BLE_Client_Check_for_Target_Server();

	    		//check whether target server was found or not
    			if(Dev_State == scanning)
    			{
    				//if not then start scanning again
    				gecko_cmd_le_gap_start_discovery(le_gap_phy_1m, le_gap_discover_generic);
    				LCD_write("Scanning", LCD_ROW_ACTION);
    			}

	    		break;
	    	}

	    	//pairing has started, passkey must be confirmed
	    	case gecko_evt_sm_confirm_passkey_id:
	    	{
	    		//function to handle passkey confirmation
	    		Custom_BLE_Client_Confirm_Passkey();
	    		break;
	    	}

//	    	case gecko_evt_sm_confirm_bonding_id:
//	    	{
//	    		LCD_write("PB0-Bond PB1-Reject", LCD_ROW_ACTION);
//	    	    NVIC_EnableIRQ(GPIO_EVEN_IRQn);
//	    	    NVIC_EnableIRQ(GPIO_ODD_IRQn);
//	    		break;
//	    	}

	    	//bonding successful
	    	case gecko_evt_sm_bonded_id:
	    	{
	    		//update necessary variables and information
	    		LCD_write("Bonded Successfully", LCD_ROW_CONNECTION);
//	    		LCD_write("Connected", LCD_ROW_CONNECTION);
	    		bonding_failure_flag = false;
	    		Dev_State = bonded;

	    		//start searching for the health thermometer service
	    		Custom_BLE_Client_Discover_Primary_Services_by_UUID(Health_Thermometer_Service_Length,
	    				Health_Thermometer_Service);

	    		break;
	    	}

	    	//bonding failure
	    	case gecko_evt_sm_bonding_failed_id:
	    	{
	    		//update the information and necessary variables, and close the connection
	    		LCD_write("Bonding Failed", LCD_ROW_ACTION);
	    		bonding_failure_flag = true;
	    		gecko_cmd_le_connection_close(Connection_Handle);

	    		break;
	    	}

	    	//connection is opened
	    	case gecko_evt_le_connection_opened_id:
	    	{

	    		LCD_write("Connection Opened", LCD_ROW_CONNECTION);

	    		//To get the connection handle value
	    		Connection_Handle = evt->data.evt_le_connection_opened.connection;//address, 6 bytes

	    		//print address of the server
	    		Print_Remote_Address();

	    		//Setting Connection Parameters
//	    		gecko_cmd_le_connection_set_parameters
//				(Connection_Handle, Conn_Int_Min_Val, Conn_Int_Max_Val, Slave_Lat_Val, Timeout_Val);

	    		//load bonding state from the structure
	    		bonding_state = evt->data.evt_le_connection_opened.bonding;

	    		//check whether the devices are already bonded or not
	    		if(bonding_state != No_Bonding)
				{
	    			//if already bonded then update state variable and information
					LCD_write("Already Bonded", LCD_ROW_ACTION);
					LCD_write("Connected", LCD_ROW_CONNECTION);
					Dev_State = bonded;

					//to re-encrypt the link
					gecko_cmd_sm_increase_security(Connection_Handle);

					//start the service search because already bonded
		    		Custom_BLE_Client_Discover_Primary_Services_by_UUID(Health_Thermometer_Service_Length,
		    				Health_Thermometer_Service);
				}
				else
				{
					//to generate passkeys and manually confirm them, and bond afterwards
					gecko_cmd_sm_increase_security(Connection_Handle);
				}

	    		break;
	    	}

	    	//Disconnected
	    	case gecko_evt_le_connection_closed_id:
	    	{
	    		//connection is closed to update display and state
	    		LCD_write("Disconnected", LCD_ROW_CONNECTION);
	    		LCD_write(" ", LCD_ROW_CLIENTADDR);
	    		LCD_write(" ", LCD_ROW_PASSKEY);
	    		LCD_write(" ", LCD_ROW_ACTION);
	    		LCD_write(" ", LCD_ROW_TEMPVALUE);
				Dev_State = disconnected;
	    		/* Check if need to boot to dfu mode */
	    		if(boot_to_dfu)
	    		{
	    			/* Enter to DFU OTA mode */
	    			gecko_cmd_system_reset(2);
	    		}
	    		else
	    		{
	    			// Restart scanning after server has disconnected
	    			Dev_State = scanning;
	    			gecko_cmd_le_gap_start_discovery(le_gap_phy_1m, le_gap_discover_generic);
	    			LCD_write("Scanning", LCD_ROW_ACTION);
	    		}

	    		break;
	    	}

	    	//server has responded to service search query
	    	case gecko_evt_gatt_service_id:
	    	{
	    		//sanity checks
	    		if(bonding_failure_flag == false)
	    		{
		    		if(Dev_State == primary_services_search)
		    		{
		    			//check whether the server returned service is the targeted one or not
			    		Custom_BLE_Client_Look_for_Service(Health_Thermometer_Service,
			    				Health_Thermometer_Service_Length);
		    		}
	    		}

	    		break;
	    	}

	    	//server has responded to characteristic search query
	    	case gecko_evt_gatt_characteristic_id:
	    	{
	    		//sanity checks
	    		if(bonding_failure_flag == false)
	    		{
		    		if(Dev_State == characteristics_search)
		    		{
		    			//find if the responded characteristic is the targeted one or not
			    		Custom_BLE_Client_Look_for_Characteristic(Temperature_Measurement_Characteristic,
			    				Temperature_Measurement_Characteristic_Length);
		    		}
	    		}

	    		break;
	    	}

	    	//server has sent indication for a characteristic value
	    	case gecko_evt_gatt_characteristic_value_id:
	    	{
	    		//sanity checks
	    		if(bonding_failure_flag == false)
	    		{
		    		if(Dev_State == setup_success)
		    		{
		    			//send confirmation to indication, read and display temperature
		    			Custom_BLE_Client_Get_Temperature_Send_Confirmation();

		    			//read rssi value from the packet
		    			gecko_cmd_le_connection_get_rssi(Connection_Handle);
		    		}
	    		}

	    		break;
	    	}

	    	//rssi value available
	    	case gecko_evt_le_connection_rssi_id:
	    	{
	    		//display rssi value
	    		int8_t Rssi_Val = evt->data.evt_le_connection_rssi.rssi;
	    		char Rssi_Print[13];
	    		snprintf(Rssi_Print, 13, "RSSI: %ddBm", Rssi_Val);
	    		LCD_write(Rssi_Print, LCD_ROW_ACTION);
	    		break;
	    	}

	    	//most gatt API calls will generate this event at the end
	    	case gecko_evt_gatt_procedure_completed_id:
	    	{
	    		if(bonding_failure_flag == false)
	    		{
	    			//handling for each possible state
		    		switch(Dev_State)
		    		{
		    			//targeted service wasn't found
						case primary_services_search:
						{
							//so close the connection
							Custom_BLE_Client_Close_Connection();
							break;
						}

						//service found and API completed
						case health_thermometer_service_found:
						{
							//start searching for targeted characteristic
							Custom_BLE_Client_Discover_Characteristics_by_UUID(Service_Handle,
									Temperature_Measurement_Characteristic_Length, Temperature_Measurement_Characteristic);
							break;
						}

						//targeted characteristic was found and the API completed
						case temperature_characterstic_found:
						{
							//set indication settings for the particular characteristic
							Custom_BLE_Client_Set_Indications(Characteristic_Handle);
							break;
						}

						//indications set and API completed
						case indications_enabled:
						{
							//everything is setup
							Dev_State = setup_success;
							break;
						}

						default:
						{
							break;
						}
		    		}
	    		}

	    		break;
	    	}

	    	// External event has occurred - from interrupt
	    	case gecko_evt_system_external_signal_id:
	    	{
    			// Reading event ID in case of multiple events
    			Event_Read = evt->data.evt_system_external_signal.extsignals;
    			if(Event_Read == Event_Button0_Pressed) // Searching for button0 ISR
    			{
    				if(button0_read == true)	//Button is pressed
    				{
    					switch(Dev_State)
    					{

    						//connection was opened and key pressed, so must be passkey event trigger
    						case connection_opened:
    						{
    							//confirm the passkey
								gecko_cmd_sm_passkey_confirm(Connection_Handle, true);
								LCD_write("Passkey Confirmed", LCD_ROW_ACTION);
								Dev_State = paired;
								LCD_write("            ", LCD_ROW_PASSKEY);
								break;
    						}

//    						case paired:
//    						{
//    							gecko_cmd_sm_bonding_confirm(Connection_Handle, true);
//    							break;
//    						}

    						default:
    						{
    							break;
    						}
    					}

    					button0_read = false;
    				}
    			}
    			else if(Event_Read == Event_Button1_Pressed)
    			{
    	    		if(button1_read == true)
    	    		{
    					switch(Dev_State)
    					{
    						case connection_opened:
    						{
    	    	    			gecko_cmd_sm_passkey_confirm(Connection_Handle, false);
    	    	    			LCD_write("Passkey Rejected", LCD_ROW_PASSKEY);
    							break;
    						}

//    						case paired:
//    						{
//    							gecko_cmd_sm_bonding_confirm(Connection_Handle, false);
//    							break;
//    						}

    						default:
							{
    							break;
							}
    					}
    	    			button1_read = false;
    	    		}
    			}

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