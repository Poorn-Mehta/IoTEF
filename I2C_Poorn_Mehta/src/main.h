#ifndef		__main_h__
#define 	__main_h__

//***********************************************************************************
// Include files
//***********************************************************************************

#include <stdint.h>
#include <stdbool.h>

#include "em_emu.h"
#include "em_cmu.h"
#include "em_core.h"

/* Board headers */
#include "init_mcu.h"
#include "init_board.h"
#include "ble-configuration.h"
#include "board_features.h"

/* Bluetooth stack headers */
#include "bg_types.h"
#include "native_gecko.h"
#include "gatt_db.h"

#endif

//***********************************************************************************
// defined files
//***********************************************************************************

#define Event_Letimer_Period_Reached	1
#define Event_Timer0_Period_Reached		2
//#define Event_Si7021_Temperature_Read_Complete	3

//***********************************************************************************
// global variables
//***********************************************************************************

//extern uint8_t Global_Events;

//***********************************************************************************
// function prototypes
//***********************************************************************************
