#ifndef LCD_DRIVER_H_
#define LCD_DRIVER_H_

#include "hal-config.h"

#if (HAL_SPIDISPLAY_ENABLE == 1)

#include "bg_types.h"

/**
 *  LCD content can be updated one row at a time using function LCD_write().
 *  Row number is passed as parameter,the possible values are defined below.
 */

#define Name_Row			1
#define Provision_Status	2
#define Mesh_Status			3
#define Friendship_Status	4
#define Passkey_Value		4
#define Fingerprint_Row		5
#define Mesh_Data			6
#define Flash_Data			7
#define Alarm_Value			8
#define Alarm_Row			9
#define Moisture_Row		10
#define Smoke_Row			11
#define Threshold_Row		12


#define LCD_ROW_MAX          13    /* total number of rows used */


#define LCD_ROW_LEN        32   /* up to 32 characters per each row */

//char *header - a C string that contains the header which is persistent. For ex for a BLE Server -> header = "BLE SERVER"
void LCD_init(char *header);

//char *str  - the C string you want to display in the row number
void LCD_write(char *str, uint8 row);

#endif /* HAL_SPIDISPLAY_ENABLE */

#endif /* LCD_DRIVER_H_ */
