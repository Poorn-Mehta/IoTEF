#ifndef		__Custom_BLE_Client_h__
#define 	__Custom_BLE_Client_h__

//***********************************************************************************
// Include files
//***********************************************************************************
#include "src/main.h"
#include "src/gpio.h"

//***********************************************************************************
// defined files
//***********************************************************************************

#define Conn_Int_Min_Val	60	// 60*1.25 = 75ms
#define Conn_Int_Max_Val	60
#define Slave_Lat_Val	3		// Skipping 3, connection event on 4th - 4*75 = 300ms
#define Timeout_Val		10000


#define Scan_Interval                 16   //10ms
#define Scan_Window                   16   //10ms
#define Scan_Passive                 0

#define Health_Thermometer_Service_Length	2
#define Temperature_Measurement_Characteristic_Length 2

#define Tx_Power_Service_Length		2
#define Tx_Power_Characteristic_Length	2

#define Health_Thermometer_ID	1
#define	Tx_Power_ID		2

#define Number_of_Primary_Services	2

#define Temperature_Byte_Stream_Length	5
#define Temperature_Print_Array_Length	14

#define Server_Address_Length	6

//***********************************************************************************
// global variables
//***********************************************************************************
//also test array as a pointer and vice versa in this code while using memcmp
//typedef struct{
//	const uint8_t Service_UUID[];
//	uint8_t	Service_Length;
//	uint32_t Service_Handle;
//}Primary_Services;
//
//typedef struct{
//	const uint8_t Characteristic_UUID[];
//	uint8_t	Characteristic_Length;
//	uint32_t Characteristic_Handle;
//}Characteristics;
//
//typedef struct{
//
//};

typedef enum {
  disconnected,
  scanning,
  target_server_found,
  connected,
  paired,
  bonded,
  primary_services_search,
  health_thermometer_service_found,
  characteristics_search,
  temperature_characterstic_found,
  indications_enabled,
  setup_success,
  data_read_ready
} Global_Device_States;

extern Global_Device_States Dev_State;
extern const uint8_t Health_Thermometer[2];


//***********************************************************************************
// function prototypes
//***********************************************************************************

void Client_Address(void);
void Custom_BLE_Client_Delete_Bondings(void);
void Custom_BLE_Client_Check_for_Target_Server(void);
void Print_Remote_Address(void);
void Custom_BLE_Client_Confirm_Passkey(void);
void Custom_BLE_Client_Connect_to_Server(bd_addr Target_Address);
void Custom_BLE_Client_Discover_Primary_Services_by_UUID(uint8_t length, const uint8_t *uuid);
void Custom_BLE_Client_Discover_Characteristics_by_UUID(uint32_t service_handle, uint8_t length, const uint8_t *uuid);
void Custom_BLE_Client_Set_Indications(uint16_t characteristic_handle);
void Custom_BLE_Client_Look_for_Service(const uint8_t *uuid, uint8_t length);
void Custom_BLE_Client_Look_for_Characteristic(const uint8_t *uuid, uint8_t length);
void Custom_BLE_Client_Close_Connection(void);
void Custom_BLE_Client_Get_Temperature_Send_Confirmation(void);

#endif
