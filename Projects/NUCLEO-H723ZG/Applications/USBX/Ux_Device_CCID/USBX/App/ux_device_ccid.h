/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    ux_device_ccid.h
  * @author  MCD Application Team
  * @brief   USBX Device CCID applicative header file
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
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __UX_DEVICE_CCID_H__
#define __UX_DEVICE_CCID_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "ux_api.h"
#include "ux_device_class_ccid.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "app_usbx_device.h"

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* Define CCID APDU related Struct.  */
typedef struct _CCID_APDU_Command
{
  UCHAR   *buffer;    /* Command buffer of packed data.  */
  ULONG   length;     /* Command length of packed data.  */

  UCHAR   *data;      /* Extracted pointer to data bytes.  */
  UCHAR   *Le;        /* Extracted pointer to Number of data expected in response.  */

  UCHAR   CLA;        /* Extracted class byte.  */
  UCHAR   INS;        /* Extracted instruction byte.  */
  UCHAR   P1;         /* Extracted parameter 1.  */
  UCHAR   P2;         /* Extracted parameter 2.  */
  UCHAR   Lc;         /* Extracted parameter Number of data bytes send to card.  */
} CCID_APDU_Command;

typedef struct _CCID_APDU_Response
{
  UCHAR   *buffer;    /* Response buffer of packed data.  */
  ULONG   length;     /* Response buffer max length.  */
  USHORT  Le;         /* Actual response length 0~256.   */
  UCHAR   SW1;        /* Extracted status byte 1.  */
  UCHAR   SW2;        /* Extracted status byte 2.  */
} CCID_APDU_Response;

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
VOID USBD_CCID_Activate(VOID *ccid_instance);
VOID USBD_CCID_Deactivate(VOID *ccid_instance);

UINT USBD_CCID_icc_power_on(ULONG slot, UX_DEVICE_CLASS_CCID_MESSAGES *io_msg);
UINT USBD_CCID_icc_power_off(ULONG slot, UX_DEVICE_CLASS_CCID_MESSAGES *io_msg);
UINT USBD_CCID_get_slot_status(ULONG slot, UX_DEVICE_CLASS_CCID_MESSAGES *io_msg);
UINT USBD_CCID_xfr_block(ULONG slot, UX_DEVICE_CLASS_CCID_MESSAGES *io_msg);
UINT USBD_CCID_get_parameters(ULONG slot, UX_DEVICE_CLASS_CCID_MESSAGES *io_msg);
UINT USBD_CCID_reset_parameters(ULONG slot, UX_DEVICE_CLASS_CCID_MESSAGES *io_msg);
UINT USBD_CCID_set_parameters(ULONG slot, UX_DEVICE_CLASS_CCID_MESSAGES *io_msg);
UINT USBD_CCID_escape(ULONG slot, UX_DEVICE_CLASS_CCID_MESSAGES *io_msg);
UINT USBD_CCID_icc_clock(ULONG slot, UX_DEVICE_CLASS_CCID_MESSAGES *io_msg);
UINT USBD_CCID_t0_apdu(ULONG slot, UX_DEVICE_CLASS_CCID_MESSAGES *io_msg);
UINT USBD_CCID_secure(ULONG slot, UX_DEVICE_CLASS_CCID_MESSAGES *io_msg);
UINT USBD_CCID_mechanical(ULONG slot, UX_DEVICE_CLASS_CCID_MESSAGES *io_msg);
UINT USBD_CCID_abort(ULONG slot, UX_DEVICE_CLASS_CCID_MESSAGES *io_msg);
UINT USBD_CCID_set_data_rate_and_clock_frequency(ULONG slot, UX_DEVICE_CLASS_CCID_MESSAGES *io_msg);

/* USER CODE BEGIN EFP */
VOID usbx_ccid_thread_entry(ULONG thread_input);
VOID usbx_ccid_apdu_process(CCID_APDU_Command *, CCID_APDU_Response *);

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */

#ifdef __cplusplus
}
#endif
#endif  /* __UX_DEVICE_CCID_H__ */
