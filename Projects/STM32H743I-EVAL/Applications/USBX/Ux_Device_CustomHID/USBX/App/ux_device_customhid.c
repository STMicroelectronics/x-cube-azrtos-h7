/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    ux_device_customhid.c
  * @author  MCD Application Team
  * @brief   USBX Device Custom HID applicative source file
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2022 STMicroelectronics.
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
#include "ux_device_customhid.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */

UX_SLAVE_CLASS_HID *hid_custom;
UX_SLAVE_CLASS_HID_EVENT hid_event;
UX_DEVICE_CLASS_HID_RECEIVED_EVENT hid_received_event;

extern TX_QUEUE ux_hid_msgqueue;
extern ADC_HandleTypeDef hadc1;
ALIGN_32BYTES (int32_t ADCConvertedValue[8]) = {0};
int32_t ADC_Prev_ConvertedValue = 0;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */
static VOID Led_Toggle(UINT event_idx, UINT state);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  USBD_Custom_HID_Activate
  *         This function is called when insertion of a Custom HID device.
  * @param  hid_instance: Pointer to the hid class instance.
  * @retval none
  */
VOID USBD_Custom_HID_Activate(VOID *hid_instance)
{
  /* USER CODE BEGIN USBD_Custom_HID_Activate */

  /* Save the Custom HID instance */
  hid_custom = (UX_SLAVE_CLASS_HID*) hid_instance;

  /* Perform an ADC automatic self-calibration */
  HAL_ADCEx_Calibration_Start(&hadc1, ADC_CALIB_OFFSET, ADC_SINGLE_ENDED);

  /* Enable ADC, start conversion of regular group and transfer result through DMA  */
  HAL_ADC_Start_DMA(&hadc1, (uint32_t *) &ADCConvertedValue, 1);

  /* USER CODE END USBD_Custom_HID_Activate */

  return;
}

/**
  * @brief  USBD_Custom_HID_Deactivate
  *         This function is called when extraction of a Custom HID device.
  * @param  hid_instance: Pointer to the hid class instance.
  * @retval none
  */
VOID USBD_Custom_HID_Deactivate(VOID *hid_instance)
{
  /* USER CODE BEGIN USBD_Custom_HID_Deactivate */
  UX_PARAMETER_NOT_USED(hid_instance);

  /* Reset the Custom HID instance */
  hid_custom = UX_NULL;

  /* Stop ADC conversion of regular group */
  HAL_ADC_Stop_DMA(&hadc1);

  /* USER CODE END USBD_Custom_HID_Deactivate */

  return;
}

/**
  * @brief  USBD_Custom_HID_SetFeature
  *         This function is invoked when the host sends a HID SET_REPORT
  *         to the application over Endpoint 0 (Set Feature).
  * @param  hid_instance: Pointer to the hid class instance.
  * @param  hid_event: Pointer to structure of the hid event.
  * @retval status
  */
UINT USBD_Custom_HID_SetFeature(UX_SLAVE_CLASS_HID *hid_instance,
                                UX_SLAVE_CLASS_HID_EVENT *hid_event)
{
  UINT status = UX_SUCCESS;

  /* USER CODE BEGIN USBD_Custom_HID_SetFeature */
  UX_PARAMETER_NOT_USED(hid_instance);

  UINT event_idx, state;

  /* Get LED report number */
  event_idx = hid_event->ux_device_class_hid_event_report_id;

  /* Get LED states from hid event buffer */
  state = hid_event->ux_device_class_hid_event_buffer[0];

  Led_Toggle(event_idx, state);

  /* USER CODE END USBD_Custom_HID_SetFeature */

  return status;
}

/**
  * @brief  USBD_Custom_HID_GetReport
  *         This function is invoked when host is requesting event through
  *         control GET_REPORT request.
  * @param  hid_instance: Pointer to the hid class instance.
  * @param  hid_event: Pointer to structure of the hid event.
  * @retval status
  */
UINT USBD_Custom_HID_GetReport(UX_SLAVE_CLASS_HID *hid_instance,
                               UX_SLAVE_CLASS_HID_EVENT *hid_event)
{
  UINT status = UX_SUCCESS;

  /* USER CODE BEGIN USBD_Custom_HID_GetReport */
  UX_PARAMETER_NOT_USED(hid_instance);
  UX_PARAMETER_NOT_USED(hid_event);
  /* USER CODE END USBD_Custom_HID_GetReport */

  return status;
}

#ifdef UX_DEVICE_CLASS_HID_INTERRUPT_OUT_SUPPORT

/**
  * @brief  USBD_Custom_HID_SetReport
  *         This function is invoked when the host sends a HID SET_REPORT
  *         to the application over Endpoint OUT (Set Report).
  * @param  hid_instance: Pointer to the hid class instance.
  * @retval none
  */
VOID USBD_Custom_HID_SetReport(struct UX_SLAVE_CLASS_HID_STRUCT *hid_instance)
{
  /* USER CODE BEGIN USBD_Custom_HID_SetReport */
  UX_PARAMETER_NOT_USED(hid_instance);

  UINT event_idx, state;

  ux_utility_memory_set(&hid_received_event, 0, sizeof(UX_DEVICE_CLASS_HID_RECEIVED_EVENT));

  if(ux_device_class_hid_receiver_event_get(hid_custom, &hid_received_event) == UX_SUCCESS)
  {
    /* Get LED report number */
    event_idx = hid_received_event.ux_device_class_hid_received_event_data[0];

    /* Get LED states from hid event buffer */
    state = hid_received_event.ux_device_class_hid_received_event_data[1];

    /* Toggle Leds */
    Led_Toggle(event_idx, state);

    /* Free hid received event */
    ux_device_class_hid_receiver_event_free(hid_custom);
  }

  /* USER CODE END USBD_Custom_HID_SetReport */

  return;
}

/**
  * @brief  USBD_Custom_HID_EventMaxNumber
  *         This function to set receiver event max number parameter.
  * @param  none
  * @retval receiver event max number
  */
ULONG USBD_Custom_HID_EventMaxNumber(VOID)
{
  ULONG max_number = 0U;

  /* USER CODE BEGIN USBD_Custom_HID_EventMaxNumber */

  max_number = 1;

  /* USER CODE END USBD_Custom_HID_EventMaxNumber */

  return max_number;
}

/**
  * @brief  USBD_Custom_HID_EventMaxLength
  *         This function to set receiver event max length parameter.
  * @param  none
  * @retval receiver event max length
  */
ULONG USBD_Custom_HID_EventMaxLength(VOID)
{
  ULONG max_length = 0U;

  /* USER CODE BEGIN USBD_Custom_HID_EventMaxLength */

  /* Set max event length */
  max_length = USBD_HID_CUSTOM_EPOUT_HS_MPS;

  /* USER CODE END USBD_Custom_HID_EventMaxLength */

  return max_length;
}

#endif /* UX_DEVICE_CLASS_HID_INTERRUPT_OUT_SUPPORT */

/* USER CODE BEGIN 1 */

/**
  * @brief  Function implementing usbx_cutomhid_thread_entry.
  * @param  thread_input: not used
  * @retval none
  */
VOID usbx_cutomhid_thread_entry(ULONG thread_input)
{
  UX_SLAVE_DEVICE *device;

  UX_PARAMETER_NOT_USED(thread_input);

  device = &_ux_system_slave->ux_system_slave_device;

  ux_utility_memory_set(&hid_event, 0, sizeof(UX_SLAVE_CLASS_HID_EVENT));

  while (1)
  {
    /* Check if the device state already configured */
    if ((device->ux_slave_device_state == UX_DEVICE_CONFIGURED) && (hid_custom != UX_NULL))
    {

      /* Wait for a hid event */
      if (tx_queue_receive(&ux_hid_msgqueue, &hid_event, TX_WAIT_FOREVER) != TX_SUCCESS)
      {
        Error_Handler();
      }

      /* Send hid event */
      ux_device_class_hid_event_set(hid_custom, &hid_event);
    }
    else
    {
      /* Sleep thread for 10ms */
      tx_thread_sleep(MS_TO_TICK(10));
    }
  }
}

/**
  * @brief  HAL_GPIO_EXTI_Callback
  *         EXTI line detection callbacks.
  * @param  GPIO_Pin: Specifies the pins connected EXTI line
  * @retval none
  */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
  /* Set hid event length */
  hid_event.ux_device_class_hid_event_length = 1;

  /* Set button tamper report id */
  hid_event.ux_device_class_hid_event_report_id = TAMPER_REPORT_ID;

    /* Set button tamper event buffer */
  if(HAL_GPIO_ReadPin(BUTTON_USER_GPIO_Port, BUTTON_USER_Pin) == GPIO_PIN_RESET)
  {
    hid_event.ux_device_class_hid_event_buffer[0] = 0x01;
  }
  else
  {
    hid_event.ux_device_class_hid_event_buffer[0] = 0x00;
  }

  /* Send hid event */
  ux_device_class_hid_event_set(hid_custom, &hid_event);
}

/**
  * @brief  HAL_ADC_ConvCpltCallback
  *         Conversion complete callback in non-blocking mode.
  * @param  hadc: ADC handle
  * @retval none
  */
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc)
{

  /* Set hid event length */
  hid_event.ux_device_class_hid_event_length = 1;

  /* Set potentiometer report id */
  hid_event.ux_device_class_hid_event_report_id = ADC_REPORT_ID;

  /* D-Cache Invalidate by address of ADCConvertedValue */
  SCB_InvalidateDCache_by_Addr(ADCConvertedValue, 4);

  if (abs((ADCConvertedValue[0] >> 4) - (ADC_Prev_ConvertedValue >> 4)) > 4)
  {
    /* Set event buffer */
    hid_event.ux_device_class_hid_event_buffer[0] = (uint8_t)(ADCConvertedValue[0] >> 4);

    /* Put a hid event message queue */
    if(tx_queue_send(&ux_hid_msgqueue, &hid_event, TX_NO_WAIT) != TX_SUCCESS)
    {
      Error_Handler();
    }

    /* Update previous ADC Converted value */
    ADC_Prev_ConvertedValue = ADCConvertedValue[0];
  }

}

/**
  * @brief  Led_Toggle
  * @param  event_idx: LED Report Number
  * @param  state: LED states (ON/OFF)
  * @retval none
  */
static VOID Led_Toggle(UINT event_idx, UINT state)
{
  switch (event_idx)
  {
    case LED1_REPORT_ID:
      (state == 1) ? BSP_LED_On(LED1) : BSP_LED_Off(LED1);
      break;

    case LED2_REPORT_ID:
      (state == 1) ? BSP_LED_On(LED2) : BSP_LED_Off(LED2);
      break;

    case LED3_REPORT_ID:
      (state == 1) ? BSP_LED_On(LED3) : BSP_LED_Off(LED3);
      break;

    case LED4_REPORT_ID:
      (state == 1) ? BSP_LED_On(LED4) : BSP_LED_Off(LED4);
      break;

    default:
      BSP_LED_Off(LED1);
      BSP_LED_Off(LED2);
      BSP_LED_Off(LED3);
      BSP_LED_Off(LED4);
      break;
  }
}

/* USER CODE END 1 */
