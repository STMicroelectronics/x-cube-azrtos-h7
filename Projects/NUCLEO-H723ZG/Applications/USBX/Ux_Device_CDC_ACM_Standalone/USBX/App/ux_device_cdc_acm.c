/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    ux_device_cdc_acm.c
  * @author  MCD Application Team
  * @brief   USBX Device applicative file
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
#include "ux_device_cdc_acm.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "main.h"
#include "app_usbx_device.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define APP_RX_DATA_SIZE                          2048
#define APP_TX_DATA_SIZE                          2048

/* Rx/TX flag */
#define RX_NEW_RECEIVED_DATA                      0x01
#define TX_NEW_TRANSMITTED_DATA                   0x02

/* Data length for vcp */
#define VCP_WORDLENGTH8                           8
#define VCP_WORDLENGTH9                           9

/* the minimum baudrate */
#define MIN_BAUDRATE                              9600

#define APP_CDC_ACM_READ_STATE_TX_START  (UX_STATE_APP_STEP + 0)
#define APP_CDC_ACM_READ_STATE_TX_WAIT   (UX_STATE_APP_STEP + 1)
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */
static UINT write_state = UX_STATE_RESET;
static UINT read_state = UX_STATE_RESET;

volatile ULONG EventFlag = 0;
#if defined ( __ICCARM__ ) /* IAR Compiler */
#pragma location = 0x24028000
#elif defined ( __CC_ARM ) /* MDK ARM Compiler */
__attribute__((section(".UsbxAppSection")))
#elif defined ( __GNUC__ ) /* GNU Compiler */
__attribute__((section(".UsbxAppSection")))
#endif

/* Data received over uart are stored in this buffer */
uint8_t UserRxBufferFS[APP_RX_DATA_SIZE];

/* Data to send over USB CDC are stored in this buffer   */
uint8_t UserTxBufferFS[APP_TX_DATA_SIZE];

/* Increment this pointer or roll it back to
start address when data are received over USART */
uint32_t UserTxBufPtrIn;

/* Increment this pointer or roll it back to
start address when data are sent over USB */
uint32_t UserTxBufPtrOut;

/* uart3 handler */
extern UART_HandleTypeDef huart3;

UX_SLAVE_CLASS_CDC_ACM_LINE_CODING_PARAMETER CDC_VCP_LineCoding =
{
  115200, /* baud rate */
  0x00,   /* stop bits-1 */
  0x00,   /* parity - none */
  0x08    /* nb. of bits 8 */
};

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */
static void USBD_CDC_VCP_Config(UX_SLAVE_CLASS_CDC_ACM_LINE_CODING_PARAMETER *);
extern void MX_USART3_UART_Init(void);
extern void Error_Handler(void);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
/**
  * @brief  CDC_ACM_Init
            Initializes the CDC media low layer
  * @param  cdc_acm: cdc Instance
  * @retval none
  */
VOID CDC_ACM_Init(VOID *cdc_acm)
{
  /* Status */
  UINT ux_status = UX_SUCCESS;

  /* USER CODE BEGIN 3 */
  /* Configure the UART peripheral */
  MX_USART3_UART_Init();

  /* Get uart3 baudrate */
  CDC_VCP_LineCoding.ux_slave_class_cdc_acm_parameter_baudrate = huart3.Init.BaudRate;

  /*set the data type : only 8bits and 9bits are supported */
  switch (huart3.Init.WordLength)
  {
    case UART_WORDLENGTH_8B:
    {
      /* Set data bit to 8 */
      CDC_VCP_LineCoding.ux_slave_class_cdc_acm_parameter_data_bit = VCP_WORDLENGTH8;
      break;
    }

    case UART_WORDLENGTH_9B:
    {
      /* Set data bit to 9 */
      CDC_VCP_LineCoding.ux_slave_class_cdc_acm_parameter_data_bit = VCP_WORDLENGTH9;
      break;
    }

    default :
    {
      /* By default set data bit to 8 */
      CDC_VCP_LineCoding.ux_slave_class_cdc_acm_parameter_data_bit = VCP_WORDLENGTH8;
      break;
    }
  }

  /* Get uart3 Parity */
  CDC_VCP_LineCoding.ux_slave_class_cdc_acm_parameter_parity = huart3.Init.Parity;

  /* Get uart3 StopBits */
  CDC_VCP_LineCoding.ux_slave_class_cdc_acm_parameter_stop_bit = huart3.Init.StopBits;

  /* Set device_class_cdc_acm with default parameters */
  ux_status = ux_device_class_cdc_acm_ioctl(cdc_acm,
                                            UX_SLAVE_CLASS_CDC_ACM_IOCTL_SET_LINE_CODING,
                                            &CDC_VCP_LineCoding);
  /* Check Status */
  if (ux_status != UX_SUCCESS)
  {
    Error_Handler();
  }

  /* Put UART peripheral in IT reception process */

  /* Any data received will be stored in "UserTxBufferFS" buffer  */
  if (HAL_UART_Receive_IT(&huart3, (uint8_t *)UserTxBufferFS, 1) != HAL_OK)
  {
    /* Transfer error in reception process */
    Error_Handler();
  }
  
  EventFlag = 0;

  /* USER CODE END 3 */
}

/**
  * @brief  CDC_ACM_DeInit
            DeInitializes the CDC media low layer
  * @param  cdc_acm: cdc Instance
  * @retval none
  */
VOID CDC_ACM_DeInit(VOID *cdc_acm)
{
  /* USER CODE BEGIN 4 */
  /* DeInitialize the UART peripheral */
  if (HAL_UART_DeInit(&huart3) != HAL_OK)
  {
    /* Initialization Error */
    Error_Handler();
  }
  /* USER CODE END 4 */
}

/**
  * @brief  CDC_ACM_Control
            Manage the CDC class requests
  * @param  cdc_acm: cdc instance
  * @retval none
  */
VOID CDC_ACM_Control(VOID *cdc_acm)
{
  UX_SLAVE_TRANSFER *transfer_request;
  UX_SLAVE_DEVICE   *device;
  ULONG             request;
  UINT ux_status = UX_SUCCESS;

  /* Get the pointer to the device.  */
  device = &_ux_system_slave -> ux_system_slave_device;

  /* Get the pointer to the transfer request associated with the control endpoint. */
  transfer_request = &device -> ux_slave_device_control_endpoint.
                     ux_slave_endpoint_transfer_request;

  /* Extract all necessary fields of the request. */
  request = *(transfer_request -> ux_slave_transfer_request_setup + UX_SETUP_REQUEST);

  /* Here we proceed only the standard request we know of at the device level.  */
  switch (request)
  {
    /* Set Line Coding Command */
    case UX_SLAVE_CLASS_CDC_ACM_SET_LINE_CODING :
    {
      /* Get the Line Coding parameters */
      ux_status = ux_device_class_cdc_acm_ioctl(cdc_acm,
                                                UX_SLAVE_CLASS_CDC_ACM_IOCTL_GET_LINE_CODING,
                                                &CDC_VCP_LineCoding);
      /* Check Status */
      if (ux_status != UX_SUCCESS)
      {
        Error_Handler();
      }

      /* Check if baudrate < 9600) then set it to 9600 */
      if (CDC_VCP_LineCoding.ux_slave_class_cdc_acm_parameter_baudrate < MIN_BAUDRATE)
      {
        CDC_VCP_LineCoding.ux_slave_class_cdc_acm_parameter_baudrate = MIN_BAUDRATE;

        /* Set the new configuration of ComPort */
        USBD_CDC_VCP_Config(&CDC_VCP_LineCoding);
      }
      else
      {
        /* Set the new configuration of ComPort */
        USBD_CDC_VCP_Config(&CDC_VCP_LineCoding);
      }
      break;
    }

    /* Get Line Coding Command */
    case UX_SLAVE_CLASS_CDC_ACM_GET_LINE_CODING :
    {
      ux_status = ux_device_class_cdc_acm_ioctl(cdc_acm,
                                                UX_SLAVE_CLASS_CDC_ACM_IOCTL_SET_LINE_CODING,
                                                &CDC_VCP_LineCoding);

      /* Check Status */
      if (ux_status != UX_SUCCESS)
      {
        Error_Handler();
      }
      break;
    }

    /* Set the the control line state */
    case UX_SLAVE_CLASS_CDC_ACM_SET_CONTROL_LINE_STATE :
    default :
      break;
  }
}

VOID CDC_ACM_Read_Task(VOID)
{
  UX_SLAVE_DEVICE *device;
  UX_SLAVE_INTERFACE *data_interface;
  UX_SLAVE_CLASS_CDC_ACM *cdc_acm;
  UINT  status;
  ULONG read_length;
  static ULONG actual_length;

  /* Get device */
  device = &_ux_system_slave->ux_system_slave_device;

  /* Check if device is configured */
  if (device->ux_slave_device_state != UX_DEVICE_CONFIGURED)
  {
    read_state = UX_STATE_RESET;
    return;
  }

  /* Get Data interface (interface 1) */
  data_interface = device->ux_slave_device_first_interface->ux_slave_interface_next_interface;
  cdc_acm =  data_interface->ux_slave_interface_class_instance;
  read_length = (_ux_system_slave->ux_system_slave_speed == UX_HIGH_SPEED_DEVICE) ? 512 : 64;

  /* Run state machine.  */
  switch(read_state)
  {
    case UX_STATE_RESET:
      read_state = UX_STATE_WAIT;
      /* Fall through.  */
    case UX_STATE_WAIT:
      status = ux_device_class_cdc_acm_read_run(cdc_acm,
                                                (UCHAR *)UserRxBufferFS, read_length,
                                                &actual_length);
      /* Error.  */
      if (status <= UX_STATE_ERROR)
      {
        /* Reset state.  */
        read_state = UX_STATE_RESET;
        return;
      }
      if (status == UX_STATE_NEXT)
      {
        if (actual_length != 0)
        {
          read_state = APP_CDC_ACM_READ_STATE_TX_START;
        }
        else
        {
          read_state = UX_STATE_RESET;
        }
        return;
      }
      /* Wait.  */
      return;
    case APP_CDC_ACM_READ_STATE_TX_START:
      /* Send the data via UART */
      status = HAL_UART_Transmit_DMA(&huart3, (uint8_t *)UserRxBufferFS,
                                      actual_length);
      if (status == HAL_BUSY)
      {
        /* Keep trying.  */
        return;
      }
      if (status != HAL_OK)
      {
        /* Transfer error in reception process */
        Error_Handler();
      }
      /* DMA started.  */
      read_state = APP_CDC_ACM_READ_STATE_TX_WAIT;
      /* Fall through.  */
    case APP_CDC_ACM_READ_STATE_TX_WAIT:
      if (EventFlag & TX_NEW_TRANSMITTED_DATA)
      {  
        EventFlag &= ~TX_NEW_TRANSMITTED_DATA;
      }
      read_state = UX_STATE_WAIT;
      return;
    default:
      return;
  }
}


VOID CDC_ACM_Write_Task(VOID)
{
  UX_SLAVE_DEVICE    *device;
  UX_SLAVE_INTERFACE *data_interface;
  UX_SLAVE_CLASS_CDC_ACM *cdc_acm;
  ULONG actual_length;
  ULONG buffptr;
  ULONG buffsize;
  UINT ux_status = UX_SUCCESS;

  /* Get device */
  device = &_ux_system_slave->ux_system_slave_device;

  /* Check if device is configured */
  if (device->ux_slave_device_state != UX_DEVICE_CONFIGURED)
  {
    read_state = UX_STATE_RESET;
    return;
  }

  /* Get Data interface */
  data_interface = device->ux_slave_device_first_interface->ux_slave_interface_next_interface;
  cdc_acm =  data_interface->ux_slave_interface_class_instance;

  switch(write_state)
  {
    case UX_STATE_RESET:
      if (EventFlag & RX_NEW_RECEIVED_DATA)
      {
        EventFlag &= ~RX_NEW_RECEIVED_DATA;
        /* Check if there is a new data to send */
        if (UserTxBufPtrOut != UserTxBufPtrIn)
        {
          /* Check buffer overflow and Rollback */
          if (UserTxBufPtrOut > UserTxBufPtrIn)
          {
            buffsize = APP_RX_DATA_SIZE - UserTxBufPtrOut;
          }
          else
          {
            /* Calculate data size */
            buffsize = UserTxBufPtrIn - UserTxBufPtrOut;
          }

          /* Copy UserTxBufPtrOut in buffptr */
          buffptr = UserTxBufPtrOut;

          /* Send data over the class cdc_acm_write */
          ux_status = ux_device_class_cdc_acm_write_run(cdc_acm,
                                                        (UCHAR *)(&UserTxBufferFS[buffptr]),
                                                        buffsize, &actual_length);
          if (ux_status != UX_STATE_WAIT)
          {
            /* Reset state.  */
            read_state = UX_STATE_RESET;
            return;
          }
          write_state = UX_STATE_WAIT;
          return;
        }
        return;
      }
      return;
      
    case UX_STATE_WAIT:
      /* Continue to run state machine.  */
      ux_status = ux_device_class_cdc_acm_write_run(cdc_acm, UX_NULL, 0, &actual_length);
      /* Check if there is  fatal error.  */
      if (ux_status < UX_STATE_IDLE)
      {
        /* Reset state.  */
        read_state = UX_STATE_RESET;
        return;
      }
      /* Check if dataset is transmitted */
      if (ux_status <= UX_STATE_NEXT)
      {
        /* Increment the UserTxBufPtrOut pointer */
        UserTxBufPtrOut += actual_length;

        /* Rollback UserTxBufPtrOut if it equal to APP_TX_DATA_SIZE */
        if (UserTxBufPtrOut == APP_TX_DATA_SIZE)
        {
          UserTxBufPtrOut = 0;
        }
        write_state = UX_STATE_RESET;
      }
      /* Keep waiting.  */
      return;
    default:
      return;
  }
}

/**
  * @brief Tx Transfer completed callback.
  * @param huart UART handle.
  * @retval None
  */
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
  EventFlag |= TX_NEW_TRANSMITTED_DATA;
}

/**
  * @brief  Rx Transfer completed callback
  * @param  huart: UART handle
  * @retval None
  */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{

  EventFlag |= RX_NEW_RECEIVED_DATA;
  
    /* Increment the UserTxBufPtrIn pointer */
  UserTxBufPtrIn++;

  /* Rollback the UserTxBufPtrIn if it equal to APP_TX_DATA_SIZE */
  if (UserTxBufPtrIn == APP_TX_DATA_SIZE)
  {
    UserTxBufPtrIn = 0;
  }
  
  /* Start another reception: provide the buffer pointer with offset and the buffer size */
  if (HAL_UART_Receive_IT(&huart3, (uint8_t *)UserTxBufferFS + UserTxBufPtrIn, 1) != HAL_OK)
  {
    /* Transfer error in reception process */
    Error_Handler();
  }
}

/**
  * @brief  UART error callbacks
  * @param  UartHandle: UART handle
  * @retval None
  */
void HAL_UART_ErrorCallback(UART_HandleTypeDef *UartHandle)
{
  /* Transfer error occurred in reception and/or transmission process */
  Error_Handler();
}

/**
  * @brief  USBD_CDC_VCP_Config
  *         Configure the COM Port with the parameters received from host.
  * @param  UX_SLAVE_CLASS_CDC_ACM_LINE_CODING_PARAMETER : linecoding struct.
  * @retval None.
  * @note   When a configuration is not supported, a default value is used.
  */
static void USBD_CDC_VCP_Config(UX_SLAVE_CLASS_CDC_ACM_LINE_CODING_PARAMETER
                                *CDC_VCP_LineCoding)
{
  /* Deinitialization uart3 */
  if (HAL_UART_DeInit(&huart3) != HAL_OK)
  {
    /* Deinitialization Error */
    Error_Handler();
  }

  /* set the Stop bit */
  switch (CDC_VCP_LineCoding->ux_slave_class_cdc_acm_parameter_stop_bit)
  {
    case 0:
    {
      huart3.Init.StopBits = UART_STOPBITS_1;
      break;
    }
    case 2:
    {
      huart3.Init.StopBits = UART_STOPBITS_2;
      break;
    }
    default :
    {
      huart3.Init.StopBits = UART_STOPBITS_1;
      break;
    }
  }

  /* set the parity bit */
  switch (CDC_VCP_LineCoding->ux_slave_class_cdc_acm_parameter_parity)
  {
    case 0:
    {
      huart3.Init.Parity = UART_PARITY_NONE;
      break;
    }
    case 1:
    {
      huart3.Init.Parity = UART_PARITY_ODD;
      break;
    }
    case 2:
    {
      huart3.Init.Parity = UART_PARITY_EVEN;
      break;
    }
    default :
    {
      huart3.Init.Parity = UART_PARITY_NONE;
      break;
    }
  }

  /* Set the data type : only 8bits and 9bits is supported */
  switch (CDC_VCP_LineCoding->ux_slave_class_cdc_acm_parameter_data_bit)
  {
    case 0x07:
    {
      /* With this configuration a parity (Even or Odd) must be set */
      huart3.Init.WordLength = UART_WORDLENGTH_8B;
      break;
    }
    case 0x08:
    {
      if (huart3.Init.Parity == UART_PARITY_NONE)
      {
        huart3.Init.WordLength = UART_WORDLENGTH_8B;
      }
      else
      {
        huart3.Init.WordLength = UART_WORDLENGTH_9B;
      }

      break;
    }
    default :
    {
      huart3.Init.WordLength = UART_WORDLENGTH_8B;
      break;
    }
  }

  /* Get the uart baudrate from vcp */
  huart3.Init.BaudRate = CDC_VCP_LineCoding->ux_slave_class_cdc_acm_parameter_baudrate;

  /* Set the Hw flow control to none */
  huart3.Init.HwFlowCtl = UART_HWCONTROL_NONE;

  /* Set the uart mode */
  huart3.Init.Mode = UART_MODE_TX_RX;

  /* the the uart sampling */
  huart3.Init.OverSampling = UART_OVERSAMPLING_16;

  /* Initialization uart3 */
  if (HAL_UART_Init(&huart3) != HAL_OK)
  {
    /* Initialization Error */
    Error_Handler();
  }

  /* Start reception: provide the buffer pointer with offset and the buffer size */
  HAL_UART_Receive_IT(&huart3, (uint8_t *)(UserTxBufferFS + UserTxBufPtrIn), 1);
}

/* USER CODE END 0 */

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */
