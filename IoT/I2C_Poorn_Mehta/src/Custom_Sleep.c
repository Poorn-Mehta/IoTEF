//***********************************************************************************
// Include files
//***********************************************************************************
#include "src/Custom_Sleep.h"

//***********************************************************************************
// Functions
//***********************************************************************************

/*******************************************************************************
 * # License
 * <b>Copyright 2016 Silicon Laboratories, Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 *
 * DISCLAIMER OF WARRANTY/LIMITATION OF REMEDIES: Silicon Labs has no
 * obligation to support this Software. Silicon Labs is providing the
 * Software "AS IS", with no express or implied warranties of any kind,
 * including, but not limited to, any implied warranties of merchantability
 * or fitness for any particular purpose or warranties against infringement
 * of any proprietary rights of a third party.
 *
 * Silicon Labs will not be liable for any consequential, incidental, or
 * special damages, or any other relief, or for any claim by any third party,
 * arising from your use of this Software.
 *
 ******************************************************************************/
/* The above Silicon Laboratories IP has been applied, and adhered to by the
 * developer for the following three functions:
 * Blue_Gecko_BlockSleepMode()
 * Blue_Gecko_UnblockSleepMode()
 * Blue_Gecko_Sleep()
 */

// Following 3 functions ensure that while a peripheral is running, and is having a particular lowest
// Energy mode in which it can operate, and not below it, then the module won't drop below that
// Particular lowest mode. If a peripheral is enabled and can be operated down to EM2 only and not the
// EM3, then block function will increase the value of 3rd element of array, preventing sleep function
// To drop down below EM2. The sleep function will go to the lowest allowed energy mode.
// In this application, LETIEMR is running forever and thus Unblock function hasn't been used.
// However, if needed, it can be called at the end of the peripheral disabling function.
// Also, arrays are used because multiple peripherals can increase the value, and the particular
// EM mode will stay as the lowest allowed EM mode, until all of the peripherals using that mode
// are disabled.

uint8_t EM_Array[Total_Modes];	// Protected by atomic operation

void Blue_Gecko_Clear_EM_Array(void)	// Setting all 0 at init
{
	uint8_t i;
	for (i = 0; i < Total_Modes; i ++)
	{
		EM_Array[i] = 0;
	}
}

// Blocking mode that is required by peripherals.
void Blue_Gecko_BlockSleepMode(uint8 Required_Mode)
{
	CORE_AtomicDisableIrq();
	EM_Array[Required_Mode] += 1;
	CORE_AtomicEnableIrq();
}

void Blue_Gecko_UnblockSleepMode(uint8 Required_Mode)
{
	CORE_AtomicDisableIrq();	// Disabling interrupts so that variable doesn't get corrupted
	if(EM_Array[Required_Mode] > 0)
	{
		EM_Array[Required_Mode] -= 1;	// Deduct 1 for indicating that a peripheral
		// Using that mode has just turned off
	}
	else
	{
		EM_Array[Required_Mode] = 0;	// Just as a precaution
	}
	CORE_AtomicEnableIrq();
}

void Blue_Gecko_Sleep(void)
{
	// Following conditions indicate that some peripheral(s) has/have set the respective modes
	// As their own lowest allowed energy modes. The sleep function will take care of all the
	// Peripherals by stepping down one EM mode at time, from EM0 to EM4(EM3 for now)
	// Only after ensuring that the mode which it has skipped (ex EM1 for the condition of EM_Array[2] > 0)
	// Was not being used by any active peripheral (In other words, all the active peripherals are
	// Able to work in an EM mode which is lower than the skipped one); it will step down further.
	if(EM_Array[0] > 0)
	{
		return;
	}
	else if(EM_Array[1] > 0)
	{
		EMU_EnterEM1();
	}
	else if(EM_Array[2] > 0)
	{
		EMU_EnterEM2(true);
	}
	else	// Not configured for EM4 now
	{
		EMU_EnterEM3(true);
	}
}
