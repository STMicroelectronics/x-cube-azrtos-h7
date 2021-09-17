/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    ux_host_cdc_acm.c
  * @author  MCD Application Team
  * @brief   USBX Host applicative file
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
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "app_usbx_host.h"
#include "main.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define APP_RX_DATA_SIZE             2048U
#define BLOCK_SIZE                   64U
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */
extern UX_HOST_CLASS_CDC_ACM         *app_cdc_acm;
extern TX_EVENT_FLAGS_GROUP          ux_app_EventFlag;
extern ux_app_stateTypeDef           ux_app_state;
extern TX_BYTE_POOL                  *ux_app_byte_pool;
extern ux_app_devInfotypeDef          ux_dev_info;
UX_HOST_CLASS_CDC_ACM_RECEPTION      app_cdc_acm_reception;
ULONG                                tx_actual_length;
ULONG                                command_received_count;
UCHAR                                *UserRxBuffer;
UCHAR                                UserTxBuffer[] = "USBX_STM32_Host_CDC_ACM \n";
uint8_t                              ReceptionSizeTab[APP_RX_DATA_SIZE / BLOCK_SIZE];
uint16_t                             RxSzeIdx;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */
void cdc_acm_send_app_thread_entry(ULONG arg);
void cdc_acm_recieve_app_thread_entry(ULONG arg);
void cdc_acm_reception_callback(struct UX_HOST_CLASS_CDC_ACM_STRUCT *cdc_acm, UINT status,
                                UCHAR *reception_buffer, ULONG reception_size);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/**
  * @brief  cdc_acm_send_app_thread_entry .
  * @param  ULONG arg
  * @retval Void
  */
void cdc_acm_send_app_thread_entry(ULONG arg)
{
  ULONG     send_dataflag = 0;
  UINT      status = 0;

  while (1)
  {
    /* Wait until the requested flag NEW_DATA_TO_SEND is received */
    if (tx_event_flags_get(&ux_app_EventFlag, NEW_DATA_TO_SEND, TX_OR_CLEAR,
                           &send_dataflag, TX_WAIT_FOREVER) != TX_SUCCESS)
    {
      Error_Handler();
    }
    /* Start sending data*/
    status = _ux_host_class_cdc_acm_write(app_cdc_acm, UserTxBuffer,
                                          _ux_utility_string_length_get(UserTxBuffer), &tx_actual_length);
    if (status == UX_SUCCESS)
    {
      USBH_UsrLog("Data sent successfully");
    }
    else
    {
      USBH_ErrLog("Unable to send data");
    }
  }
}

/**
  * @brief  cdc_acm_recieve_app_thread_entry .
  * @param  ULONG arg
  * @retval Void
  */
void cdc_acm_recieve_app_thread_entry(ULONG arg)
{
  UINT           status;
  UCHAR          *ptr_read = NULL;
  ULONG          op_count = 0;
  ULONG          receive_dataflag = 0;
  uint16_t       count = 0;
  uint16_t       print_count = 0;

  while (1)
  {
    switch (ux_app_state)
    {
      case App_Ready:

        /* allocate memory from usb_pool for user reception buffer*/
        if (tx_byte_allocate(ux_app_byte_pool, (VOID **) &UserRxBuffer,
                             APP_RX_DATA_SIZE, TX_NO_WAIT) != TX_SUCCESS)
        {
          Error_Handler();
        }

        /* pointer to USB User buffer reception*/
        ptr_read = UserRxBuffer;

        /* Setup the background reception parameter. */

        /* Set the desired max read size for each transaction.
        For example, if this value is 64, then the maximum amount of
        data received from the device in a single transaction is 64.
        If the amount of data received from the device is less than this value,
        the callback will still be invoked with the actual amount of data received. */
        app_cdc_acm_reception.ux_host_class_cdc_acm_reception_block_size = BLOCK_SIZE;

        /* Initialize the buffer where the data from the device is read to */
        _ux_utility_memory_set(UserRxBuffer, 0, APP_RX_DATA_SIZE);

        /* Set the buffer where the data from the device is read to. */
        app_cdc_acm_reception.ux_host_class_cdc_acm_reception_data_buffer = UserRxBuffer;

        /* Set the size of the data reception buffer.
        Note that this should be at least as large as ux_host_class_cdc_acm_reception_block_size. */
        app_cdc_acm_reception.ux_host_class_cdc_acm_reception_data_buffer_size = APP_RX_DATA_SIZE;

        /* Set the callback that is to be invoked upon each reception transfer completion. */
        app_cdc_acm_reception.ux_host_class_cdc_acm_reception_callback = cdc_acm_reception_callback;

        /* Start background reception using the values we defined in the reception parameter. */
        status = ux_host_class_cdc_acm_reception_start(app_cdc_acm, &app_cdc_acm_reception);

        /* If status equals UX_SUCCESS, background reception has successfully started. */
        if (status == UX_SUCCESS)
        {
          USBH_UsrLog("Ready to send or receive data \n");
          ux_app_state = App_Start;
        }
        else
        {
          USBH_UsrLog("Unable to start reception\n");
          ux_app_state = App_Idle;
        }

        break;

      case App_Start:

        /* Wait until the requested flag NEW_RECEIVED_DATA is received */
        if (tx_event_flags_get(&ux_app_EventFlag, NEW_RECEIVED_DATA, TX_OR_CLEAR,
                               &receive_dataflag, TX_WAIT_FOREVER) != TX_SUCCESS)
        {
          Error_Handler();
        }
        /* Check if CDC_ACM is connected */
        if (ux_dev_info.CDC_Device_Type == CDC_ACM_Device)
        {
          while (op_count < command_received_count)
          {
            /* check if ptr_read reached end of user buffer */
            if (ptr_read >= app_cdc_acm_reception.ux_host_class_cdc_acm_reception_data_buffer +
                app_cdc_acm_reception.ux_host_class_cdc_acm_reception_data_buffer_size)
            {
              ptr_read = app_cdc_acm_reception.ux_host_class_cdc_acm_reception_data_buffer;

              /* Reinitialize index of reception array */
              count = 0;
            }

            /* start printing received data */
            for (print_count = 0; print_count < ReceptionSizeTab[count]; print_count++)
            {
              /* print received data for each block size */
              USBH_UsrLog("%c", *ptr_read);
              ptr_read++;
            }

            /* Increment pointer by max packet size */
            ptr_read += (BLOCK_SIZE - print_count);

            /* proceed to next read operation */
            count++;
            op_count++;
          }
        }
        break;

      case App_Idle:
      default:
        tx_thread_sleep(1);
        break;
    }
  }
}
/**
  * @brief  cdc_acm_reception_callback .
  * @param  struct UX_HOST_CLASS_CDC_ACM_STRUCT *cdc_acm
            UINT status
            UCHAR *reception_buffer
            ULONG reception_size
  * @retval Void
  */
void cdc_acm_reception_callback(struct UX_HOST_CLASS_CDC_ACM_STRUCT *cdc_acm,
                                UINT status, UCHAR *reception_buffer, ULONG reception_size)
{
  /* Total number of reception operation */
  command_received_count++;

  /* save reception size for each operation */
  ReceptionSizeTab[RxSzeIdx] = reception_size;

  /* Increment index for next reception size */
  RxSzeIdx++;

  /* check if tail has reached end of user buffer */
  if (app_cdc_acm_reception.ux_host_class_cdc_acm_reception_data_tail + app_cdc_acm_reception.ux_host_class_cdc_acm_reception_block_size >=
      app_cdc_acm_reception.ux_host_class_cdc_acm_reception_data_buffer + app_cdc_acm_reception.ux_host_class_cdc_acm_reception_data_buffer_size)
  {
    /* We are at the end of the buffer. Move back to the beginning.  */
    app_cdc_acm_reception.ux_host_class_cdc_acm_reception_data_tail = app_cdc_acm_reception.ux_host_class_cdc_acm_reception_data_buffer;

    /*Reinitialize the index of reception array */
    RxSzeIdx = 0U;
  }
  else
  {
    /* Program the tail to be after the current buffer.  */
    app_cdc_acm_reception.ux_host_class_cdc_acm_reception_data_tail += app_cdc_acm_reception.ux_host_class_cdc_acm_reception_block_size;
  }

  /* Set NEW_RECEIVED_DATA flag */
  if (tx_event_flags_set(&ux_app_EventFlag, NEW_RECEIVED_DATA, TX_OR) != TX_SUCCESS)
  {
    Error_Handler();
  }
}
/* USER CODE END 0 */

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */
