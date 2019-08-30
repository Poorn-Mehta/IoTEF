/*
 * fingerprint.h
 *
 *  Created on: Dec 3, 2018
 *      Author: poorn
 */

#ifndef FINGERPRINT_H_
#define FINGERPRINT_H_

#include "src/main.h"
#include "src/gpio.h"
#include "src/mesh_friend.h"

#define Fingerprint_UART	USART2
#define	Fingerprint_Clock	cmuClock_USART2

#define UART2_Port			gpioPortF
#define UART2_Tx			4
#define UART2_Tx_Loc		17 << 8
#define UART2_Tx_En			1 << 1
#define UART2_Rx			5
#define UART2_Rx_Loc		17 << 0
#define UART2_Rx_En			1 << 0
#define	Fingerprint_Port  	UART2_Port
#define Fingerprint_Tx   	UART2_Tx
#define Fingerprint_Tx_Loc	UART2_Tx_Loc
#define Fingerprint_Tx_En	UART2_Tx_En
#define Fingerprint_Rx_En	UART2_Rx_En
#define Fingerprint_Rx   	UART2_Rx
#define Fingerprint_Rx_Loc	UART2_Rx_Loc

#define Fingerprint_Start_Byte1		0x55
#define Fingerprint_Start_Byte2		0xAA
#define Fingerprint_Address			0x0001
#define Fingerprint_ACK_Resp		0x30

#define Fingerprint_Cmd_Resp_Checksum_Length	10
#define Fingerprint_Cmd_Resp_Length		12

#define Fingerprint_Open			0x01
#define Fingerprint_ChangeBaud		0x04
#define Fingerprint_LED				0x12
#define Fingerprint_GetEnrollCount	0x20
#define Fingerprint_CheckEnrolled	0x21
#define Fingerprint_EnrollStart		0x22
#define Fingerprint_Enroll1			0x23
#define Fingerprint_Enroll2			0x24
#define Fingerprint_Enroll3			0x25
#define Fingerprint_IsPressFinger	0x26
#define Fingerprint_DeleteID		0x40
#define Fingerprint_DeleteAll		0x41
#define Fingerprint_Verify			0x50
#define Fingerprint_Identify		0x51
#define Fingerprint_CaptureFinger	0x60

#define Fingerprint_Error_Authorization		0x1008

#define Fingerprint_NACK_Msg()			LCD_write("Error - NACK", Fingerprint_Row)
#define Fingerprint_Err_Para_Msg()		LCD_write("Error - FP Parameter", Fingerprint_Row)
#define Fingerprint_No_Finger_Msg()		LCD_write("No Finger", Fingerprint_Row)
#define Fingerprint_Remove_Finger_Msg()	LCD_write("Remove Finger", Fingerprint_Row)

enum {
//	No_Finger_on_Sensor,
//	Finger_is_on_Sensor,
	Authorization_not_Checked,
	Authorization_Completed
} Fingerprint_State;


void Fingerprint_Init(void);
void Fingerprint_Read_Byte(volatile uint8_t *address);
void Fingerprint_Write_Byte(uint8_t data);
uint16_t Fingerprint_Calculate_Checksum(uint8_t *fp_chk, uint8_t len);
void Fingerprint_Set_Send_Command_Packet(uint16_t cmd, uint32_t param, uint8_t *fp_tx);
void Fingerprint_Get_Read_Response_Packet(uint16_t *resp, uint32_t *param, volatile uint8_t *fp_rx);
void Fingerprint_Setup(uint8_t LED_State);
uint8_t Fingerprint_Check_for_Finger(uint8_t state);
void Fingerprint_Wait_for_Finger(uint8_t state);
void Fingerprint_Delete_Everything(void);
void Fingerprint_Add_Fingerprint(void);
void Fingerprint_Check_Authorization(void);

#endif /* FINGERPRINT_H_ */
