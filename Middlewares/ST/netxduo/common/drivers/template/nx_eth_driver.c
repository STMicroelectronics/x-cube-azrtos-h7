/**************************************************************************/
/*                                                                        */
/*       Copyright (c) Microsoft Corporation. All rights reserved.        */
/*                                                                        */
/*       This software is licensed under the Microsoft Software License   */
/*       Terms for Microsoft Azure RTOS. Full text of the license can be  */
/*       found in the LICENSE file at https://aka.ms/AzureRTOS_EULA       */
/*       and in the root directory of this software.                      */
/*                                                                        */
/**************************************************************************/


/* Include driver specific include file.  */
#include "nx_eth_driver.h"
/*Includes -----------------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/*Defines -----------------------------------------------------------------*/
/* USER CODE BEGIN Defines */

/* USER CODE END Defines */

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */
VOID  nx_eth_driver(NX_IP_DRIVER *driver_req_ptr)
{
  /* USER CODE BEGIN INIT */
  
  /* USER CODE END INIT */
  
    /* Process according to the driver request type in the IP control
       block.  */
  switch (driver_req_ptr -> nx_ip_driver_command)
  {

  case NX_LINK_INTERFACE_ATTACH:
  /* USER CODE BEGIN INIT */
  
  /* USER CODE END INIT */
      break;

  case NX_LINK_INITIALIZE:
  /* USER CODE BEGIN NX_LINK_INITIALIZE */
  
  /* USER CODE END NX_LINK_INITIALIZE */
      break;

  case NX_LINK_ENABLE:
  /* USER CODE BEGIN NX_LINK_ENABLE */
  
  /* USER CODE END NX_LINK_ENABLE */
      break;

  case NX_LINK_DISABLE:
  /* USER CODE BEGIN NX_LINK_DISABLE */
  
  /* USER CODE END NX_LINK_DISABLE */
      break;

  case NX_LINK_ARP_SEND:
  /* USER CODE BEGIN NX_LINK_ARP_SEND */
  
  /* USER CODE END NX_LINK_ARP_SEND */
      break;

  case NX_LINK_ARP_RESPONSE_SEND:
  /* USER CODE BEGIN NX_LINK_ARP_RESPONSE_SEND */
  
  /* USER CODE END NX_LINK_ARP_RESPONSE_SEND */
      break;

  case NX_LINK_PACKET_BROADCAST:
  /* USER CODE BEGIN NX_LINK_PACKET_BROADCAST */
  
  /* USER CODE END NX_LINK_PACKET_BROADCAST */
      break;

  case NX_LINK_RARP_SEND:
  /* USER CODE BEGIN NX_LINK_RARP_SEND */
  
  /* USER CODE END NX_LINK_RARP_SEND */
      break;

  case NX_LINK_PACKET_SEND:
  /* USER CODE BEGIN NX_LINK_PACKET_SEND */
  
  /* USER CODE END NX_LINK_PACKET_SEND */
      break;

  case NX_LINK_MULTICAST_JOIN:
  /* USER CODE BEGIN NX_LINK_MULTICAST_JOIN */
  
  /* USER CODE END NX_LINK_MULTICAST_JOIN */
      break;

  case NX_LINK_MULTICAST_LEAVE:
  /* USER CODE BEGIN NX_LINK_MULTICAST_LEAVE */
  
  /* USER CODE END NX_LINK_MULTICAST_LEAVE */
      break;

  case NX_LINK_GET_STATUS:
  /* USER CODE BEGIN NX_LINK_GET_STATUS */
  
  /* USER CODE END NX_LINK_GET_STATUS */
      break;

  case NX_LINK_DEFERRED_PROCESSING:
  /* USER CODE BEGIN NX_LINK_DEFERRED_PROCESSING */
  
  /* USER CODE END NX_LINK_DEFERRED_PROCESSING */
      break;


#ifdef NX_ENABLE_INTERFACE_CAPABILITY
  case NX_INTERFACE_CAPABILITY_GET:
  /* USER CODE BEGIN NX_INTERFACE_CAPABILITY_GET */
  
  /* USER CODE END NX_INTERFACE_CAPABILITY_GET */
      break;

  case NX_INTERFACE_CAPABILITY_SET:
  /* USER CODE BEGIN NX_INTERFACE_CAPABILITY_SET */
  
  /* USER CODE END NX_INTERFACE_CAPABILITY_SET */
      break;
#endif /* NX_ENABLE_INTERFACE_CAPABILITY */

  default:
  /* USER CODE BEGIN DEFAULT */
  
  /* USER CODE END DEFAULT */
      driver_req_ptr->nx_ip_driver_status =  NX_UNHANDLED_COMMAND;
      driver_req_ptr->nx_ip_driver_status =  NX_DRIVER_ERROR;
  }
}

  /* USER CODE BEGIN 1 */
  
  /* USER CODE END 1 */

