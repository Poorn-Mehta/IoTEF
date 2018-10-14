/***********************************************************************************************//**
 * \file   main.c
 * \brief  Silicon Labs Thermometer Client Example
 *
 * This example implements a BLE client that will scan for Thermometer Examples.
 * By default it will handle up to four connections and print them on the UART.
 * It will use CR characters to update the values in a single line, so the terminal
 * program should be set up to not use CR as newline.
 ***************************************************************************************************
 * <b> (C) Copyright 2018 Silicon Labs, http://www.silabs.com</b>
 ***************************************************************************************************
 * This file is licensed under the Silicon Labs License Agreement. See the file
 * "Silabs_License_Agreement.txt" for details. Before using this software for
 * any purpose, you must agree to the terms of that agreement.
 **************************************************************************************************/

//***********************************************************************************
// Include files
//***********************************************************************************
#include "src/Custom_BLE_Client.h"

//***********************************************************************************
// Functions
//***********************************************************************************

void Client_Address(void)
{
	struct gecko_msg_system_get_bt_address_rsp_t* Client_Addr;
	char Client_Address_Print[18];
	Client_Addr = gecko_cmd_system_get_bt_address();
	snprintf(Client_Address_Print, sizeof(Client_Address_Print), "%0.2X:%0.2X:%0.2X:%0.2X:%0.2X:%0.2X",
			Client_Addr->address.addr[5], Client_Addr->address.addr[4], Client_Addr->address.addr[3],
			Client_Addr->address.addr[2], Client_Addr->address.addr[1], Client_Addr->address.addr[0]);
	LCD_write("BT ADDR", LCD_ROW_BTADDR1);
	LCD_write(Client_Address_Print, LCD_ROW_BTADDR2);
}

void Custom_BLE_Client_Delete_Bondings(void)
{
    Dev_State = disconnected;
    LCD_write("Delete Bondings?", LCD_ROW_PASSKEY);
    LCD_write("PB0-Yes PB1-No", LCD_ROW_ACTION);
    valid_button_press = true;
    SLEEP_Sleep();
    if(button0_read == true)
    {
    	gecko_cmd_sm_delete_bondings();
    }
    button0_read = false;
    button1_read = false;
}

void Custom_BLE_Client_Check_for_Target_Server(void)
{
	bd_addr Remote_Address = evt->data.evt_le_gap_scan_response.address;
	char Remote_Address_String[18];
	snprintf(Remote_Address_String, sizeof(Remote_Address_String),
			"%0.2X:%0.2X:%0.2X:%0.2X:%0.2X:%0.2X",
			Remote_Address.addr[5], Remote_Address.addr[4], Remote_Address.addr[3],
			Remote_Address.addr[2], Remote_Address.addr[1], Remote_Address.addr[0]);
	LCD_write(Remote_Address_String, LCD_ROW_CLIENTADDR);
	if(evt->data.evt_le_gap_scan_response.packet_type == 0)
	{
		if(memcmp(&evt->data.evt_le_gap_scan_response.address.addr[0],
				&Server_Addr.addr[0], Server_Address_Length) == 0)
		{
			Dev_State = target_server_found;
			LCD_write("Server Found", LCD_ROW_ACTION);
			Custom_BLE_Client_Connect_to_Server(Server_Addr);
		}
	}
}

void Print_Remote_Address(void)
{
	bd_addr Remote_Address = evt->data.evt_le_connection_opened.address;
	char Remote_Address_String[18];
	snprintf(Remote_Address_String, sizeof(Remote_Address_String),
			"%0.2X:%0.2X:%0.2X:%0.2X:%0.2X:%0.2X",
			Remote_Address.addr[5], Remote_Address.addr[4], Remote_Address.addr[3],
			Remote_Address.addr[2], Remote_Address.addr[1], Remote_Address.addr[0]);
	LCD_write(Remote_Address_String, LCD_ROW_CLIENTADDR);
}

void Custom_BLE_Client_Confirm_Passkey(void)
{
	uint32_t key;
	key = evt->data.evt_sm_confirm_passkey.passkey; // reading passkey
	char Passkey_String[7];
	snprintf(Passkey_String, sizeof(Passkey_String), "%0.6ld", key);	// printing the passkey
	LCD_write(Passkey_String, LCD_ROW_PASSKEY);
	LCD_write("PB0-Accept PB1-Reject", LCD_ROW_ACTION);
	valid_button_press = 1;
}

void Custom_BLE_Client_Connect_to_Server(bd_addr Target_Address)
{
	gecko_cmd_le_gap_end_procedure();
	struct gecko_msg_le_gap_connect_rsp_t* Response =
			gecko_cmd_le_gap_connect(Target_Address, le_gap_address_type_public, le_gap_phy_1m);
	if(Response->result == 0)
	{
		Dev_State = connected;
		LCD_write("Connected", LCD_ROW_ACTION);
	}
	else
	{
		LCD_write("Connection Failed", LCD_ROW_PASSKEY);
		LCD_write("PB0-Retry PB1-Stop", LCD_ROW_ACTION);
		valid_button_press = 1;
	}
}

void Custom_BLE_Client_Discover_Primary_Services_by_UUID(uint8_t length, const uint8_t *uuid)
{
	struct gecko_msg_gatt_discover_primary_services_by_uuid_rsp_t*	Response;
	Response = gecko_cmd_gatt_discover_primary_services_by_uuid(
			Connection_Handle, length,  uuid);
	if(Response->result != 0)
	{
		LCD_write("Error-Service Search", LCD_ROW_PASSKEY);
		Custom_BLE_Client_Close_Connection();
	}
	else
	{
		LCD_write("Service Search", LCD_ROW_ACTION);
		Dev_State = primary_services_search;
	}
}

void Custom_BLE_Client_Discover_Characteristics_by_UUID(uint32_t service_handle, uint8_t length, const uint8_t *uuid)
{
	struct gecko_msg_gatt_discover_characteristics_by_uuid_rsp_t* Response;
	Response = gecko_cmd_gatt_discover_characteristics_by_uuid(Connection_Handle, service_handle,
			length, uuid);
	if(Response->result != 0)
	{
		LCD_write("Error-Char Search", LCD_ROW_PASSKEY);
		Custom_BLE_Client_Close_Connection();
	}
	else
	{
		Dev_State = characteristics_search;
	}
}

void Custom_BLE_Client_Set_Indications(uint16_t characteristic_handle)
{
	struct gecko_msg_gatt_set_characteristic_notification_rsp_t* Response;
	Response = gecko_cmd_gatt_set_characteristic_notification(Connection_Handle,
			characteristic_handle, gatt_indication);
	if(Response->result != 0)
	{
		LCD_write("Error-Indication", LCD_ROW_PASSKEY);
		char hex[5];
		snprintf(hex, 5, "%x", Response->result);
		LCD_write(hex, LCD_ROW_PASSKEY);
	}
	else
	{
		LCD_write("Indication Set", LCD_ROW_PASSKEY);
		Dev_State = indications_enabled;
	}
}

void Custom_BLE_Client_Look_for_Service(const uint8_t *uuid, uint8_t length)
{
	if(memcmp(&evt->data.evt_gatt_service.uuid.data[0],	uuid, length) == 0)
	{
		LCD_write("Service Found", LCD_ROW_ACTION);
		Service_Handle = evt->data.evt_gatt_service.service;
		Dev_State = health_thermometer_service_found;
	}
	else
	{
		LCD_write("No Thermometer", LCD_ROW_PASSKEY);
	}
}

void Custom_BLE_Client_Look_for_Characteristic(const uint8_t *uuid, uint8_t length)
{
	if(memcmp(&evt->data.evt_gatt_characteristic.uuid.data[0], uuid, length) == 0)
	{
		LCD_write("Characteristic Found", LCD_ROW_ACTION);
		Characteristic_Handle = evt->data.evt_gatt_characteristic.characteristic;
		Dev_State = temperature_characterstic_found;
	}
	else
	{
		LCD_write("No Temperature", LCD_ROW_PASSKEY);
	}
}

void Custom_BLE_Client_Close_Connection(void)
{
	LCD_write("Closing Connection", LCD_ROW_ACTION);
	gecko_cmd_le_connection_close(Connection_Handle);
	LCD_write(" ", LCD_ROW_ACTION);
	char hex[5];
	snprintf(hex, 5, "%x", Dev_State);
	LCD_write(hex, LCD_ROW_PASSKEY);
}

void Custom_BLE_Client_Get_Temperature_Send_Confirmation(void)
{
	if((evt->data.evt_gatt_characteristic_value.characteristic == Characteristic_Handle)
			&& (evt->data.evt_gatt_characteristic_value.att_opcode == gatt_handle_value_indication))
	{
//		LCD_write("Current Temperature", LCD_ROW_ACTION);
		uint8_t Temperature_Array[Temperature_Byte_Stream_Length];
		memcpy(Temperature_Array, &evt->data.evt_gatt_characteristic_value.value.data[0], Temperature_Byte_Stream_Length);
		int32_t Temperature_Value;
		Temperature_Value = (Temperature_Array[1] + (Temperature_Array[2] << 8) + (Temperature_Array[3] << 16));
		char Temperature_Print[Temperature_Print_Array_Length];
		snprintf(Temperature_Print, Temperature_Print_Array_Length, "Temp: %0.2ld.%0.3luC",
				(Temperature_Value / 1000), (Temperature_Value % 1000));
		LCD_write(Temperature_Print, LCD_ROW_TEMPVALUE);
		struct gecko_msg_gatt_send_characteristic_confirmation_rsp_t* Response;
		Response = gecko_cmd_gatt_send_characteristic_confirmation(Connection_Handle);
		if(Response->result != 0)
		{
			LCD_write("Indication Failed", LCD_ROW_PASSKEY);
		}
		else
		{
			LCD_write("Indication Sent", LCD_ROW_PASSKEY);
		}
	}
	else
	{
		LCD_write("Incorrect Value", LCD_ROW_PASSKEY);
		LCD_write("Received", LCD_ROW_ACTION);
	}
}
