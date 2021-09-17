/**
  ******************************************************************************
  * @file    app_threadx.c
  * @author  MCD Application Team
  * @brief   ThreadX applicative file
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2020-2021 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/

/* Private includes ----------------------------------------------------------*/
#include "app_threadx.h"

/* Private typedef -----------------------------------------------------------*/
typedef enum {
PROCESSING_NOT_STARTED    = 99,
WRITING_TO_READWRITE      = 88,
WRITING_TO_READONLY       = 77,
READING_FROM_READWRITE    = 66,
READING_FROM_READONLY     = 55,
PROCESSING_FINISHED       = 44
} ProgressState;

/* Private define ------------------------------------------------------------*/
#define DEFAULT_STACK_SIZE         1024
#define MODULE_DATA_SIZE           32*1024
#define OBJECT_MEM_SIZE            16*1024

#define READONLY_REGION            0x24040000
#define READWRITE_REGION           0x24040100
#define SHARED_MEM_SIZE            0xFF

#define MODULE_FLASH_ADDRESS       0x08020000


/* Private macro -------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/

/* Define the ThreadX object control blocks */
TX_THREAD               ModuleManager;
TXM_MODULE_INSTANCE     my_module;
TX_QUEUE                ResidentQueue;

/* Define the module data pool area. */
ALIGN_32BYTES (UCHAR  module_data_area[MODULE_DATA_SIZE]);

/* Define the object pool area.  */
ALIGN_32BYTES (UCHAR  object_memory[OBJECT_MEM_SIZE]);

/* Define the count of memory faults.  */
ULONG                   memory_faults = 0;

static UCHAR tx_byte_pool_buffer[TX_APP_MEM_POOL_SIZE];
static TX_BYTE_POOL ModuleManagerBytePool;

/* Private function prototypes -----------------------------------------------*/
void Error_Handler(void);
VOID pretty_msg(char *p_msg, ULONG r_msg);
VOID module_manager_entry(ULONG thread_input);
VOID module_fault_handler(TX_THREAD *thread, TXM_MODULE_INSTANCE *module);

/**
  * @brief  Application ThreadX Initialization.
  * @param memory_ptr: memory pointer
  * @retval none
  */
VOID tx_application_define(VOID *first_unused_memory)
{
  CHAR *pointer;

  if (tx_byte_pool_create(&ModuleManagerBytePool, "Module Manager Byte Pool", tx_byte_pool_buffer, TX_APP_MEM_POOL_SIZE) != TX_SUCCESS)
  {
    /* USER CODE BEGIN TX_Byte_Pool_Error */
    Error_Handler();
    /* USER CODE END TX_Byte_Pool_Error */
  }
  else
  {

    /* Allocate the stack for Module Manager Thread.  */
    if (tx_byte_allocate(&ModuleManagerBytePool, (VOID **) &pointer,
                         DEFAULT_STACK_SIZE, TX_NO_WAIT) != TX_SUCCESS)
    {
      Error_Handler();
    }

    /* Create Module Manager Thread.  */
    if (tx_thread_create(&ModuleManager, "Module Manager Thread", module_manager_entry, 0,
                         pointer, DEFAULT_STACK_SIZE,
                         MODULE_MANAGER_THREAD_PRIO, MODULE_MANAGER_THREAD_PREEMPTION_THRESHOLD,
                         TX_NO_TIME_SLICE, TX_AUTO_START) != TX_SUCCESS)
    {
      Error_Handler();
    }

    /* Allocate the stack for resident_queue.  */
    if (tx_byte_allocate(&ModuleManagerBytePool, (VOID **) &pointer,
                         16 * sizeof(ULONG), TX_NO_WAIT) != TX_SUCCESS)
    {
      Error_Handler();
    }

    /* Create the resident_queue */
    if (tx_queue_create(&ResidentQueue, "Resident Queue", TX_1_ULONG,
                        pointer, 16 * sizeof(ULONG)) != TX_SUCCESS)
    {
      Error_Handler();
    }
  }
}

 /**
   * @brief  MX_AZURE_RTOS_Init
   * @param  None
   * @retval None
   */
void MX_AZURE_RTOS_Init(void)
{
  /* USER CODE BEGIN  Before_Kernel_Start */

  /* USER CODE END  Before_Kernel_Start */

  tx_kernel_enter();

  /* USER CODE BEGIN  Kernel_Start_Error */

  /* USER CODE END  Kernel_Start_Error */
}

/**
  * @brief  Module Manager main thread.
  * @param  thread_input: thread id
  * @retval none
  */
VOID module_manager_entry(ULONG thread_input)
{
  UINT   status;
  CHAR   p_msg[64];
  ULONG  r_msg = PROCESSING_NOT_STARTED;
  ULONG  module_properties;

  /* Initialize the module manager. */
  status = txm_module_manager_initialize((VOID *) module_data_area, MODULE_DATA_SIZE);

  if(status != TX_SUCCESS)
  {
    Error_Handler();
  }

  /* Create a pool for module objects. */
  status = txm_module_manager_object_pool_create(object_memory, OBJECT_MEM_SIZE);

  if(status != TX_SUCCESS)
  {
    Error_Handler();
  }

  /* Register a fault handler. */
  status = txm_module_manager_memory_fault_notify(module_fault_handler);

  if(status != TX_SUCCESS)
  {
    Error_Handler();
  }

  /* Load the module from the specified address */
  status = txm_module_manager_in_place_load(&my_module, "my module", (VOID *) MODULE_FLASH_ADDRESS);

  if(status != TX_SUCCESS)
  {
    Error_Handler();
  }

  /* Enable shared memory region for module with read-only access permission. */
  status = txm_module_manager_external_memory_enable(&my_module, (void*)READONLY_REGION, SHARED_MEM_SIZE, 0);

  if(status != TX_SUCCESS)
  {
    Error_Handler();
  }

  /* Enable shared memory region for module with read and write access permission. */
  status = txm_module_manager_external_memory_enable(&my_module, (void*)READWRITE_REGION, SHARED_MEM_SIZE, TXM_MODULE_MANAGER_SHARED_ATTRIBUTE_WRITE);

  if(status != TX_SUCCESS)
  {
    Error_Handler();
  }

  /* Get module properties. */
  status = txm_module_manager_properties_get(&my_module, &module_properties);

  if(status != TX_SUCCESS)
  {
    Error_Handler();
  }

  /* Print loaded module info */
  printf("Module <%s> is loaded from address 0x%08X\n", my_module.txm_module_instance_name, MODULE_FLASH_ADDRESS);
  printf("Module code section size: %i bytes, data section size: %i\n", (int)my_module.txm_module_instance_code_size, (int)my_module.txm_module_instance_data_size);
  printf("Module Attributes:\n");
  printf("  - Compiled for %s compiler\n", ((module_properties >> 25) == 1)? "CubeIDE (GNU)" : ((module_properties >> 24) == 1)? "ARM KEIL" : "IAR EW");
  printf("  - Shared/external memory access is %s\n", ((module_properties & 0x04) == 0)? "Disabled" : "Enabled");
  printf("  - MPU protection is %s\n", ((module_properties & 0x02) == 0)? "Disabled" : "Enabled");
  printf("  - %s mode execution is enabled for the module\n\n", ((module_properties & 0x01) == 0)? "Privileged" : "User");

  /* Start the modules. */
  status = txm_module_manager_start(&my_module);

  if(status != TX_SUCCESS)
  {
    Error_Handler();
  }

  printf("Module execution is started\n");

  /* Get Module's progress messages */
  while(r_msg != PROCESSING_FINISHED)
  {
    if(tx_queue_receive(&ResidentQueue, &r_msg, TX_TIMER_TICKS_PER_SECOND) == TX_SUCCESS)
    {
      /* Convert the message to a user friendly string */
      pretty_msg(p_msg, r_msg);

      printf("Module is executing: %s\n", p_msg);

      /* Check if the last executed operation resulted in memory violation */
      if(memory_faults)
      {
        /* A memory access fault just occurred */
        printf("A memory fault occurred while module executed: %s\n", p_msg);
        break;
      }
    }
  }

  /* Stop the modules. */
  status = txm_module_manager_stop(&my_module);

  if(status != TX_SUCCESS)
  {
    Error_Handler();
  }

  /* Unload the modules. */
  status = txm_module_manager_unload(&my_module);

  if(status != TX_SUCCESS)
  {
    Error_Handler();
  }

  /* Toggle green LED to indicated success of operations */
  while(1) {
    BSP_LED_Toggle(LED_GREEN);
    tx_thread_sleep(TX_TIMER_TICKS_PER_SECOND / 2);
  }
}

VOID module_fault_handler(TX_THREAD *thread, TXM_MODULE_INSTANCE *module)
{
    /* Just increment the fault counter.   */
    memory_faults++;
}

VOID pretty_msg(char *p_msg, ULONG r_msg)
{
  memset(p_msg, 0, 64);

  switch(r_msg)
  {
  case WRITING_TO_READWRITE:
    memcpy(p_msg, "Writing to ReadWrite Region", 27);
    break;
  case WRITING_TO_READONLY:
    memcpy(p_msg, "Writing to ReadOnly Region", 26);
    break;
  case READING_FROM_READWRITE:
    memcpy(p_msg, "Reading from ReadWrite Region", 29);
    break;
  case READING_FROM_READONLY:
    memcpy(p_msg, "Reading from ReadOnly Region", 28);
    break;
  case PROCESSING_FINISHED:
    memcpy(p_msg, "All operations were done", 24);
    break;
  default:
    memcpy(p_msg, "Invalid option", 14);
    break;
  }
}
