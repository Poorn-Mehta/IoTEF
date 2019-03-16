/*
 * mesh_friend.c
 *
 *  Created on: Dec 3, 2018
 *      Author: poorn
 */

#include "src/mesh_friend.h"

uint16_t Element_Index, Appkey_Index, Response;
uint8_t Trans_ID, conn_handle;

uint16_t New_Threshold, Old_Threshold, Publisher_Data, Publisher_Address;
uint8_t Alarm_Status, Old_Alarm_Status;

char lcd_string[20];

void generic_level_client_get(uint16_t s_addr)
{
	uint8_t local_counter;
	uint32_t dly;
	for(local_counter = 0; local_counter < 3; local_counter ++)
	{
		errorcode_t	resp;
		resp = mesh_lib_generic_client_get(
			  	MESH_GENERIC_LEVEL_CLIENT_MODEL_ID,
				Element_Index,
				s_addr,
				Appkey_Index,
				Generic_Level_State_Type
			  );


		  if (resp) {
			  sprintf(lcd_string, "Mesh Error:%d",resp);
			  LCD_write(lcd_string, Mesh_Status);
			printf("gecko_cmd_mesh_generic_client_get failed,code %d\r\n", resp);
		  } else
		  {
			printf("\n\rClient Get used on address %d\n\r\r\n", s_addr);
			sprintf(lcd_string, "Client Get to:0x%x",s_addr);
			LCD_write(lcd_string, Mesh_Status);
		  }
		  for(dly = 0; dly < 100000; dly ++);
	}
}

void generic_level_client_set(uint16_t s_addr)
{
	errorcode_t	resp;
	Trans_ID += 1;
	struct mesh_generic_request	custom;
	custom.kind = mesh_generic_request_level;
	if(s_addr == John_Moisture_LPN_Server_Address)	custom.level = New_Threshold;
	else if(s_addr == Andrew_Lux_Friend_Server_Address)		custom.level = Door_Open;
	resp = mesh_lib_generic_client_set(
			MESH_GENERIC_LEVEL_CLIENT_MODEL_ID,
			Element_Index,
			s_addr,
			Appkey_Index,
			Trans_ID,
			&custom,
			false,
			false,
			true); //resp required

	  if (resp) {
		  sprintf(lcd_string, "Mesh Error:%d",resp);
		  LCD_write(lcd_string, Mesh_Status);
		printf("gecko_cmd_mesh_generic_client_publish failed,code %d\r\n", resp);
	  }
	  else
	  {
			printf("\n\rClient Set used on address %d\n\r\r\n", s_addr);
			sprintf(lcd_string, "Client Set to:0x%x",s_addr);
			LCD_write(lcd_string, Mesh_Status);
	  }
}

void Alarm_Info_Update(void)
{
	switch(Alarm_Status)
	{
		case 0:
			LCD_write("Alarm - Clear", Alarm_Row);
			break;
		case (1 << John_Alarm):
				LCD_write("Alarm - MT", Alarm_Row);
				break;
		case (1 << Andrew_Alarm):
				LCD_write("Alarm - LX", Alarm_Row);
				break;
		case (1 << Khalid_Alarm):
				LCD_write("Alarm - SK", Alarm_Row);
				break;
		case ((1 << John_Alarm) | (1 << Andrew_Alarm)):
				LCD_write("Alarm - MT & LX", Alarm_Row);
				break;
		case ((1 << John_Alarm) | (1 << Khalid_Alarm)):
				LCD_write("Alarm - MT & SK", Alarm_Row);
				break;
		case ((1 << Andrew_Alarm) | (1 << Khalid_Alarm)):
				LCD_write("Alarm - LX & SK", Alarm_Row);
				break;
		case ((1 << John_Alarm) | (1 << Andrew_Alarm) | (1 << Khalid_Alarm)):
				LCD_write("Alarm - ALL", Alarm_Row);
				break;
		default:
			break;
	}
}

void Store_Alarm_to_Flash(void)
{
		sprintf(lcd_string, "alarm - %d",Alarm_Status);
		LCD_write(lcd_string, Alarm_Value);
		Response = gecko_cmd_flash_ps_save(Flash_Key_Address, 1, &Alarm_Status)->result;
		if(Response)
		{
		  printf("\n\rFlash Writing Error - %x",Response);
		  sprintf(lcd_string, "f-w_err - %x",Response);
		  LCD_write(lcd_string, Flash_Data);
		}
		else
		{
		  sprintf(lcd_string, "flash_w - %x",Alarm_Status);
		  LCD_write(lcd_string, Flash_Data);
		}
}

void Load_Alarm_from_Flash(void)
{
    struct gecko_msg_flash_ps_load_rsp_t* Flash_Resp;
    Flash_Resp = gecko_cmd_flash_ps_load(Flash_Key_Address);
    Alarm_Status = Flash_Resp->value.data[0];
    Response = Flash_Resp->result;
	  if(Response)
	  {
		  if(Response == Key_Not_Written)
		  {
			  Alarm_Status = 0;
			  Store_Alarm_to_Flash();
			  Load_Alarm_from_Flash();
		  }
		  else
		  {
			  printf("\n\rFlash Reading Error - %x",Response);
			  sprintf(lcd_string, "f-r_err - %x",Response);
			  LCD_write(lcd_string, Flash_Data);
		  }
	  }
	  else
	  {
			sprintf(lcd_string, "alarm - %d",Alarm_Status);
			LCD_write(lcd_string, Alarm_Value);
			Alarm_Info_Update();
			if(Alarm_Status) 	Custom_Timer0_Start();
			sprintf(lcd_string, "flash_r - %d",Alarm_Status);
			LCD_write(lcd_string, Flash_Data);
	  }
}

void Alarm_Update(void)
{
	Old_Alarm_Status = Alarm_Status;
	switch(Publisher_Address)
	{
		case John_Moisture_LPN_Server_Address:
			Alarm_Status |= (1 << John_Alarm);
			Custom_Timer0_Start();
			break;
		case Andrew_Lux_Friend_Server_Address:
			Alarm_Status |= (1 << Andrew_Alarm);
			Custom_Timer0_Start();
			break;
		case Khalid_Smoke_LPN_Server_Address:
			Alarm_Status |= (1 << Khalid_Alarm);
			Custom_Timer0_Start();
			break;
		default:
			break;
	}

	if(Alarm_Status != Old_Alarm_Status)	Store_Alarm_to_Flash();

	Alarm_Info_Update();

}

/**
 * Set device name in the GATT database. A unique name is generated using
 * the two last bytes from the Bluetooth address of this device. Name is also
 * displayed on the LCD.
 */
void set_device_name(bd_addr *pAddr)
{
  // create unique device name using the last two bytes of the Bluetooth address
  sprintf(lcd_string, "Poorn's Node %x:%x", pAddr->addr[1], pAddr->addr[0]);

  printf("Device name: '%s'\r\n", lcd_string);

  Response = gecko_cmd_gatt_server_write_attribute_value(gattdb_device_name, 0, strlen(lcd_string), (uint8 *)lcd_string)->result;
  if (Response) {
    printf("gecko_cmd_gatt_server_write_attribute_value() failed, code %x\r\n", Response);
  }

  // show device name on the LCD
  LCD_write(lcd_string, Name_Row);
//  LCD_write("", 5);
}

/**
 *  this function is called to initiate factory reset. Factory reset may be initiated
 *  by keeping one of the WSTK pushbuttons pressed during reboot. Factory reset is also
 *  performed if it is requested by the provisioner (event gecko_evt_mesh_node_reset_id)
 */
void initiate_factory_reset(void)
{
  printf("factory reset\r\n");
  LCD_write("\n***\nFACTORY RESET\n***", Mesh_Status);

  /* if connection is open then close it before rebooting */
  if (conn_handle != 0xFF) {
    gecko_cmd_le_connection_close(conn_handle);
  }

  /* perform a factory reset by erasing PS storage. This removes all the keys and other settings
     that have been configured for this node */
  gecko_cmd_flash_ps_erase_all();
  gecko_cmd_hardware_set_soft_timer(2 * 32768, TIMER_ID_FACTORY_RESET, 1);
}


void friend_node_init(void)
{
	 /* Initialize mesh lib */
	 mesh_lib_init(malloc, free, 8);

	 //Initialize Friend functionality
	 printf("Friend mode initialization\r\n");
	 Response = gecko_cmd_mesh_friend_init()->result;
	 if (Response) printf("Friend init failed 0x%x\r\n", Response);
}

//void Soft_Timer_Handler(void)
//{
//
//}
