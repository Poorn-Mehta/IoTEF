#ifndef		__Custom_Sleep_h__
#define 	__Custom_Sleep_h__

//***********************************************************************************
// Include files
//***********************************************************************************
#include "src/main.h"

//***********************************************************************************
// defined files
//***********************************************************************************

#define Total_Modes			5	// Number of EM modes in Blue Gecko
#define Lowest_Allowed_Sleep_Mode	3	// Modify to compare avg current consumption


//***********************************************************************************
// global variables
//***********************************************************************************



//***********************************************************************************
// function prototypes
//***********************************************************************************
void Blue_Gecko_Clear_EM_Array(void);
void Blue_Gecko_BlockSleepMode(uint8 Required_Mode);
void Blue_Gecko_UnblockSleepMode(uint8 Required_Mode);
void Blue_Gecko_Sleep(void);

#endif
