/***********************************************************************************************//**
 * \file   main.c
 * \brief  Silicon Labs Thermometer Example Application
 *
 * This Thermometer and OTA example allows the user to measure temperature
 * using the temperature sensor on the WSTK. The values can be read with the
 * Health Thermometer reader on the Blue Gecko smartphone app.
 ***************************************************************************************************
 * <b> (C) Copyright 2016 Silicon Labs, http://www.silabs.com</b>
 ***************************************************************************************************
 * This file is licensed under the Silicon Labs License Agreement. See the file
 * "Silabs_License_Agreement.txt" for details. Before using this software for
 * any purpose, you must agree to the terms of that agreement.
 **************************************************************************************************/

//***********************************************************************************
// Include files
//***********************************************************************************
#include "src/Custom_BLE_Server.h"

//***********************************************************************************
// Functions
//***********************************************************************************

//bool bonding_status = 0;

void Server_Address(void)
{
	struct gecko_msg_system_get_bt_address_rsp_t* Server_Address;
	char Server_Address_Print[18];
	Server_Address = gecko_cmd_system_get_bt_address();
	snprintf(Server_Address_Print, sizeof(Server_Address_Print), "%0.2X:%0.2X:%0.2X:%0.2X:%0.2X:%0.2X",
			Server_Address->address.addr[5], Server_Address->address.addr[4], Server_Address->address.addr[3],
			Server_Address->address.addr[2], Server_Address->address.addr[1], Server_Address->address.addr[0]);
	LCD_write("BT ADDR", LCD_ROW_BTADDR1);
	LCD_write(Server_Address_Print, LCD_ROW_BTADDR2);
}

void Custom_BLE_Server_Delete_Bondings(void)
{
    LCD_write("Delete Bondings?", LCD_ROW_PASSKEY);
    LCD_write("PB0-Yes PB1-No", LCD_ROW_ACTION);
    NVIC_EnableIRQ(GPIO_EVEN_IRQn);
    NVIC_EnableIRQ(GPIO_ODD_IRQn);
    SLEEP_Sleep();
    if(button0_read == true)
    {
    	gecko_cmd_sm_delete_bondings();
    }
//    else if(button1_read == true)
//    {
//    	bonding_status = 1;
//    }
    LCD_write(" ", LCD_ROW_PASSKEY);
    LCD_write(" ", LCD_ROW_ACTION);
    button0_read = false;
    button1_read = false;
}


void Custom_BLE_Server_Get_Temperature_Send_Notification(void)
{
//	  if(bonding_status)
//	  {
//		  LCD_write("Already Bonded", LCD_ROW_ACTION);
//	  }
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

	  Custom_I2C0_Abort();	// Send the abort command to completely halt the I2C bus
	  Custom_I2C0_Reset();	// Resetting the I2C registers and pin settings

	  uint8_t htmTempBuffer[5]; /* Stores the temperature data in the Health Thermometer (HTM) format. */
	  uint8_t flags = 0x00;   /* HTM flags set as 0 for Celsius, no time stamp and no temperature type. */
	  uint32_t temperature;   /* Stores the temperature data read from the sensor in the correct format */
	  uint8_t *p = htmTempBuffer; /* Pointer to HTM temperature buffer needed for converting values to bitstream. */

	    /* Convert flags to bitstream and append them in the HTM temperature data buffer (htmTempBuffer) */
	  UINT8_TO_BITSTREAM(p, flags);

	      /* Convert sensor data to correct temperature format */
	  temperature = FLT_TO_UINT32(Si7021_Temperature_C, -3);

	  /* Convert temperature to bitstream and place it in the HTM temperature data buffer (htmTempBuffer) */
	  UINT32_TO_BITSTREAM(p, temperature);

	  /* Send indication of the temperature in htmTempBuffer to all "listening" clients.
	       * This enables the Health Thermometer in the Blue Gecko app to display the temperature.
	       *  0xFF as connection ID will send indications to all connections. */
	  gecko_cmd_gatt_server_send_characteristic_notification(
	        0xFF, gattdb_temperature_measurement, 5, htmTempBuffer);
}
