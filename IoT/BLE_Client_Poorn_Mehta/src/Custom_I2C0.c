//***********************************************************************************
// Include files
//***********************************************************************************
#include "src/Custom_I2C0.h"
#include "src/Custom_TIMER0.h"
#include "src/gpio.h"

//***********************************************************************************
// Functions
//***********************************************************************************

void Custom_I2C0_Reset(void)
{
	I2C_Reset(I2C0);	// Using emlib function to reset the registers

	// Resetting the pins to decrease around 40uA current draw per pin
	// when idle
	GPIO_PinModeSet(Si7021_SCL_Port, Si7021_SCL_Pin, gpioModeDisabled, 1);
	GPIO_PinModeSet(Si7021_SDA_Port, Si7021_SDA_Pin, gpioModeDisabled, 1);
}

void Custom_I2C0_Clock_Enable(void)
{
	CMU_ClockSelectSet(cmuClock_HFPER, cmuSelect_HFXO);	// Setting HFPER, on which I2C runs
	CMU_ClockEnable(cmuClock_HFPER, true);	// Enabling HFPERCLK
	CMU_ClockEnable(cmuClock_I2C0, true);	// Enabling I2C branch
}

void Custom_I2C0_Reset_Slave(void)	// This just resets the I2C state of the slave
{
	uint8_t temp_cnt;
	for(temp_cnt = 0; temp_cnt < 9; temp_cnt ++)	// Toggling SCL 9 times
	{
		GPIO_PinOutClear(Si7021_SCL_Port, Si7021_SCL_Pin);
		GPIO_PinOutSet(Si7021_SCL_Port, Si7021_SCL_Pin);
	}
}

void Custom_I2C0_Read_Array_Cleanup(void)	// Array clear function, just as a precaution
{
	uint8_t loop_counter;
	for(loop_counter = 0; loop_counter < Custom_I2C0_Max_Rx_Data; loop_counter ++)
	{
		Custom_I2C0_Rx_Data[loop_counter] = 0;
	}
}

void Custom_I2C0_Init(void)
{
	// Setting pins for I2C transfer
	GPIO_PinModeSet(Si7021_SCL_Port, Si7021_SCL_Pin, gpioModeWiredAnd, 1);
	GPIO_PinModeSet(Si7021_SDA_Port, Si7021_SDA_Pin, gpioModeWiredAnd, 1);

	// Routing to sensor
	I2C0->ROUTELOC0 = ((I2C_ROUTELOC0_SCLLOC_LOC14) | (I2C_ROUTELOC0_SDALOC_LOC16));
	I2C0->ROUTEPEN = ((I2C_ROUTEPEN_SCLPEN) | (I2C_ROUTEPEN_SDAPEN));

	// Resetting the I2C state of the sensor
	Custom_I2C0_Reset_Slave();

	// Setting up structure to use emlib I2C_Init function
	const I2C_Init_TypeDef  Custom_I2C0_Init_Struct =
			{
				false,	// Do not enable when Init is done
				true,	// Master Mode
				0,		// Use HFPERCLK
				Custom_I2C0_Freq,	// Operating Freq
				i2cClockHLRStandard	// Clock low to high ratio
			};
	I2C_Init(I2C0, &Custom_I2C0_Init_Struct);	// Calling emlib function

	// Aborting to reset the I2C from master side
	Custom_I2C0_Abort();

	// Enabling the functioning of I2C
	Custom_I2C0_EN();

	// Cleaning up received array, just as a precaution
	Custom_I2C0_Read_Array_Cleanup();
}

// Writing to slave function, Master Transmit mode
bool Custom_I2C0_Write(uint8_t address, uint8_t length)
{
	if(length <= Custom_I2C0_Max_Tx_Data)	// Check for range of data
	{
		uint8_t loop_counter;	// Setting up counter for multibyte transfer
		address <<= 1;		// 7 bit of address, adjusting
		address &= ~(0x01);		// Writing, so clearing LSB
		Custom_I2C0_Start();	// Sending start condition
		Custom_I2C0_Tx(address);	// Transmitting address
		Custom_I2C0_Wait_TXC();		// Waiting for transmission to be completed
		Custom_I2C0_Wait_State(Custom_I2C0_Address_ACK_Wait_State);	// Waiting for ACK. Set the breakpoint to check for NACK
		for(loop_counter = 0; loop_counter < length; loop_counter ++)	// Transmitting bytes
		{
			Custom_I2C0_Tx(Custom_I2C0_Tx_Data[loop_counter]);	// Putting data inside TXDATA register
			Custom_I2C0_Wait_TXC();	// Waiting for transmission to be completed
			Custom_I2C0_Wait_State(Custom_I2C0_Data_ACK_Wait_State);	// Waiting for ACK. Set up breakpoint to check for NACK
		}
		return 0;	// Return with no error
	}
	return 1;	// Return with error
}

// Reading bytes from Slave. Master Receiver mode.
bool Custom_I2C0_Read(uint8_t address, uint8_t length)
{
	if(length <= Custom_I2C0_Max_Rx_Data)	// Check for range of data
	{
		uint8_t loop_counter;	// Setting up counter for multibyte transfer
		address <<= 1;	// 7 bit of address, adjusting
		address |= 0x01;	// Reading, so setting LSB
		Custom_I2C0_Start();	// Sending start condition
		Custom_I2C0_Tx(address);	// Transmitting address
		Custom_I2C0_Wait_TXC();	// Waiting for transmission to be completed
		Custom_I2C0_Wait_RXC();	// Waiting for RXDATAV flag to be set. Ideally I should be
								// checking for ACK from slave, and thus the related state
								// but debugging showed me that i2c is never entering that
								// state, and my program was getting stuck too in that line so
		Custom_I2C0_Wait_State(Custom_I2C0_RXDATAV_Wait_State);	// Waiting for RXDATAV flag to be reflected on state
		for(loop_counter = 0; loop_counter < length; loop_counter ++)	// Multibyte read
		{
			Custom_I2C0_Rx_Data[loop_counter] = Custom_I2C0_Rx();	// Reading byte from RXDATA register
			if(loop_counter < (length - 1))	// Execute for all except last byte
			{
				Custom_I2C0_Send_Ack();		// Sending ACK from Master
				Custom_I2C0_Wait_RXC();		// Wait for next byte
				Custom_I2C0_Wait_State(Custom_I2C0_RXDATAV_Wait_State);	// Reflecting RXDATAV flag in state
			}
			else
			{
				Custom_I2C0_Send_Nack();	// Last byte, so send NACK from Master
				Custom_I2C0_Stop();		// Stop the I2C data transfer
			}
		}
		return 0;	// Return without error
	}
	return 1;	// Return with error
}
