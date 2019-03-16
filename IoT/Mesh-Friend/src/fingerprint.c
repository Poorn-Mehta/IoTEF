/*
 * fingerprint.c
 *
 *  Created on: Dec 3, 2018
 *      Author: poorn
 */

#include "src/fingerprint.h"

uint8_t FP_Tx_Array[30];
uint32_t FP_Parameter;
volatile uint8_t FP_Rx_Array[30];
uint16_t command, response;
uint8_t Fingerprint_Current_ID = 0;
uint8_t FP_Button;

void Fingerprint_Init(void)
{
	CMU_ClockSelectSet(cmuClock_HFPER, cmuSelect_HFXO);
	CMU_ClockEnable(cmuClock_HFPER, true);
	CMU_ClockEnable(cmuClock_GPIO, true);
	CMU_ClockEnable(Fingerprint_Clock, true);

	GPIO_DriveStrengthSet(Fingerprint_Port, gpioDriveStrengthStrongAlternateStrong);

	GPIO_PinModeSet(Fingerprint_Port, Fingerprint_Tx, gpioModePushPull, 1);
	GPIO_PinModeSet(Fingerprint_Port, Fingerprint_Rx, gpioModeInput, 0);

	const USART_InitAsync_TypeDef Fingerprint_Init_Struct =
						  {
							usartDisable,           /* Enable RX/TX when initialization is complete. */
							0,                     /* Use current configured reference clock for configuring baud rate. */
							9600,                /* 9600 bits/s. */
							usartOVS16,            /* 16x oversampling. */
							usartDatabits8,        /* 8 data bits. */
							usartNoParity,         /* No parity. */
							usartStopbits1,        /* 1 stop bit. */
							false,                 /* Do not disable majority vote. */
							false,                 /* Not USART PRS input mode. */
							usartPrsRxCh0,         /* PRS channel 0. */
							false,                 /* Auto CS functionality enable/disable switch */
							0,                     /* Auto CS Hold cycles */
							0,                     /* Auto CS Setup cycles */
							usartHwFlowControlNone /* No HW flow control */
						  };

	USART_InitAsync(Fingerprint_UART, &Fingerprint_Init_Struct);

	Fingerprint_UART->ROUTELOC0 = ((Fingerprint_Tx_Loc) | (Fingerprint_Rx_Loc));
	Fingerprint_UART->ROUTEPEN = ((Fingerprint_Tx_En) | (Fingerprint_Rx_En));

	USART_Enable(Fingerprint_UART, usartEnable);

	printf("Init Completed\n\r");

}

void Fingerprint_Read_Byte(volatile uint8_t *address)
{
	CORE_AtomicDisableIrq();
	*address = (volatile uint8_t)USART_Rx(Fingerprint_UART);
	CORE_AtomicEnableIrq();
}

void Fingerprint_Write_Byte(uint8_t data)
{
	CORE_AtomicDisableIrq();
	USART_Tx(Fingerprint_UART, data);
	CORE_AtomicEnableIrq();
}

uint16_t Fingerprint_Calculate_Checksum(uint8_t *fp_chk, uint8_t len)
{
	uint8_t chk_i;
	uint16_t chksum = 0;
	for(chk_i = 0; chk_i < len; chk_i ++)
	{
		chksum += *fp_chk++;
	}
	return chksum;
}

void Fingerprint_Set_Send_Command_Packet(uint16_t cmd, uint32_t param, uint8_t *fp_tx)
{
	uint16_t chksum;
	uint8_t *tmp, tmp_i;
	tmp = fp_tx;
	*fp_tx++ = Fingerprint_Start_Byte1;
	*fp_tx++ = Fingerprint_Start_Byte2;
	*fp_tx++ = Fingerprint_Address & 0x00FF;
	*fp_tx++ = (Fingerprint_Address & 0xFF00) >> 8;
	*fp_tx++ = param & 0x000000FF;
	*fp_tx++ = (param & 0x0000FF00) >> 8;
	*fp_tx++ = (param & 0x00FF0000) >> 16;
	*fp_tx++ = (param & 0xFF000000) >> 24;
	*fp_tx++ = cmd & 0x00FF;
	*fp_tx++ = (cmd & 0xFF00) >> 8;
	chksum = Fingerprint_Calculate_Checksum(tmp, Fingerprint_Cmd_Resp_Checksum_Length);
	*fp_tx++ = chksum & 0x00FF;
	*fp_tx++ = (chksum & 0xFF00) >> 8;
	fp_tx = tmp;
	for(tmp_i = 0; tmp_i < Fingerprint_Cmd_Resp_Length; tmp_i ++)
	{
		Fingerprint_Write_Byte(*fp_tx++);
	}
}

void Fingerprint_Get_Read_Response_Packet(uint16_t *resp, uint32_t *param, volatile uint8_t *fp_rx)
{
	uint16_t chksum;
	uint8_t *tmp, tmp_i;
	*param = 0;
	*resp = 0;
	tmp = (uint8_t *)fp_rx;
	for(tmp_i = 0; tmp_i < Fingerprint_Cmd_Resp_Length; tmp_i ++)
	{
		Fingerprint_Read_Byte(fp_rx++);
	}
	fp_rx = tmp;
	if(*fp_rx++ != Fingerprint_Start_Byte1)		printf("Error: Expected 0x%02x Got 0x%02x\n\r", \
														Fingerprint_Start_Byte1, *fp_rx);
	if(*fp_rx++ != Fingerprint_Start_Byte2)		printf("Error: Expected 0x%02x Got 0x%02x\n\r", \
														Fingerprint_Start_Byte2, *fp_rx);
	if(*fp_rx++ != (Fingerprint_Address & 0x00FF))		printf("Error: Expected 0x%02x Got 0x%02x\n\r", \
																(Fingerprint_Address & 0x00FF), *fp_rx);
	if(*fp_rx++ != ((Fingerprint_Address & 0xFF00) >> 8))		printf("Error: Expected 0x%02x Got 0x%02x\n\r", \
																		((Fingerprint_Address & 0xFF00) >> 8), *fp_rx);

	*param |= *fp_rx++;
	*param |= (*fp_rx++) << 8;
	*param |= (*fp_rx++) << 16;
	*param |= (*fp_rx++) << 24;

	*resp |= *fp_rx++;
	*resp |= (*fp_rx++) << 8;

	chksum = Fingerprint_Calculate_Checksum(tmp, Fingerprint_Cmd_Resp_Checksum_Length);

	if(*fp_rx++ != (chksum & 0x00FF))			printf("Error: Expected 0x%02x Got 0x%02x\n\r", \
														(chksum & 0x00FF), *fp_rx);
	if(*fp_rx++ != ((chksum & 0xFF00) >> 8))			printf("Error: Expected 0x%02x Got 0x%02x\n\r", \
														((chksum & 0xFF00) >> 8), *fp_rx);
}

void Fingerprint_Setup(uint8_t LED_State)
{
	Fingerprint_Set_Send_Command_Packet(Fingerprint_Open, 0, &FP_Tx_Array[0]);
	Fingerprint_Get_Read_Response_Packet(&response, &FP_Parameter, &FP_Rx_Array[0]);
	if(response != Fingerprint_ACK_Resp)
	{
		printf("Err: Negative ACK\n\r");
		//Fingerprint_NACK_Msg();
	}
	else if(FP_Parameter != 0)
	{
		printf("Err: Wrong FP_Parameter\n\r");
		//Fingerprint_Err_Para_Msg();
	}
	else
	{
		printf("Fingerprint Scanner Initialized\n\r");
		//LCD_write("FP Init Done", Fingerprint_Row);
	}

	Fingerprint_Set_Send_Command_Packet(Fingerprint_LED, LED_State, &FP_Tx_Array[0]);
	Fingerprint_Get_Read_Response_Packet(&response, &FP_Parameter, &FP_Rx_Array[0]);
	if(response != Fingerprint_ACK_Resp)
	{
		printf("Err: Negative ACK\n\r");
		//Fingerprint_NACK_Msg();
	}
	else if(FP_Parameter != 0)
	{
		printf("Err: Wrong FP_Parameter\n\r");
		//Fingerprint_Err_Para_Msg();
	}
	else
	{
		printf("Fingerprint LED Configured\n\r");
		//LCD_write("FP LED Configured", Fingerprint_Row);
	}

	Fingerprint_Set_Send_Command_Packet(Fingerprint_GetEnrollCount, 0, &FP_Tx_Array[0]);
	Fingerprint_Get_Read_Response_Packet(&response, &FP_Parameter, &FP_Rx_Array[0]);
	if(response != Fingerprint_ACK_Resp)
	{
		printf("Err: Negative ACK\n\r");
		//Fingerprint_NACK_Msg();
	}
	else
	{
		printf("Currently there are %ld fingerprints are in database\n\r", FP_Parameter);
		sprintf(lcd_string, "Database - %ld", FP_Parameter);
		LCD_write(lcd_string, Fingerprint_Row);
	}
	Fingerprint_Current_ID = FP_Parameter;
	if(Fingerprint_Current_ID == 0)
	{
		printf("No Fingerprints are there in the Database\n\rAdd one now\n\r");
		LCD_write("DB Empty - Add New", Fingerprint_Row);
		Fingerprint_Add_Fingerprint();
	}

	Fingerprint_State = Authorization_not_Checked;
}

uint8_t Fingerprint_Check_for_Finger(uint8_t state)
{
	if(state)
	{
		FP_Parameter = 1;
//		printf("Checking for a finger\n\r");
		//Fingerprint_No_Finger_Msg();
		Fingerprint_Set_Send_Command_Packet(Fingerprint_IsPressFinger, 0, &FP_Tx_Array[0]);
		Fingerprint_Get_Read_Response_Packet(&response, &FP_Parameter, &FP_Rx_Array[0]);
		if(response != Fingerprint_ACK_Resp)
		{
			printf("Err: Negative ACK\n\r");
			//Fingerprint_NACK_Msg();
			return 1;
		}
		else	return (FP_Parameter);
	}
	else
	{
		FP_Parameter = 0;
//		printf("Checking to lift the finger\n\r");
		//Fingerprint_Remove_Finger_Msg();
		Fingerprint_Set_Send_Command_Packet(Fingerprint_IsPressFinger, 0, &FP_Tx_Array[0]);
		Fingerprint_Get_Read_Response_Packet(&response, &FP_Parameter, &FP_Rx_Array[0]);
		if(response != Fingerprint_ACK_Resp)
		{
			return 0;
			printf("Err: Negative ACK\n\r");
			//Fingerprint_NACK_Msg();
		}
		else	return (FP_Parameter);
	}
}

void Fingerprint_Wait_for_Finger(uint8_t state)
{
	if(state)
	{
		FP_Parameter = 1;
		printf("Waiting for a finger\n\r");
		//LCD_write("ON HOLD: Put Finger", Fingerprint_Row);
		while(FP_Parameter != 0)
		{
			Fingerprint_Set_Send_Command_Packet(Fingerprint_IsPressFinger, 0, &FP_Tx_Array[0]);
			Fingerprint_Get_Read_Response_Packet(&response, &FP_Parameter, &FP_Rx_Array[0]);
			if(response != Fingerprint_ACK_Resp)
			{
				printf("Err: Negative ACK\n\r");
				//Fingerprint_NACK_Msg();
			}
		}
		printf("Finger Detected\n\r");
		//LCD_write("Finger Detected", Fingerprint_Row);
	}
	else
	{
		FP_Parameter = 0;
		printf("Waiting to lift the finger\n\r");
		//LCD_write("ON HOLD: Lift Finger", Fingerprint_Row);
		while(FP_Parameter == 0)
		{
			Fingerprint_Set_Send_Command_Packet(Fingerprint_IsPressFinger, 0, &FP_Tx_Array[0]);
			Fingerprint_Get_Read_Response_Packet(&response, &FP_Parameter, &FP_Rx_Array[0]);
			if(response != Fingerprint_ACK_Resp)
			{
				printf("Err: Negative ACK\n\r");
				//Fingerprint_NACK_Msg();
			}
		}
		printf("No Finger\n\r");
		//Fingerprint_No_Finger_Msg();
	}
}

void Fingerprint_Delete_Everything(void)
{
	Fingerprint_Set_Send_Command_Packet(Fingerprint_DeleteAll, 0, &FP_Tx_Array[0]);
	Fingerprint_Get_Read_Response_Packet(&response, &FP_Parameter, &FP_Rx_Array[0]);
	if(response != Fingerprint_ACK_Resp)
	{
		printf("Err: Negative ACK - code 0x%04x\n\r", response);
		sprintf(lcd_string, "NACK - 0x%04x", response);
		//LCD_write(lcd_string, Fingerprint_Row);
	}
	else if(FP_Parameter != 0)
	{
		printf("Err: Wrong FP_Parameter\n\r");
		//Fingerprint_Err_Para_Msg();
	}
	else
	{
		printf("You wiped out everything :/\n\rBurn in Hell :P\n\r");
		//LCD_write("DB Deleted :(", Fingerprint_Row);
	}
}

void Fingerprint_Add_Fingerprint(void)
{
	Fingerprint_Wait_for_Finger(false);
	Fingerprint_Wait_for_Finger(true);
	printf("Starting Enrollment\n\r");
	//LCD_write("Enroll Start", Fingerprint_Row);

	Fingerprint_Set_Send_Command_Packet(Fingerprint_EnrollStart, Fingerprint_Current_ID, &FP_Tx_Array[0]);
	Fingerprint_Get_Read_Response_Packet(&response, &FP_Parameter, &FP_Rx_Array[0]);
	if(response != Fingerprint_ACK_Resp)
	{
		printf("Err: Negative ACK - code 0x%04x\n\r", response);
		sprintf(lcd_string, "NACK - 0x%04x", response);
		//LCD_write(lcd_string, Fingerprint_Row);
	}
	else if(FP_Parameter != 0)
	{
		printf("Err: Wrong FP_Parameter\n\r");
		//Fingerprint_Err_Para_Msg();
	}
	else
	{
		printf("Enrollment Started with ID %d\n\r", Fingerprint_Current_ID);
		printf("\n\r\n\r***Enrollment Stage 1 Started***\n\r\n\r");
		//LCD_write("Enroll Stage 1", Fingerprint_Row);
		sprintf(lcd_string, "Enrl Start - ID %d", Fingerprint_Current_ID);
		//LCD_write(lcd_string, Fingerprint_Row);
	}

	Fingerprint_Set_Send_Command_Packet(Fingerprint_CaptureFinger, true, &FP_Tx_Array[0]);
	Fingerprint_Get_Read_Response_Packet(&response, &FP_Parameter, &FP_Rx_Array[0]);
	if(response != Fingerprint_ACK_Resp)
	{
		printf("Err: Negative ACK - code 0x%04x\n\r", response);
		sprintf(lcd_string, "NACK - 0x%04x", response);
		//LCD_write(lcd_string, Fingerprint_Row);
	}
	else if(FP_Parameter != 0)
	{
		printf("Err: Wrong FP_Parameter\n\r");
		//Fingerprint_Err_Para_Msg();
	}
	else
	{
		printf("Fingerprint (1) Captured\n\r");
		//LCD_write("FP (1) Captured", Fingerprint_Row);
	}

	Fingerprint_Set_Send_Command_Packet(Fingerprint_Enroll1, 0, &FP_Tx_Array[0]);
	Fingerprint_Get_Read_Response_Packet(&response, &FP_Parameter, &FP_Rx_Array[0]);
	if(response != Fingerprint_ACK_Resp)
	{
		printf("Err: Negative ACK - code 0x%04x\n\r", response);
		sprintf(lcd_string, "NACK - 0x%04x", response);
		//LCD_write(lcd_string, Fingerprint_Row);
	}
	else if(FP_Parameter != 0)
	{
		printf("Err: Wrong FP_Parameter\n\r");
		//Fingerprint_Err_Para_Msg();
	}
	else
	{
		printf("\n\r\n\r***Enrollment Stage 1 Completed***\n\r\n\rLift up finger and place again\n\r");
		//LCD_write("Enrl Stage 1 Done", Fingerprint_Row);
	}

	Fingerprint_Wait_for_Finger(false);
	printf("put the same finger again\n\r");
	//LCD_write("Put Same Finger", Fingerprint_Row);
	Fingerprint_Wait_for_Finger(true);
	printf("\n\r\n\r***Enrollment Stage 2 Started***\n\r\n\r");
	//LCD_write("Enroll Stage 2", Fingerprint_Row);

	Fingerprint_Set_Send_Command_Packet(Fingerprint_CaptureFinger, true, &FP_Tx_Array[0]);
	Fingerprint_Get_Read_Response_Packet(&response, &FP_Parameter, &FP_Rx_Array[0]);
	if(response != Fingerprint_ACK_Resp)
	{
		printf("Err: Negative ACK - code 0x%04x\n\r", response);
		sprintf(lcd_string, "NACK - 0x%04x", response);
		//LCD_write(lcd_string, Fingerprint_Row);
	}
	else if(FP_Parameter != 0)
	{
		printf("Err: Wrong FP_Parameter\n\r");
		//Fingerprint_Err_Para_Msg();
	}
	else
	{
		printf("Fingerprint (2) Captured\n\r");
		//LCD_write("FP (2) Captured", Fingerprint_Row);
	}

	Fingerprint_Set_Send_Command_Packet(Fingerprint_Enroll2, 0, &FP_Tx_Array[0]);
	Fingerprint_Get_Read_Response_Packet(&response, &FP_Parameter, &FP_Rx_Array[0]);
	if(response != Fingerprint_ACK_Resp)
	{
		printf("Err: Negative ACK - code 0x%04x\n\r", response);
		sprintf(lcd_string, "NACK - 0x%04x", response);
		//LCD_write(lcd_string, Fingerprint_Row);
	}
	else if(FP_Parameter != 0)
	{
		printf("Err: Wrong FP_Parameter\n\r");
		//Fingerprint_Err_Para_Msg();
	}
	else
	{
		printf("\n\r\n\r***Enrollment Stage 2 Completed***\n\r\n\rLift up finger and place again\n\r");
		//LCD_write("Enrl Stage 2 Done", Fingerprint_Row);
	}

	Fingerprint_Wait_for_Finger(false);
	printf("put the same finger again\n\r");
	//LCD_write("Put Same Finger", Fingerprint_Row);
	Fingerprint_Wait_for_Finger(true);
	printf("\n\r\n\r***Enrollment Stage 3 Started***\n\r\n\r");
	//LCD_write("Enroll Stage 3", Fingerprint_Row);

	Fingerprint_Set_Send_Command_Packet(Fingerprint_CaptureFinger, true, &FP_Tx_Array[0]);
	Fingerprint_Get_Read_Response_Packet(&response, &FP_Parameter, &FP_Rx_Array[0]);
	if(response != Fingerprint_ACK_Resp)
	{
		printf("Err: Negative ACK - code 0x%04x\n\r", response);
		sprintf(lcd_string, "NACK - 0x%04x", response);
		//LCD_write(lcd_string, Fingerprint_Row);
	}
	else if(FP_Parameter != 0)
	{
		printf("Err: Wrong FP_Parameter\n\r");
		//Fingerprint_Err_Para_Msg();
	}
	else
	{
		printf("Fingerprint (3) Captured\n\r");
		//LCD_write("FP (2) Captured", Fingerprint_Row);
	}

	Fingerprint_Set_Send_Command_Packet(Fingerprint_Enroll3, 0, &FP_Tx_Array[0]);
	Fingerprint_Get_Read_Response_Packet(&response, &FP_Parameter, &FP_Rx_Array[0]);
	if(response != Fingerprint_ACK_Resp)
	{
		printf("Err: Negative ACK - code 0x%04x\n\r", response);
		sprintf(lcd_string, "NACK - 0x%04x", response);
		//LCD_write(lcd_string, Fingerprint_Row);
	}
	else if(FP_Parameter != 0)
	{
		printf("Err: Wrong FP_Parameter\n\r");
		//Fingerprint_Err_Para_Msg();
	}
	else
	{
		printf("\n\r\n\r***Enrollment Stage 3 Completed***\n\r\n\r");
		//LCD_write("Enrl Stage 3 Done", Fingerprint_Row);
	}

	printf("Merging all 3 images and storing in flash\n\rEnrollment Completed\n\r");
	//LCD_write("Merging and Saving", Fingerprint_Row);
	//LCD_write("Enrollment Complete", Fingerprint_Row);

	Fingerprint_Current_ID += 1;
}

void Fingerprint_Check_Authorization(void)
{
	printf("Starting Identification\n\r");
	LCD_write("Searching DB", Fingerprint_Row);

	Fingerprint_Set_Send_Command_Packet(Fingerprint_CaptureFinger, true, &FP_Tx_Array[0]);
	Fingerprint_Get_Read_Response_Packet(&response, &FP_Parameter, &FP_Rx_Array[0]);
	if(response != Fingerprint_ACK_Resp)
	{
		printf("Err: Negative ACK - code 0x%04x\n\r", response);
		sprintf(lcd_string, "NACK - 0x%04x", response);
		//LCD_write(lcd_string, Fingerprint_Row);
	}
	else if(FP_Parameter != 0)
	{
		printf("Err: Wrong FP_Parameter\n\r");
		//Fingerprint_Err_Para_Msg();
	}
	else
	{
		printf("Fingerprint Captured\n\r");
		//LCD_write("Fingerprint Captured", Fingerprint_Row);
	}

	Fingerprint_Set_Send_Command_Packet(Fingerprint_Identify, 0, &FP_Tx_Array[0]);
	Fingerprint_Get_Read_Response_Packet(&response, &FP_Parameter, &FP_Rx_Array[0]);
	if(response != Fingerprint_ACK_Resp)
	{
		printf("Fingerprint Not Found\n\r");
		LCD_write("No Match Found", Fingerprint_Row);
	}
	else
	{
		printf("Fingerprint Found\n\rAuthentication ID - %ld\n\r", FP_Parameter);
		LCD_write("MATCH!!!!", Fingerprint_Row);
		generic_level_client_set(Andrew_Lux_Friend_Server_Address);
		printf("Do you want to add a new Fingerprint?\n\rPress PB0 to Accept or PB1 to Decline\n\r");
		LCD_write("PB1:NO PB0:ADD-NEW", Fingerprint_Row);
		while((GPIO_PinInGet(Button1_port, Button1_pin) != 0) && (GPIO_PinInGet(Button0_port, Button0_pin) != 0));
		FP_Button = 1;
		if(GPIO_PinInGet(Button1_port, Button1_pin) != 0)
		{
			printf("Starting to add new fingerprint\n\r");
			//LCD_write("NEW FP Start", Fingerprint_Row);
			Fingerprint_Add_Fingerprint();
		}
		else
		{
			printf("Pleasure doing business with you!!\n\rHave fun, Door to the Heaven is open :D\n\r");
			LCD_write("Door is Opened", Fingerprint_Row);
		}
	}
//	Fingerprint_Wait_for_Finger(false);
}

