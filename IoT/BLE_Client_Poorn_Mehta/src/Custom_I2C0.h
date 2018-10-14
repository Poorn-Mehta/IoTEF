//***********************************************************************************
// Include files
//***********************************************************************************
#ifndef		__Custom_I2C0_h__
#define 	__Custom_I2C0_h__

#include "src/main.h"
#include "em_i2c.h"
#include "em_cmu.h"

//***********************************************************************************
// defined files
//***********************************************************************************

// Some macros
#define Custom_I2C0_Start()	I2C0->CMD = (I2C_CMD_START)
#define Custom_I2C0_Abort()	I2C0->CMD = (I2C_CMD_ABORT)
#define Custom_I2C0_EN()	I2C0->CTRL |= (I2C_CTRL_EN)
#define Custom_I2C0_Stop()	I2C0->CMD = (I2C_CMD_STOP)
#define Custom_I2C0_Send_Nack()		I2C0->CMD = (I2C_CMD_NACK)
#define Custom_I2C0_Send_Ack()		I2C0->CMD = (I2C_CMD_ACK)
#define Custom_I2C0_Tx(x)	I2C0->TXDATA = (x)
#define Custom_I2C0_Rx()		I2C0->RXDATA
#define Custom_I2C0_Wait_TXC()		while(I2C0->IF == ~(I2C_IF_TXC))
#define Custom_I2C0_Wait_State(x)	while(I2C0->STATE != (x))
#define Custom_I2C0_Wait_RXC()		while(I2C0->IF == ~(I2C_IF_RXDATAV))
#define Custom_I2C0_Address_ACK_Wait_State		0x97
#define Custom_I2C0_Data_ACK_Wait_State			0xD7
#define Custom_I2C0_RXDATAV_Wait_State			0xB3

// Currently, not supporting more than 255. To add more bytes, change data type of variable handling loops
#define Custom_I2C0_Max_Rx_Data		100
#define Custom_I2C0_Max_Tx_Data		100
#define Custom_I2C0_Freq		 	I2C_FREQ_FAST_MAX //	I2C_FREQ_STANDARD_MAX

// Sensors defines are required over here to route pins of I2C
#define Si7021_SCL_Port		gpioPortC
#define Si7021_SCL_Pin		10
#define Si7021_SDA_Port		gpioPortC
#define Si7021_SDA_Pin		11

// Not using these values for now. A switch structure can be made later on to automatically route pins
#define Si7021_SCL_Route	14
#define Si7021_SDA_Route	16

//***********************************************************************************
// global variables
//***********************************************************************************

volatile uint8_t Custom_I2C0_Rx_Data[Custom_I2C0_Max_Rx_Data];	// Volatile because it is going be
																// changing from outside of the
																// scope of code
uint8_t Custom_I2C0_Tx_Data[Custom_I2C0_Max_Tx_Data];
uint32_t i2c0_dly;
extern const I2C_Init_TypeDef  Custom_I2C0_Init_Struct;	// extern because it is shared

//***********************************************************************************
// function prototypes
//***********************************************************************************

void Custom_I2C0_Reset(void);
void Custom_I2C0_Read_Array_Cleanup(void);
void Custom_I2C0_Clock_Enable(void);
void Custom_I2C0_Reset_Slave(void);
void Custom_I2C0_Init(void);
bool Custom_I2C0_Read(uint8_t address, uint8_t length);
bool Custom_I2C0_Write(uint8_t address, uint8_t length);

#endif
