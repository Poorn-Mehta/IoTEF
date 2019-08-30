//***********************************************************************************
// Include files
//***********************************************************************************
#include "src/Custom_Si7021.h"

//***********************************************************************************
// Functions
//***********************************************************************************

// Firmware Revision verifying function
bool Si7021_Verify_Firmware(void)
{
	// Setting up Tx array for command
	 Custom_I2C0_Tx_Data[0] = 0x84;
	 Custom_I2C0_Tx_Data[1] = 0xB8;

	 // Sending address and then command
	 Custom_I2C0_Write(Si7021_Address, 2);

	 // Reading back data (firmware version)
	 Custom_I2C0_Read(Si7021_Address, 1);
	 if(Custom_I2C0_Rx_Data[0] == Si7021_Firmware_Value)
	 {
		 return 0;	// Return with success
	 }
	 return 1;	// Return with error
}

// Serial number verifying function
bool Si7021_Electronic_Serial_Number(void)
{
	// Setting up Tx array for command
	Custom_I2C0_Tx_Data[0] = 0xFC;
	Custom_I2C0_Tx_Data[1] = 0xC9;

	// Sending address and then command
	Custom_I2C0_Write(Si7021_Address, 2);

	// Reading back data
	Custom_I2C0_Read(Si7021_Address, 8);
	if(Custom_I2C0_Rx_Data[0] == Si7021_Electronic_Serial_Number_Value)
	{
		return 0;	// If matched then return without error
	}
	return 1;	// Return with error
}

// Temperature reading function
void Si7021_Get_Temperature_C(void)
{
	// Command for getting temperature with hold master mode
	Custom_I2C0_Tx_Data[0] = 0xE3;

	// Write command, and read data
	Custom_I2C0_Write(Si7021_Address, 1);
	Custom_I2C0_Read(Si7021_Address, 2);

	// Combine 2 bytes to form original data
	raw_temp = ((Custom_I2C0_Rx_Data[0] << 8) + Custom_I2C0_Rx_Data[1]);

	// Convert raw data to C
	Si7021_Temperature_C = (((175.72 * raw_temp) / 65536) - 46.85) * 1000;
}
