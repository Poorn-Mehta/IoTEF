/***********************************************************************************************//**
 * \file   main.c
 * \brief  Silicon Labs BT Mesh Empty Example Project
 *
 * This example demonstrates the bare minimum needed for a Blue Gecko BT Mesh C application.
 * The application starts unprovisioned Beaconing after boot
 ***************************************************************************************************
 * <b> (C) Copyright 2017 Silicon Labs, http://www.silabs.com</b>
 ***************************************************************************************************
 * This file is licensed under the Silabs License Agreement. See the file
 * "Silabs_License_Agreement.txt" for details. Before using this software for
 * any purpose, you must agree to the terms of that agreement.
 **************************************************************************************************/

/* C Standard Library headers */
#include <stdlib.h>
#include <stdio.h>

/* Board headers */
#include "init_mcu.h"
#include "init_board.h"
#include "init_app.h"
#include "ble-configuration.h"
#include "board_features.h"
#include "retargetserial.h"

/* Bluetooth stack headers */
#include "bg_types.h"
#include "native_gecko.h"
#include "gatt_db.h"
#include <gecko_configuration.h>
#include "mesh_generic_model_capi_types.h"
#include "mesh_lib.h"
#include <mesh_sizes.h>

/* Libraries containing default Gecko configuration values */
#include "em_emu.h"
#include "em_cmu.h"
#include "em_core.h"

/* Device initialization header */
#include "hal-config.h"

/* Display Interface header */
#include "lcd_driver.h"

#include "src/gpio.h"					 
#if defined(HAL_CONFIG)
#include "bsphalconfig.h"
#else
#include "bspconfig.h"
#endif

/***********************************************************************************************//**
 * @addtogroup Application
 * @{
 **************************************************************************************************/

/***********************************************************************************************//**
 * @addtogroup app
 * @{
 **************************************************************************************************/

// bluetooth stack heap
#define MAX_CONNECTIONS 2

uint8_t bluetooth_stack_heap[DEFAULT_BLUETOOTH_HEAP(MAX_CONNECTIONS) + BTMESH_HEAP_SIZE + 1760];

// Bluetooth advertisement set configuration
//
// At minimum the following is required:
// * One advertisement set for Bluetooth LE stack (handle number 0)
// * One advertisement set for Mesh data (handle number 1)
// * One advertisement set for Mesh unprovisioned beacons (handle number 2)
// * One advertisement set for Mesh unprovisioned URI (handle number 3)
// * N advertisement sets for Mesh GATT service advertisements
// (one for each network key, handle numbers 4 .. N+3)
//
#define MAX_ADVERTISERS (4 + MESH_CFG_MAX_NETKEYS)

// bluetooth stack configuration
extern const struct bg_gattdb_def bg_gattdb_data;

// Flag for indicating DFU Reset must be performed
uint8_t boot_to_dfu = 0;

const gecko_configuration_t config =
{
  .bluetooth.max_connections = MAX_CONNECTIONS,
  .bluetooth.max_advertisers = MAX_ADVERTISERS,
  .bluetooth.heap = bluetooth_stack_heap,
  .bluetooth.heap_size = sizeof(bluetooth_stack_heap) - BTMESH_HEAP_SIZE,
  .bluetooth.sleep_clock_accuracy = 100,
  .gattdb = &bg_gattdb_data,
  .btmesh_heap_size = BTMESH_HEAP_SIZE,
#if (HAL_PA_ENABLE) && defined(FEATURE_PA_HIGH_POWER)
  .pa.config_enable = 1, // Enable high power PA
  .pa.input = GECKO_RADIO_PA_INPUT_VBAT, // Configure PA input to VBAT
#endif // (HAL_PA_ENABLE) && defined(FEATURE_PA_HIGH_POWER)
  .max_timers = 16,
};

/** Timer Frequency used. */
#define TIMER_CLK_FREQ ((uint32)32768)
/** Convert msec to timer ticks. */
#define TIMER_MS_2_TIMERTICK(ms) ((TIMER_CLK_FREQ * ms) / 1000)

#define TIMER_ID_RESTART    78
#define TIMER_ID_FACTORY_RESET  77
#define TIMER_ID_PROVISIONING   66
#define TIMER_ID_FRIEND_FIND 20

#define TIMER_Button_Debouncing		11

#define Event_Button0_Pressed		1
#define Event_Button1_Pressed		2							   					
static void handle_gecko_event(uint32_t evt_id, struct gecko_cmd_packet *evt);
void mesh_native_bgapi_init(void);
bool mesh_bgapi_listener(struct gecko_cmd_packet *evt);

/** global variables */
static uint16 _elem_index = 0xffff; /* For indexing elements of the node (this example has only one element) */
static uint16 _my_address = 0;    /* Address of the Primary Element of the Node */
static uint8 num_connections = 0;     /* number of active Bluetooth connections */
static uint8 conn_handle = 0xFF;      /* handle of the last opened LE connection */

volatile bool button0_read = false;
volatile bool button1_read = false;
volatile uint16_t temp_appkey_index = 0;

uint32_t Interrupt_Read;

uint8_t cntr = 0;
char print[3];

//PB0 ISR
void GPIO_EVEN_IRQHandler(void)
{
	//basic interrupt handling flow
	CORE_AtomicDisableIrq();
	Interrupt_Read = GPIO_IntGet();
	GPIO_IntClear(Interrupt_Read);
	if(Interrupt_Read == Button0_Interrupt_Mask)
	{
		button0_read = true;
	}
	//interrupts are handled using external signal event
	gecko_external_signal(Event_Button0_Pressed);

	//used instead of timer for debouncing
	NVIC_DisableIRQ(GPIO_EVEN_IRQn);

	CORE_AtomicEnableIrq();
}

//same as above, except it is for PB1
void GPIO_ODD_IRQHandler(void)
{
	CORE_AtomicDisableIrq();
	Interrupt_Read = GPIO_IntGet();
	GPIO_IntClear(Interrupt_Read);
	if(Interrupt_Read == Button1_Interrupt_Mask)
	{
		button1_read = true;
	}
	gecko_external_signal(Event_Button1_Pressed);
	NVIC_DisableIRQ(GPIO_ODD_IRQn);
	CORE_AtomicEnableIrq();
}

//void init_model(void)	// TODO: complete this
//{
//	  mesh_lib_generic_server_register_handler(MESH_GENERIC_LEVEL_SERVER_MODEL_ID,
//	                                           0,
//	                                           pri_level_request,
//	                                           pri_level_change);
//}

// alright now here

void generic_level_update_publish(void)
{
	  struct mesh_generic_state custom_data;
	  errorcode_t response;

	  custom_data.kind = mesh_generic_state_level;
	  custom_data.level.level = cntr;


	  response = mesh_lib_generic_server_update(MESH_GENERIC_LEVEL_SERVER_MODEL_ID,
			  	  	  	  	  	  	  	  	_elem_index,
	                                        &custom_data,
	                                        0,
	                                        0);

	  if(response)
	  {
		  char temp_lcd[10];
		  snprintf(temp_lcd, 10, "err (mlib): %d",response);
		  LCD_write(temp_lcd, 5);
	  }
	  else
	  {
		  char temp_lcd[10];
		  snprintf(temp_lcd, 10, "updt: %d",cntr);
		  LCD_write(temp_lcd, 5);
		  response = mesh_lib_generic_server_publish(MESH_GENERIC_LEVEL_SERVER_MODEL_ID,
												_elem_index,
												mesh_generic_state_level);
		  if(response)
		  {
			  char temp_lcd[10];
			  snprintf(temp_lcd, 10, "err (mlib): %d",response);
			  LCD_write(temp_lcd, 5);
		  }
		  else
		  {
			  char temp_lcd[10];
			  snprintf(temp_lcd, 10, "publ: %d",cntr);
			  LCD_write(temp_lcd, 5);
		  }
	  }
}

//Functions for button press
void Button0_Function(void)
{
	if(cntr < 0xFF)		cntr += 1;
	sprintf(print, "%d", cntr);
	LCD_write(print, 3);
	generic_level_update_publish();
}

void Button1_Function(void)
{
	if(cntr > 0)		cntr -= 1;
	sprintf(print, "%d", cntr);
	LCD_write(print, 3);
	generic_level_update_publish();
}
/**
 * Set device name in the GATT database. A unique name is generated using
 * the two last bytes from the Bluetooth address of this device. Name is also
 * displayed on the LCD.
 */
void set_device_name(bd_addr *pAddr)
{
  char name[20];
  uint16 res;

  // create unique device name using the last two bytes of the Bluetooth address
  sprintf(name, "Mesh LPN %x:%x", pAddr->addr[1], pAddr->addr[0]);

  printf("Device name: '%s'\r\n", name);

  res = gecko_cmd_gatt_server_write_attribute_value(gattdb_device_name, 0, strlen(name), (uint8 *)name)->result;
  if (res) {
    printf("gecko_cmd_gatt_server_write_attribute_value() failed, code %x\r\n", res);
  }

  // show device name on the LCD
  LCD_write(name, DI_ROW_NAME);
  LCD_write("", 5);			   
}

/**
 *  this function is called to initiate factory reset. Factory reset may be initiated
 *  by keeping one of the WSTK pushbuttons pressed during reboot. Factory reset is also
 *  performed if it is requested by the provisioner (event gecko_evt_mesh_node_reset_id)
 */
void initiate_factory_reset(void)
{
  printf("factory reset\r\n");
  LCD_write("\n***\nFACTORY RESET\n***", DI_ROW_STATUS);

  /* if connection is open then close it before rebooting */
  if (conn_handle != 0xFF) {
    gecko_cmd_le_connection_close(conn_handle);
  }

  /* perform a factory reset by erasing PS storage. This removes all the keys and other settings
     that have been configured for this node */
  gecko_cmd_flash_ps_erase_all();
  gecko_cmd_hardware_set_soft_timer(2 * 32768, TIMER_ID_FACTORY_RESET, 1);
}


/**
 * Initialize LPN functionality with configuration and friendship establishment.
 */
void lpn_init(void)
{
  uint16 res;
  // Initialize LPN functionality.
  res = gecko_cmd_mesh_lpn_init()->result;
  if (res) {
    printf("LPN init failed (0x%x)\r\n", res);
    return;
  }

  res = gecko_cmd_mesh_lpn_configure(2, 5 * 1000)->result;
  if (res) {
    printf("LPN conf failed (0x%x)\r\n", res);
    return;
  }

  printf("trying to find friend...\r\n");
  res = gecko_cmd_mesh_lpn_establish_friendship(0)->result;

  if (res != 0) {
    printf("ret.code %x\r\n", res);
  }
}

/*
 * Information about node is here
 * this is a low power node just for testing
 * it is having generic level server
 */
void low_power_node_init(void)
{
	 /* Initialize mesh lib */
	 mesh_lib_init(malloc, free, 8);
	 lpn_init();
//	 init_model(); //TODO: complete this
}







int main(void)
{
  // Initialize device
  initMcu();
  // Initialize board
  initBoard();
  // Initialize application
  initApp();

  gecko_stack_init(&config);
  gecko_bgapi_class_dfu_init();
  gecko_bgapi_class_system_init();
  gecko_bgapi_class_le_gap_init();
  gecko_bgapi_class_le_connection_init();
  gecko_bgapi_class_gatt_init();
  gecko_bgapi_class_gatt_server_init();
  gecko_bgapi_class_endpoint_init();
  gecko_bgapi_class_hardware_init();
  gecko_bgapi_class_flash_init();
  gecko_bgapi_class_test_init();
  gecko_bgapi_class_sm_init();
  mesh_native_bgapi_init();
  gecko_initCoexHAL();

  gecko_bgapi_class_mesh_node_init();
  //gecko_bgapi_class_mesh_prov_init();
  gecko_bgapi_class_mesh_proxy_init();
  gecko_bgapi_class_mesh_proxy_server_init();
  //gecko_bgapi_class_mesh_proxy_client_init();
  //gecko_bgapi_class_mesh_generic_client_init();
  gecko_bgapi_class_mesh_generic_server_init();
  //gecko_bgapi_class_mesh_vendor_model_init();
  //gecko_bgapi_class_mesh_health_client_init();
  //gecko_bgapi_class_mesh_health_server_init();
  //gecko_bgapi_class_mesh_test_init();
  gecko_bgapi_class_mesh_lpn_init();
  //gecko_bgapi_class_mesh_friend_init();

  gpio_init();

  RETARGET_SerialInit();
  LCD_init("");

  while (1) {
    struct gecko_cmd_packet *evt = gecko_wait_event();
    bool pass = mesh_bgapi_listener(evt);
    if (pass) {
      handle_gecko_event(BGLIB_MSG_ID(evt->header), evt);
    }
  }
}

static void handle_gecko_event(uint32_t evt_id, struct gecko_cmd_packet *evt)
{
  uint16_t result;
  char buf[30];

  struct gecko_msg_mesh_node_provisioning_failed_evt_t  *prov_fail_evt;

  if (NULL == evt) {
	return;
  }

  switch (evt_id) {
    case gecko_evt_dfu_boot_id:
      //gecko_cmd_le_gap_set_advertising_timing(0, 1000*adv_interval_ms/625, 1000*adv_interval_ms/625, 0, 0);
      gecko_cmd_le_gap_set_mode(2, 2);
      break;

    case gecko_evt_system_boot_id:
    {
        // check pushbutton state at startup. If either PB0 or PB1 is held down then do factory reset
        if (GPIO_PinInGet(BSP_BUTTON0_PORT, BSP_BUTTON0_PIN) == 0 || GPIO_PinInGet(BSP_BUTTON1_PORT, BSP_BUTTON1_PIN) == 0) {
          initiate_factory_reset(); // TODO: change this pin in get things
        } else {
          struct gecko_msg_system_get_bt_address_rsp_t *pAddr = gecko_cmd_system_get_bt_address();

          set_device_name(&pAddr->address);

          // Initialize Mesh stack in Node operation mode, wait for initialized event
//          result = gecko_cmd_mesh_node_init()->result;
          if (result) {
            sprintf(buf, "init failed (0x%x)", result);
            LCD_write(buf, DI_ROW_STATUS);
          }
          result = gecko_cmd_mesh_node_init_oob(0, 0x08, 0x08, 0x02, 0, 0, 0)->result;

          			if(result != 0)
          			{
          				printf("OOB Failed\n\r");
          				printf("Error code: 0x%x\n\r", result);
          			}
        }
      break;
    }

    case gecko_evt_hardware_soft_timer_id:
    {
        switch(evt->data.evt_hardware_soft_timer.handle)
        {
          case TIMER_ID_FACTORY_RESET:
            gecko_cmd_system_reset(0);
            break;
          case TIMER_Button_Debouncing:
        	  NVIC_EnableIRQ(GPIO_ODD_IRQn);
        	  NVIC_EnableIRQ(GPIO_EVEN_IRQn);
        	  break;
          case TIMER_ID_RESTART:
            gecko_cmd_system_reset(0);
            break;
          case TIMER_ID_FRIEND_FIND:
          {
            printf("trying to find friend...\r\n");
            result = gecko_cmd_mesh_lpn_establish_friendship(0)->result;

            if (result != 0) {
              printf("ret.code %x\r\n", result);
            }
            break;
          }
          default:
        	  break;
        }
        break;
    }

    case gecko_evt_system_external_signal_id:
    {
		uint32_t Event_Read = evt->data.evt_system_external_signal.extsignals;
		switch(Event_Read)
		{
			case Event_Button0_Pressed:
				Button0_Function();
				gecko_cmd_hardware_set_soft_timer(3276, TIMER_Button_Debouncing, 1);
				break;
			case Event_Button1_Pressed:
				Button1_Function();
				gecko_cmd_hardware_set_soft_timer(3276, TIMER_Button_Debouncing, 1);
				break;
			default:
				break;
		}
    	break;
    } 
    case gecko_evt_mesh_node_initialized_id:
    {
        printf("node initialized\r\n");
        result = gecko_cmd_mesh_generic_server_init()->result;
        if(result)
        {
        	printf("Server Init Failed\n\r");
        	printf("Error code: %x\n\r", result);
        }

        struct gecko_msg_mesh_node_initialized_evt_t *pData = (struct gecko_msg_mesh_node_initialized_evt_t *)&(evt->data);

        if (pData->provisioned) {
          printf("node is provisioned. address:%x, ivi:%ld\r\n", pData->address, pData->ivi);

          _my_address = pData->address;
          _elem_index = 0;   // index of primary element is zero. This example has only one element.

      //    gpio_init(); // TODO: enable interrupts
          low_power_node_init();

          LCD_write("provisioned", DI_ROW_STATUS);
        } else {
          printf("node is unprovisioned\r\n");
          LCD_write("unprovisioned", DI_ROW_STATUS);

          printf("starting unprovisioned beaconing...\r\n");
//          gecko_cmd_mesh_node_start_unprov_beaconing(0x3);   // enable ADV and GATT provisioning bearer
          gecko_cmd_mesh_node_start_unprov_beaconing(0x2);   // enable GATT provisioning bearer
        }
        break;
    }

    case gecko_evt_mesh_node_provisioning_started_id:
    {
        printf("Started provisioning\r\n");
        LCD_write("provisioning...", DI_ROW_STATUS);
        break;
    }

    case gecko_evt_mesh_node_display_output_oob_id:
    		printf("**** EVENT: gecko_evt_mesh_node_display_output_oob_id: *****\n\r");

//    		struct gecko_msg_mesh_node_display_output_oob_evt_t *Pass = (struct gecko_msg_mesh_node_display_output_oob_evt_t *)&(evt->data);

    		/* Stores passkey in a string */
    		char OOB_Passkey_String[3];
//    		uint8_t OOB_Passkey = Pass->data.data[Pass->data.len-1];

//    		uint16_t OOB_Passkey = 0;
    		uint8_t tmp = evt->data.evt_mesh_node_display_output_oob.data.len;
    		uint8_t OOB_Passkey = evt->data.evt_mesh_node_display_output_oob.data.data[tmp-1];

//    		__itoa(OOB_Passkey, OOB_Passkey_String, 10);
    		sprintf(OOB_Passkey_String, "%d", OOB_Passkey);

    		/* Displays passkey on LCD and terminal */
    		LCD_write("Prov. passkey:", 3);
    		LCD_write(OOB_Passkey_String, 4);
    		printf("Pass: %d\n\r", OOB_Passkey);

    //Debugging
    //		printf("output_action: %d\n", Pass->output_action);
    //		printf("output_size: %d\n", Pass->output_size);

    		break;


    case gecko_evt_mesh_node_provisioned_id:
    {
        _elem_index = 0;   // index of primary element is zero. This example has only one element.
        low_power_node_init();
        printf("node provisioned, got address=%x\r\n", evt->data.evt_mesh_node_provisioned.address);
        LCD_write("provisioned", DI_ROW_STATUS);
		LCD_write("", 3);
		LCD_write("", 4);
		//    gpio_init(); // TODO: enable interrupts
        break;
    }

    case gecko_evt_mesh_node_provisioning_failed_id:
    {
        prov_fail_evt = (struct gecko_msg_mesh_node_provisioning_failed_evt_t  *)&(evt->data);
        printf("provisioning failed, code %x\r\n", prov_fail_evt->result);
        LCD_write("prov failed", DI_ROW_STATUS);
        /* start a one-shot timer that will trigger soft reset after small delay */
        gecko_cmd_hardware_set_soft_timer(2 * 32768, TIMER_ID_RESTART, 1);
        break;
    }



    case gecko_evt_mesh_node_model_config_changed_id:
      printf("model config changed\r\n");
      break;

      // come
    case gecko_evt_mesh_generic_server_client_request_id:
      printf("evt gecko_evt_mesh_generic_server_client_request_id\r\n");
//      mesh_lib_generic_server_event_handler(evt);

  	uint8_t var1;
  	uint16_t var2;
  	var2 = 0;
  	var1 = 0;
  	var1 = evt->data.evt_mesh_generic_server_client_request.type;
  	var2 = ((evt->data.evt_mesh_generic_server_client_request.parameters.data[1])<<8)\
  			| (evt->data.evt_mesh_generic_server_client_request.parameters.data[0]);
//    	var2 += 1;
//    	var2 *= 100;
//    	var2 /= 65535;
  	char temp_print[15];
  	snprintf(temp_print, 15, "v1:%xv2:%d", var1, var2);
  	LCD_write(temp_print, 6);
  	break;

      break;

    case gecko_evt_mesh_generic_server_state_changed_id:

      // uncomment following line to get debug prints for each server state changed event
      //server_state_changed(&(evt->data.evt_mesh_generic_server_state_changed));

      // pass the server state changed event to mesh lib handler that will invoke
      // the callback functions registered by application
      mesh_lib_generic_server_event_handler(evt);
      break;

    case gecko_evt_mesh_node_key_added_id:
      printf("got new %s key with index %x\r\n", evt->data.evt_mesh_node_key_added.type == 0 ? "network" : "application",
             evt->data.evt_mesh_node_key_added.index);
      if(evt->data.evt_mesh_node_key_added.type == 1)
    	  temp_appkey_index = evt->data.evt_mesh_node_key_added.index;
      break;

    case gecko_evt_le_connection_opened_id:
      printf("evt:gecko_evt_le_connection_opened_id\r\n");
      num_connections++;
      conn_handle = evt->data.evt_le_connection_opened.connection;
      LCD_write("connected", 3);// TODO: DI_ROW_CONNECTION
      // turn off lpn feature after GATT connection is opened
      gecko_cmd_mesh_lpn_deinit();
      LCD_write("LPN off", 4);// TODO: DI_ROW_LPN
      break;

    case gecko_evt_le_connection_closed_id:
      /* Check if need to boot to dfu mode */
      if (boot_to_dfu) {
        /* Enter to DFU OTA mode */
        gecko_cmd_system_reset(2);
      }
      printf("evt:conn closed, reason 0x%x\r\n", evt->data.evt_le_connection_closed.reason);
      conn_handle = 0xFF;
      if (num_connections > 0) {
        if (--num_connections == 0) {
          LCD_write("", 3);// TODO: DI_ROW_CONNECTION

          lpn_init();
        }
      }
      break;

    case gecko_evt_mesh_lpn_friendship_established_id:
      printf("friendship established\r\n");
      LCD_write("LPN", 4);// TODO: DI_ROW_LPN
      break;

    case gecko_evt_mesh_lpn_friendship_failed_id:
      printf("friendship failed\r\n");
      LCD_write("no friend", 4);// TODO: DI_ROW_LPN
      // try again in 2 seconds
      result  = gecko_cmd_hardware_set_soft_timer(TIMER_MS_2_TIMERTICK(2000), TIMER_ID_FRIEND_FIND, 1)->result;
      if (result) {
        printf("timer failure?!  %x\r\n", result);
      }
      break;

    case gecko_evt_mesh_lpn_friendship_terminated_id:
      printf("friendship terminated\r\n");
      LCD_write("friend lost", 4);// TODO: DI_ROW_LPN
      if (num_connections == 0) {
        // try again in 2 seconds
        result  = gecko_cmd_hardware_set_soft_timer(TIMER_MS_2_TIMERTICK(2000), TIMER_ID_FRIEND_FIND, 1)->result;
        if (result) {
          printf("timer failure?!  %x\r\n", result);
        }
      }
      break;

    case gecko_evt_mesh_node_reset_id:
      printf("evt gecko_evt_mesh_node_reset_id\r\n");
      initiate_factory_reset();
      break;

    case gecko_evt_gatt_server_user_write_request_id:
      if (evt->data.evt_gatt_server_user_write_request.characteristic == gattdb_ota_control) {
        /* Set flag to enter to OTA mode */
        boot_to_dfu = 1;
        /* Send response to Write Request */
        gecko_cmd_gatt_server_send_user_write_response(
          evt->data.evt_gatt_server_user_write_request.connection,
          gattdb_ota_control,
          bg_err_success);

        /* Close connection to enter to DFU OTA mode */
        gecko_cmd_le_connection_close(evt->data.evt_gatt_server_user_write_request.connection);
      }
      break;
    default:
      break;
  }
}
