/*
 * mesh_init.c
 *
 *  Created on: Nov 24, 2018
 *      Author: poorn
 */

#include "src/mesh_init.h"

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
const struct bg_gattdb_def bg_gattdb_data;

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


void Mesh_Init(void)
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
	  gecko_bgapi_class_mesh_generic_client_init();
	  //gecko_bgapi_class_mesh_generic_server_init();
	  //gecko_bgapi_class_mesh_vendor_model_init();
	  //gecko_bgapi_class_mesh_health_client_init();
	  //gecko_bgapi_class_mesh_health_server_init();
	  //gecko_bgapi_class_mesh_test_init();
	  //gecko_bgapi_class_mesh_lpn_init();
	  gecko_bgapi_class_mesh_friend_init();
}


