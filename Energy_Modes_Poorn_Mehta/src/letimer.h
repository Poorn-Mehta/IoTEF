/*
 * letimer.h
 *
 *  Created on: Jan 29, 2019
 *      Author: poorn
 */

#ifndef LETIMER_H_
#define LETIMER_H_

#include "src/main.h"
#include "src/gpio.h"

#define Total_Modes			5	// Number of EM modes in Blue Gecko
#define Lowest_Allowed_Sleep_Mode	3	// Modify to compare avg current consumption
#define LFXO_Frequency		32768	// Took the value from datasheet
#define ULFRCO_Frequency	1000	// Took the value from datasheet
#define LETIMER_Counter_Max		65535		// (2^16) - 1
#define Max_Prescaling_for_LETIMER	15	// Value from reference manual
#define LED_On_Time			175		// Time in Milliseconds, Vary the value for
									// different duty cycles
#define Period		2250				// Time in Milliseconds

void LETIMER0_IRQHandler(void);
void Set_Letimer_Clock_Prescaler(bool LFXO_or_ULFRCO);
void Setting_Letimer_Clock(void);
void Blue_Gecko_BlockSleepMode(uint8 Required_Mode);
void Blue_Gecko_UnblockSleepMode(uint8 Required_Mode);
void Blue_Gecko_Sleep(void);
void Blue_Gecko_Letimer_Init(void);

extern uint8_t EM_Array[Total_Modes];
extern uint32_t Interrupt_Flags;

#endif /* LETIMER_H_ */
