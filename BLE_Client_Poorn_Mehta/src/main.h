#ifndef		__main_h__
#define 	__main_h__

//***********************************************************************************
// Include files
//***********************************************************************************

#include <stdint.h>
#include <stdbool.h>

#include "lcd_driver.h"

#include "platform/emdrv/sleep/inc/sleep.h"

#include "em_emu.h"
#include "em_cmu.h"
#include "em_core.h"
#include "stdlib.h"
#include "stdio.h"

/* Board headers */
#include "init_mcu.h"
#include "init_board.h"
#include "init_app.h"
#include "ble-configuration.h"
#include "board_features.h"

/* Bluetooth stack headers */
#include "bg_types.h"
#include "native_gecko.h"
#include "gatt_db.h"
#include "infrastructure.h"

#endif

//***********************************************************************************
// defined files
//***********************************************************************************

#define Event_Letimer_Period_Reached	1
#define Event_Button0_Pressed		2
#define Event_Button1_Pressed		3
//#define Event_Si7021_Temperature_Read_Complete	3

//***********************************************************************************
// global variables
//***********************************************************************************

extern struct gecko_cmd_packet* evt;

extern bd_addr Server_Addr;

extern uint8_t Connection_Handle, Services_Count, Characteristics_Count;

extern bool bonding_failure_flag;

extern uint32_t Service_Handle;
extern uint16_t Characteristic_Handle;

//***********************************************************************************
// function prototypes
//***********************************************************************************
