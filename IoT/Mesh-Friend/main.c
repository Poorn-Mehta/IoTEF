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

/*****************************************************************************************************
 * Owner: Poorn Mehta (Poorn.Mehta@Colorado.EDU)
 * Date Last Modified: 12/06/2018
 * Model Being Used: Generic Level Client
 *>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>NODE OVERVIEW<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
 * This node is part of a project which uses Bluetooth mesh network to share important information
 * obtained from variety of sensors, and to act upon it.
 *
 * This node was tested along with 3 other nodes - all using Generic Level Servers
 * Due to limitation of the provisioner made available by Silicon Labs, only one model is used
 * CAUTION - This node uses the standard model in non standard way to achieve full functionality
 * This node works as the central device of the star network being formed (using mesh stack)
 *
 * Following are the nodes and their functionality
 *
 * NODE 1 - Moisture - Low Power <Generic Level Server> | Persistent Data:
 * 			Measures moisture level using a specific sensor. Sets alarm threshold based on the Lux value
 * 			received from another node which uses Lux sensor, and communicates to this <moisture> node
 * 			via central node. When the moisture level crosses the threshold (which is dynamically
 * 			determined using the light level presented), it raises the alarm - which is based on central node.
 *
 * 			-> PUBLISHES 0x7FFF (highest value for int8_t - which is the value type of the model being used)
 * 				to the ALARM GROUP as a mean to convey the message that the moisture alarm is raised
 * 			-> Communicates to Central node through UNICAST messaging, when the central node uses a
 * 				client_set on this node. The central node basically just relays the Lux level (also referred
 * 				to as Threshold level in the present code) - without knowing how to use or interpret that.
 * 				That task is solely upto the Moisture node. Lux node and Moisture nodes should agree and
 * 				establish a method to effectively communicate with each other
 * 			-> PUBLISHES the actual moisture level when the Alarm is OFF.
 *
 * Node 2 - Lux/Door - Friend <Generic Level Server> | Persistent Data:
 * 			Measures the intensity of light and converts it to Lux. Additionally, it (ideally) controls
 * 			a secured door using magnetic lock, which is driven by central node through fingerprint
 * 			sensor implemented on it. Moreoever, it has the capability to send an alarm - either indicating
 * 			that the door was forcefully opened somehow (by implementing special sensor) - or when the light
 * 			is 'too' bright. The functionality can be easily set and determined by the node. However, other
 * 			nodes in network need to know it in order to correctly interpret/use the information provided.
 *
 * 			-> PUBLISHES 0x7FFF to the ALARM GROUP to raise an alarm.
 * 			-> PUBLISHES the scaled Lux level (or threshold level) periodically when the alarm is OFF
 * 			-> Communicates to Central through UNICAST messaging - in both ways. Central node polls
 * 				this node at every fixed interval (5 seconds now) using client_get() to receive the
 * 				Lux/Threshold level values.
 * 			-> Central node sends this node 0x7FFF when it recognizes a fingerprint that was stored in the
 * 				database as the one belonging to a person authorized to open the secure door. This is done
 * 				by central node using client_set() on this Lux/Door node.
 *
 * Node 3 - Smoke - Low Power <Generic Level Server> | Persistent Data:
 * 			Measures the smoke level of the entity that it is configured for, and raises the alarm when
 * 			it crosses the predetermined threshold. Even this LPN's threshold can be updated dynamically,
 * 			similarly to that of the Moisture node, however it wasn't included due to time restrictions.
 * 			Besides Alarm, this node updates the scaled down smoke level value which is read from Central
 * 			node on a button press.
 *
 * 			-> PUBLISHES 0x7FFF to the ALARM GROUP to notify the group about high smoke level
 * 			-> Updates server with scaled smoke level, which is then made available to central node through
 * 				mesh stack. This uses UNICAST messaging since the central node uses client_get()
 *
 * Node 4 - Fingerprint/Central - Friend - <Generic Level Client> | Persistent Data: Alarm Status
 * 			< SENSOR: GT-521F32 LINK: https://www.sparkfun.com/products/14518 >
 * 			Subscribed to the Alarm Group. Sounds a buzzer whenever any Alarm ON is published to the group.
 * 			Displays the combination of nodes which has raised the Alarm before it was manually cleared by
 * 			presseing button 0 (PB0 - Push Button 0). Stores is to the flash - which is useful if the alarm
 * 			was on and the node went through a power cycle. On button 1 press, it gets smoke level from
 * 			Smoke node using UNICAST client get(). It also integrates fingerprint scanner to which, new
 * 			fingerprints can be added by a few means. When a stored/valid fingerprint is pressed, the node
 * 			sends the command of Door Open to Lux/Door node by using client_set() on that node, and with
 * 			the value of 0x7FFF. This node polls Lux/Door node for new scaled Lux/Threshold value. If it
 * 			is different from the last one, then relays it to Moisture node.
 *
 * 			-> SUBSCRIBES to the group. Services the Alarm.
 * 			-> If any node publishes any other value than the ALARM ON (0x7FFF) then treats is uniquely.
 * 				Moisture node's publishes are treated as Mositure Level absolute values, Lux/Door node's
 * 				publishes are treated as scaled Lux level. Smoke node's publishes are treated as scaled
 * 				Smoke Level.
 * 			-> Communicates to all 3 nodes using UNICAST messaging.
 * 				- To/From Smoke - Gets smoke level on PB1 press (client_get()).
 * 				- To/From Lux/Door - Gets lux level periodically (client_get()). Sets Door state to Open (client_set()).
 * 				- To/From Moisture - Sets threshold level (a.k.a. scaled Lux level) if it is different from last received
 * 										lux level.
 *
 *****************************************************************************************************/


#include "src/gpio.h"
#include "src/main.h"
#include "src/mesh_friend.h"
#include "src/mesh_init.h"
#include "src/Custom_Timer0.h"
#include "src/fingerprint.h"


// Flag for indicating DFU Reset must be performed
uint8_t boot_to_dfu = 0;

#define Event_Button0_Pressed		1
#define Event_Button1_Pressed		2


static void handle_gecko_event(uint32_t evt_id, struct gecko_cmd_packet *evt);

bool mesh_bgapi_listener(struct gecko_cmd_packet *evt);

/** global variables */
uint16_t Element_Index = 0xffff;
uint16_t Appkey_Index = 0;
uint8_t Trans_ID = 0;
uint8_t conn_handle = 0xFF;      /* handle of the last opened LE connection */

char lcd_string[20];

static uint16 _my_address = 0;    /* Address of the Primary Element of the Node */
static uint8 num_connections = 0;     /* number of active Bluetooth connections */

uint16_t Andrew_Timeout = 0;
uint16_t Response;

volatile bool button0_read = false;
volatile bool button1_read = false;

uint16_t Publisher_Data, Publisher_Address;
uint8_t Alarm_Status = 0;
uint8_t Old_Alarm_Status = 0;

//uint8_t write;
uint16_t Moisture_Level;
uint16_t New_Threshold = 0;
uint16_t Old_Threshold = 0;
uint16_t Smoke_Level;


uint32_t Interrupt_Read;
uint8_t buzz_st = 0;

uint8_t FP_Button = 0;

uint8_t cntr = 0;
char print[3];

//Timer0 IRQ for Sounding the Alarm
void TIMER0_IRQHandler(void)	// Timer ISR, will execute 80ms after TIMER0 is enabled
{
	CORE_AtomicDisableIrq();	// No nested interrupts, atomic operation
	Interrupt_Read = TIMER_IntGetEnabled(TIMER0);	// Reading interrupt flag register
	TIMER_IntClear(TIMER0, 0x0FFF);		// Clearing all interrupts in register
	if(Interrupt_Read == TIMER_IF_UF)	// Checking for the occurrence of specific interrupt
	{
		//Toggling GPIO pin
		if(buzz_st)
		{
			GPIO_PinOutSet(Alarm_port, Alarm_pin);
			buzz_st = 0;
		}
		else
		{
			GPIO_PinOutClear(Alarm_port, Alarm_pin);
			buzz_st = 1;
		}
	}
	CORE_AtomicEnableIrq();		// End of ISR
}

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

//Functions for button press
void Button0_Function(void)	//This button gets smoke level from smoke LPN
{
	if(FP_Button == 1)	FP_Button = 0;
	else
	{
		printf("Getting Smoke\n\r");
		sprintf(lcd_string, "Get Smoke");
		LCD_write(lcd_string, Mesh_Data);

		//API call to get the smoke level
		generic_level_client_get(Khalid_Smoke_LPN_Server_Address);
	}
}

void Button1_Function(void) //This button clears the alarm state back to 0
{
	if(FP_Button == 1)	FP_Button = 0;
	else
	{
		printf("Clearing Alarms\n\r");
		LCD_write("Alarm - Clear", Alarm_Row);
		Alarm_Status = 0;
		Old_Alarm_Status = 0;
		sprintf(lcd_string, "alarm - %d",Alarm_Status);
		LCD_write(lcd_string, Alarm_Value);

		//Stopping the toggling of GPIO
		Custom_Timer0_Stop();

		//Store the new alarm state to the flash
		Store_Alarm_to_Flash();
	}

}

/*
 * Information about node is here
 * this is my primary node (central one for the project)
 */


//#pragma GCC push_options
//#pragma GCC optimize ("O0")

int main()
{
  Mesh_Init();

  //Configuring GPIO pins
  gpio_init();

  //Configuring timer0
  Custom_Timer0_Init();

  //Configuring Debug monitor output
  RETARGET_SerialInit();

  //Configuring LCD
  LCD_init("");

  //Setting up fingerprint sensor
  Fingerprint_Init(); //p30 - orange, p32 - yellow
  Fingerprint_Setup(true);

  //Loading Alarm State using persistent memory
  Load_Alarm_from_Flash();

  while (1)
  {
	  //Default code from empty mesh project
    struct gecko_cmd_packet *evt = gecko_wait_event();
    bool pass = mesh_bgapi_listener(evt);
    if(pass)
    {
      handle_gecko_event(BGLIB_MSG_ID(evt->header), evt);
    }
  }
}

//#pragma GCC pop_options

static void handle_gecko_event(uint32_t evt_id, struct gecko_cmd_packet *evt)
{

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
        //Check pushbutton state at startup. If either PB0 or PB1 is held down then do factory reset
        if((GPIO_PinInGet(Button1_port, Button1_pin) == 0) || (GPIO_PinInGet(Button0_port, Button0_pin) == 0))
        {
          initiate_factory_reset();
        }
        else
        {
        	//Get the device address and set the name of the node
			struct gecko_msg_system_get_bt_address_rsp_t *pAddr = gecko_cmd_system_get_bt_address();
			set_device_name(&pAddr->address);

			//Intializing mesh stack with required out of band provisioning

			//Output authentication method, numeric value, size of 2 digits
			Response = gecko_cmd_mesh_node_init_oob(OOB_Public_Key, OOB_Output_Auth_Method,
					OOB_Output_Numeric, OOB_Size, OOB_Ignore, OOB_Ignore, OOB_Ignore)->result;
			if(Response != 0)
			{
				printf("OOB Failed\n\r");
				sprintf(lcd_string, "Error code: 0x%x\n\r", Response);
				LCD_write(lcd_string, Mesh_Status);
				printf("%s\n\r", lcd_string);
			}
		}
		break;
    }

    case gecko_evt_hardware_soft_timer_id:
    {
    	//Handling software timers
        switch(evt->data.evt_hardware_soft_timer.handle)
        {
          //Factory reset
          case TIMER_ID_FACTORY_RESET:
          {
        	  //Query for clearing up the fingerprint sensor database
				printf("Do you want to NUKE the whole Database?\n\rPress PB0 to Accept or PB1 to Decline\n\r");
				LCD_write("PB1:NO PB0:WIPE DB", Fingerprint_Row);
				while((GPIO_PinInGet(Button1_port, Button1_pin) != 0) && (GPIO_PinInGet(Button0_port, Button0_pin) != 0));
				if(GPIO_PinInGet(Button1_port, Button1_pin) != 0)
				{
					printf("Bombs Away\n\r");

					//Function to delete fingerprint database
					Fingerprint_Delete_Everything();
				}

				//Resetting board
				gecko_cmd_system_reset(0);
				break;
          }

          //Enabling Interrupts
          case TIMER_Button_Debouncing:
        	  NVIC_EnableIRQ(GPIO_ODD_IRQn);
        	  NVIC_EnableIRQ(GPIO_EVEN_IRQn);
        	  break;

          //Restart
          case TIMER_ID_RESTART:
            gecko_cmd_system_reset(0);
            break;

          //Provisioning is completed
          case	TIMER_Provisioning:
        	  printf("\n\r\n\r*************NORMAL OPERATION START*************\n\r\n\r");

        	  //Enabling Interrupts
        	  NVIC_EnableIRQ(GPIO_ODD_IRQn);
        	  NVIC_EnableIRQ(GPIO_EVEN_IRQn);

        	  //Starting regular polling of fingerprint sensor
        	  gecko_cmd_hardware_set_soft_timer(32768, TIMER_Regular_Poll, 0); //32768 = 1s
        	  break;

          //Regular poll timeout
          case TIMER_Regular_Poll:

        	  //Increment count (to poll that node every 5 second)
        	  Andrew_Timeout +=1;
        	  if(Andrew_Timeout >= 5)
        	  {
        		  //API to get level value from Lux node
        		  generic_level_client_get(Andrew_Lux_Friend_Server_Address);
        		  Andrew_Timeout = 0;
        		  LCD_write("", Fingerprint_Row);
        	  }

        	  //If authorization hasn't been called then check for the user to put the fingerprint
        	  //Else check for the user to lift the finger
        	  if(Fingerprint_State == Authorization_not_Checked)
        	  {
            	  if(Fingerprint_Check_for_Finger(true) == 0)
    			  {
            		  printf("Finger Detected\n\r");
            		  Fingerprint_Check_Authorization();
            		  Fingerprint_State = Authorization_Completed;
    			  }
        	  }
        	  else if(Fingerprint_State == Authorization_Completed)
        	  {
        		  if(Fingerprint_Check_for_Finger(false) != 0)	Fingerprint_State = Authorization_not_Checked;
        	  }
          default:
        	  break;
        }
        break;
    }

    //Handling button interrupts along with debouncing
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

    //Node initialization
    case gecko_evt_mesh_node_initialized_id:
    {
        printf("node initialized\r\n");

        //One of the most important API calls - initializes client
        Response = gecko_cmd_mesh_generic_client_init()->result;
        if(Response)
        {
        	printf("Client Init Failed\n\r");
        	printf("Error code: %x\n\r", Response);
        }

        //Checking whether already provisioned or not
        struct gecko_msg_mesh_node_initialized_evt_t *pData = (struct gecko_msg_mesh_node_initialized_evt_t *)&(evt->data);
        if(pData->provisioned)
        {
          printf("node is provisioned. address:%x, ivi:%ld\r\n", pData->address, pData->ivi);
          _my_address = pData->address;
          Element_Index = 0;   // index of primary element is zero. This example has only one element.
          friend_node_init();	//Initializing mesh library and friend feature
          sprintf(lcd_string, "Provisioned - %d", pData->address);
          LCD_write(lcd_string, Provision_Status);
    	  NVIC_EnableIRQ(GPIO_ODD_IRQn);
    	  NVIC_EnableIRQ(GPIO_EVEN_IRQn);
    	  gecko_cmd_hardware_set_soft_timer(32768, TIMER_Regular_Poll, 0);
        }
        else
        {
          printf("node is unprovisioned\r\n");
          LCD_write("unprovisioned", Provision_Status);
          printf("starting unprovisioned beaconing...\r\n");
//          gecko_cmd_mesh_node_start_unprov_beaconing(0x3);   // enable ADV and GATT provisioning bearer
          gecko_cmd_mesh_node_start_unprov_beaconing(0x2);   // enable GATT provisioning bearer
        }
        break;
    }

    case gecko_evt_mesh_node_provisioning_started_id:
    {
        printf("Started provisioning\r\n");
        LCD_write("Provisioning...", Provision_Status);
        break;
    }

    //Event which displays (and generates) the random number which is to be entered on provisioner
    case gecko_evt_mesh_node_display_output_oob_id:
    {
		printf("OOB Display Key Event\n\r");
		char OOB_Passkey_String[3];
		uint8_t tmp = evt->data.evt_mesh_node_display_output_oob.data.len;
		uint8_t OOB_Passkey = evt->data.evt_mesh_node_display_output_oob.data.data[tmp-1];
		sprintf(OOB_Passkey_String, "%d", OOB_Passkey);
		LCD_write("Prov. passkey:", Mesh_Status);
		LCD_write(OOB_Passkey_String, Passkey_Value);
		printf("OOB Passkey: %d\n\r", OOB_Passkey);
		break;
    }


    case gecko_evt_mesh_node_provisioned_id:
    {
        Element_Index = 0;   // index of primary element is zero. This example has only one element.
        friend_node_init();
        printf("node provisioned, got address=%x\r\n", evt->data.evt_mesh_node_provisioned.address);
        sprintf(lcd_string, "Provisioned - %d", evt->data.evt_mesh_node_provisioned.address);
        LCD_write(lcd_string, Provision_Status);
		LCD_write("", Mesh_Status);
		LCD_write("", Passkey_Value);
        break;
    }

    case gecko_evt_mesh_node_provisioning_failed_id:
    {
        prov_fail_evt = (struct gecko_msg_mesh_node_provisioning_failed_evt_t  *)&(evt->data);
        printf("provisioning failed, code %x\r\n", prov_fail_evt->result);
        LCD_write("prov failed", Provision_Status);
        /* start a one-shot timer that will trigger soft reset after small delay */
        gecko_cmd_hardware_set_soft_timer(2 * 32768, TIMER_ID_RESTART, 1);
        break;
    }

    case gecko_evt_mesh_node_key_added_id:
    {
        printf("got new %s key with index %x\r\n", evt->data.evt_mesh_node_key_added.type == 0 ? "network" : "application",
               evt->data.evt_mesh_node_key_added.index);
        //Storing application key index for use in unicast client initiated communications
        if(evt->data.evt_mesh_node_key_added.type == Application_Key_Type)
      	  Appkey_Index = evt->data.evt_mesh_node_key_added.index;
        break;
    }

    //Empirically it was found out that this event is the one which take place
    //After the provisioner has finished each and every setting. So after a fixed delay, starting regular functionality.
    case gecko_evt_mesh_node_model_config_changed_id:
    {
        printf("model config changed\r\n");
        gecko_cmd_hardware_set_soft_timer((32768 * Prov_Timeout_Seconds), TIMER_Provisioning, 1);
        break;
    }


    case gecko_evt_le_connection_opened_id:
      printf("evt:gecko_evt_le_connection_opened_id\r\n");
      num_connections++;
      conn_handle = evt->data.evt_le_connection_opened.connection;
      break;

    case gecko_evt_le_connection_parameters_id:
      printf("evt:gecko_evt_le_connection_parameters_id\r\n");
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
          //LCD_write("", 3);// TODO: DI_ROW_CONNECTION
        }
      }
      break;

    case gecko_evt_mesh_friend_friendship_established_id:
    {
    	uint16_t tmp_addr;
    	tmp_addr = evt->data.evt_mesh_friend_friendship_established.lpn_address;
      printf("evt gecko_evt_mesh_friend_friendship_established, lpn_address=%x\r\n", tmp_addr);
      sprintf(lcd_string, "FRIEND - %d", tmp_addr);
      LCD_write(lcd_string, Friendship_Status);
      break;
    }

    case gecko_evt_mesh_friend_friendship_terminated_id:
      printf("evt gecko_evt_mesh_friend_friendship_terminated, reason=%x\r\n", evt->data.evt_mesh_friend_friendship_terminated.reason);
      LCD_write("NO LPN", Friendship_Status);
      break;

    //This event is generated whenever there is a published message to the group to which the model is subscribed to
    //Or when the server responds to Client_Get()
    case gecko_evt_mesh_generic_client_server_status_id:
    {
    	//Saving original threshold value (Lux level) to compare it with the new one
    	//If they're different, a unicast client_set will be sent to Moisture LPN
    	Old_Threshold = New_Threshold;

    	//Reading and storing publisher address and data
    	Publisher_Address = evt->data.evt_mesh_generic_client_server_status.server_address;
    	Publisher_Data = ((evt->data.evt_mesh_generic_client_server_status.parameters.data[1])<<8)\
    			| (evt->data.evt_mesh_generic_client_server_status.parameters.data[0]);

    	//Execute alarm update function if that's what was published/returned in client_get
    	if(Publisher_Data == Alarm_ON)	Alarm_Update();

    	//Otherwise treat values separately
    	//One from three - Moisture Level, Lux/Threshold Level, Smoke Level
    	else
    	{
    		switch(Publisher_Address)
    		{
    			case John_Moisture_LPN_Server_Address:
    				Moisture_Level = Publisher_Data;
    				printf("Moisture Level is %d\n\r", Moisture_Level);
    				sprintf(lcd_string, "Moist - %d",Moisture_Level);
    				LCD_write(lcd_string, Moisture_Row);
    				break;
    			case Andrew_Lux_Friend_Server_Address:
    				New_Threshold = Publisher_Data;
    				if(New_Threshold != Old_Threshold)	generic_level_client_set(John_Moisture_LPN_Server_Address);
    				printf("New Threshold is %d\n\r", New_Threshold);
    				sprintf(lcd_string, "Lux - %d", New_Threshold);
    				LCD_write(lcd_string, Threshold_Row);
    				break;
    			case Khalid_Smoke_LPN_Server_Address:
    				Smoke_Level = Publisher_Data;
    				printf("Smoke Level is %d\n\r", Smoke_Level);
    				sprintf(lcd_string, "Smoke - %d", Smoke_Level);
    				LCD_write(lcd_string, Smoke_Row);
    				break;
    			default:
    				break;
    		}
    	}
    	sprintf(lcd_string, "P:0x%04x D:0x%04x", Publisher_Address, Publisher_Data);
    	LCD_write(lcd_string, Mesh_Data);
    	break;
    }

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
