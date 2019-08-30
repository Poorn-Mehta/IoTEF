#ifndef		__Custom_BLE_Server_h__
#define 	__Custom_BLE_Server_h__

//***********************************************************************************
// Include files
//***********************************************************************************
#include "src/main.h"
#include "src/gpio.h"
#include "src/Custom_Si7021.h"

//***********************************************************************************
// defined files
//***********************************************************************************

#define Conn_Int_Min_Val	60	// 60*1.25 = 75ms
#define Conn_Int_Max_Val	60
#define Slave_Lat_Val	3		// Skipping 3, connection event on 4th - 4*75 = 300ms
#define Timeout_Val		300
#define BG13_Min_Power	-300	// From datasheet, value in dB = absolute value / 10
#define BG13_Max_Power	105		// From datasheet

//***********************************************************************************
// global variables
//***********************************************************************************



//***********************************************************************************
// function prototypes
//***********************************************************************************

void Custom_BLE_Server_Get_Temperature_Send_Notification(void);

#endif
