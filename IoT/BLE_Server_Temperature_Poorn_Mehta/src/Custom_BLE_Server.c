

//***********************************************************************************
// Include files
//***********************************************************************************
#include "src/Custom_BLE_Server.h"

//***********************************************************************************
// Functions
//***********************************************************************************

void Custom_BLE_Server_Get_Temperature_Send_Notification(void)
{
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
