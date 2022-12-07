/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    ux_device_ccid.c
  * @author  MCD Application Team
  * @brief   USBX Device CCID applicative source file
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
#include "ux_device_ccid.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
static union CCID_PROTOCOL_DATA
{
  struct
  {
    UCHAR bmFindexDindex;
    UCHAR bmTCCKST0;
    UCHAR bGuardTimeT0;
    UCHAR bWaitingIntegerT0;
    UCHAR bClockStop;
  } t0;
  struct
  {
    UCHAR bmFindexDindex;
    UCHAR bmTCCKST1;
    UCHAR bGuardTimeT1;
    UCHAR bmWaitingIntegersT1;
    UCHAR bClockStop;
    UCHAR bIFSC;
    UCHAR bNadValue;
  } t1;
  UCHAR bytes[8];
}usbx_ccid_protocol_data;

static struct _CCID_CONTROLS
{
  ULONG   inserted:1;
  ULONG   time_extension:1;
  ULONG   read_binary_pattern:8;
} ccid_controls;

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* Define CCID device command handle */
UX_DEVICE_CLASS_CCID_HANDLES USBD_CCID_Handles = {
  /* USER CODE BEGIN USBD_CCID_Handles */

  USBD_CCID_icc_power_on,
  USBD_CCID_icc_power_off,
  USBD_CCID_get_slot_status,
  USBD_CCID_xfr_block,
  USBD_CCID_get_parameters,
  USBD_CCID_reset_parameters,
  USBD_CCID_set_parameters,
  USBD_CCID_escape,
  USBD_CCID_icc_clock,
  USBD_CCID_t0_apdu,
  UX_NULL, /*secure not supported*/
  UX_NULL, /*mechanical not supported*/
  USBD_CCID_abort,
  USBD_CCID_set_data_rate_and_clock_frequency,

  /* USER CODE END USBD_CCID_Handles */
};

ULONG USBD_CCID_Clocks[USBD_CCID_N_CLOCKS] = {
  /* USER CODE BEGIN USBD_CCID_Clocks */
  USBD_CCID_DEFAULT_CLOCK_FREQ
  /* USER CODE BEGIN USBD_CCID_Clocks */
};

ULONG USBD_CCID_DataRates[USBD_CCID_N_DATA_RATES] = {
  /* USER CODE BEGIN USBD_CCID_DataRates */
  USBD_CCID_DEFAULT_DATA_RATE
  /* USER CODE BEGIN USBD_CCID_DataRates */
};

/* USER CODE BEGIN PV */
UX_DEVICE_CLASS_CCID *ccid;
UCHAR usbx_ccid_protocol = 0;
static struct _CCID_CONTROLS ccid_controls = {1, 0, 0x00};

/* the Answer to Reset T0 */
static const UCHAR usbx_ccid_atr_t0[] =
{
  0x3B, 0xBE, 0x95, 0x00, 0x00, 0x41, 0x03, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x90, 0x00
};

/* the Card Hold Verification CHV */
static UCHAR CHV[8] = {0};

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  USBD_CCID_Activate
  *         This function is called while inserting CCID device.
  * @param  ccid_instance: Pointer to the ccid class instance.
  * @retval none
  */
VOID USBD_CCID_Activate(VOID *ccid_instance)
{
  /* USER CODE BEGIN USBD_CCID_Activate */

  /* Save the CCID instance */
  ccid = (UX_DEVICE_CLASS_CCID *)ccid_instance;

  /* USER CODE END USBD_CCID_Activate */

  return;
}

/**
  * @brief  USBD_CCID_Deactivate
  *         This function is called while extracting CCID device.
  * @param  ccid_instance: Pointer to the ccid class instance.
  * @retval none
  */
VOID USBD_CCID_Deactivate(VOID *ccid_instance)
{
  /* USER CODE BEGIN USBD_CCID_Deactivate */

  /* Reset the ccid instance */
  if ((VOID *)ccid == ccid_instance)
  {
    ccid = UX_NULL;
  }

  /* USER CODE END USBD_CCID_Deactivate */

  return;
}

/**
  * @brief  USBD_CCID_icc_power_on
  *         Activate the card slot and get the ATR from ICC
  * @param  slot: card slot
  * @param  io_msg: CCID message struct
  * @retval status
  */
UINT USBD_CCID_icc_power_on(ULONG slot, UX_DEVICE_CLASS_CCID_MESSAGES *io_msg)
{
  UINT status = UX_SUCCESS;

  /* USER CODE BEGIN USBD_CCID_icc_power_on */

  UX_PARAMETER_NOT_USED(slot);
  UX_DEVICE_CLASS_CCID_RDR_TO_PC_DATA_BLOCK_HEADER *data_block;
  UCHAR *rsp_data;

  /* Access to data */
  data_block = (UX_DEVICE_CLASS_CCID_RDR_TO_PC_DATA_BLOCK_HEADER *)io_msg -> ux_device_class_ccid_messages_rdr_to_pc;

  /* Access to response data */
  rsp_data = io_msg -> ux_device_class_ccid_messages_rdr_to_pc + UX_DEVICE_CLASS_CCID_MESSAGE_HEADER_LENGTH;

  /* Response ATR */
  ux_utility_memory_copy(rsp_data, (void *)usbx_ccid_atr_t0,
                         sizeof(usbx_ccid_atr_t0));

  /* Update bStatus, bError */
  data_block -> bStatus = UX_DEVICE_CLASS_CCID_ICC_ACTIVE;

  /* Update data length : Reply message header */
  UX_DEVICE_CLASS_CCID_MESSAGE_LENGTH_SET(data_block, sizeof(usbx_ccid_atr_t0));
  io_msg -> ux_device_class_ccid_messages_rdr_to_pc_length = sizeof(usbx_ccid_atr_t0) + 10;

  /* USER CODE END USBD_CCID_icc_power_on */

  return status;
}

/**
  * @brief  USBD_CCID_icc_power_off
  *         Deactivate the card slot
  * @param  slot: card slot
  * @param  io_msg: CCID message struct
  * @retval status
  */
UINT USBD_CCID_icc_power_off(ULONG slot, UX_DEVICE_CLASS_CCID_MESSAGES *io_msg)
{
  UINT status = UX_SUCCESS;

  /* USER CODE BEGIN USBD_CCID_icc_power_off */

  UX_PARAMETER_NOT_USED(slot);
  UX_DEVICE_CLASS_CCID_RDR_TO_PC_SLOT_STATUS_HEADER *rsp;

  /* Access to data */
  rsp = (UX_DEVICE_CLASS_CCID_RDR_TO_PC_SLOT_STATUS_HEADER *)io_msg -> ux_device_class_ccid_messages_rdr_to_pc;

  /* Update bStatus, bError */
  rsp -> bStatus = UX_DEVICE_CLASS_CCID_ICC_INACTIVE;

  /* USER CODE END USBD_CCID_icc_power_off */

  return status;
}

/**
  * @brief  USBD_CCID_get_slot_status
  *         Get current status of the slot
  * @param  slot: card slot
  * @param  io_msg: CCID message struct
  * @retval status
  */
UINT USBD_CCID_get_slot_status(ULONG slot, UX_DEVICE_CLASS_CCID_MESSAGES *io_msg)
{
  UINT status = UX_SUCCESS;

  /* USER CODE BEGIN USBD_CCID_get_slot_status */
  UX_PARAMETER_NOT_USED(slot);
  UX_PARAMETER_NOT_USED(io_msg);
  /* USER CODE END USBD_CCID_get_slot_status */

  return status;
}

/**
  * @brief  USBD_CCID_xfr_block
  *         Transfer data block to the ICC
  * @param  slot: card slot
  * @param  io_msg: CCID message struct
  * @retval status
  */
UINT USBD_CCID_xfr_block(ULONG slot, UX_DEVICE_CLASS_CCID_MESSAGES *io_msg)
{
  UINT status = UX_SUCCESS;

  /* USER CODE BEGIN USBD_CCID_xfr_block */

  UX_DEVICE_CLASS_CCID_PC_TO_RDR_XFR_BLOCK_HEADER *cmd;
  UX_DEVICE_CLASS_CCID_RDR_TO_PC_DATA_BLOCK_HEADER *rsp;
  UCHAR *cmd_data;
  UCHAR *rsp_data;
  CCID_APDU_Command apdu_cmd;
  CCID_APDU_Response apdu_rsp;

  /* Access to command */
  cmd = (UX_DEVICE_CLASS_CCID_PC_TO_RDR_XFR_BLOCK_HEADER *)io_msg -> ux_device_class_ccid_messages_pc_to_rdr;

  /* Access to data */
  rsp = (UX_DEVICE_CLASS_CCID_RDR_TO_PC_DATA_BLOCK_HEADER *)io_msg -> ux_device_class_ccid_messages_rdr_to_pc;

  /* Access to data buffers */
  cmd_data = (UCHAR *)io_msg -> ux_device_class_ccid_messages_pc_to_rdr + UX_DEVICE_CLASS_CCID_MESSAGE_HEADER_LENGTH;
  rsp_data = (UCHAR *)io_msg -> ux_device_class_ccid_messages_rdr_to_pc + UX_DEVICE_CLASS_CCID_MESSAGE_HEADER_LENGTH;

  /* Time extension */
  if (ccid_controls.time_extension)
  {
    ux_device_class_ccid_time_extension(ccid, slot, 10);
    ccid_controls.time_extension = 0;
    tx_thread_sleep(10);
  }

  /* Handle data */
  if (usbx_ccid_protocol == 0)
  {
    apdu_cmd.buffer = cmd_data;
    apdu_cmd.length = UX_DEVICE_CLASS_CCID_MESSAGE_LENGTH_GET(cmd);
    apdu_cmd.CLA    = cmd_data[0];
    apdu_cmd.INS    = cmd_data[1];
    apdu_cmd.P1     = cmd_data[2];
    apdu_cmd.P2     = cmd_data[3];
    apdu_cmd.Lc     = 0;
    apdu_cmd.Le     = UX_NULL;

    apdu_rsp.buffer = rsp_data;
    apdu_rsp.length = io_msg -> ux_device_class_ccid_messages_rdr_to_pc_length;
    apdu_rsp.Le     = 0;
    apdu_rsp.SW1    = 0;
    apdu_rsp.SW2    = 0;

    /* Manage APDU command and response */
    usbx_ccid_apdu_process(&apdu_cmd, &apdu_rsp);

    rsp_data[apdu_rsp.Le] = apdu_rsp.SW1;
    rsp_data[apdu_rsp.Le + 1] = apdu_rsp.SW2;
    UX_DEVICE_CLASS_CCID_MESSAGE_LENGTH_SET(rsp, apdu_rsp.Le + 2);
    io_msg -> ux_device_class_ccid_messages_rdr_to_pc_length = apdu_rsp.Le + 2 + 10;

    status = UX_SUCCESS;
  }

  if (usbx_ccid_protocol == 1)
  {
    /* Not supported : CCID event */
    rsp -> bStatus = UX_DEVICE_CLASS_CCID_SLOT_STATUS(0, 1);
    rsp -> bError = 0;

    /* In error case, no data */
    UX_DEVICE_CLASS_CCID_MESSAGE_LENGTH_SET(rsp, 0);
    io_msg -> ux_device_class_ccid_messages_rdr_to_pc_length = 10;

    status = UX_FUNCTION_NOT_SUPPORTED;
  }

  /* USER CODE END USBD_CCID_xfr_block */

  return status;
}

/**
  * @brief  USBD_CCID_get_parameters
  *         Get slot parameters
  * @param  slot: card slot
  * @param  io_msg: CCID message struct
  * @retval status
  */
UINT USBD_CCID_get_parameters(ULONG slot, UX_DEVICE_CLASS_CCID_MESSAGES *io_msg)
{
  UINT status = UX_SUCCESS;

  /* USER CODE BEGIN USBD_CCID_get_parameters */

  UX_PARAMETER_NOT_USED(slot);
  UX_DEVICE_CLASS_CCID_RDR_TO_PC_PARAMETERS_HEADER *rsp;
  UX_DEVICE_CLASS_CCID_RDR_TO_PC_PARAMETERS_T0 *t0;
  UX_DEVICE_CLASS_CCID_RDR_TO_PC_PARAMETERS_T1 *t1;

  /* Access to data */
  rsp = (UX_DEVICE_CLASS_CCID_RDR_TO_PC_PARAMETERS_HEADER *)io_msg -> ux_device_class_ccid_messages_rdr_to_pc;

  /* Parameters protocol */
  rsp->bProtocolNum = usbx_ccid_protocol;

  if (rsp -> bProtocolNum == 0)
  {
    t0 = (UX_DEVICE_CLASS_CCID_RDR_TO_PC_PARAMETERS_T0 *)rsp;
    ux_utility_memory_copy(&t0 -> bmFindexDindex, &usbx_ccid_protocol_data.t0, 5);

    /* Update data length */
    UX_DEVICE_CLASS_CCID_MESSAGE_LENGTH_SET(rsp, 5);
    io_msg -> ux_device_class_ccid_messages_rdr_to_pc_length = 5 + UX_DEVICE_CLASS_CCID_MESSAGE_HEADER_LENGTH;
  }

  if (rsp -> bProtocolNum == 1)
  {
    t1 = (UX_DEVICE_CLASS_CCID_RDR_TO_PC_PARAMETERS_T1 *)rsp;
    ux_utility_memory_copy(&t1 -> bmFindexDindex, &usbx_ccid_protocol_data.t1, 7);

    /* Update data length */
    UX_DEVICE_CLASS_CCID_MESSAGE_LENGTH_SET(rsp, 7);
    io_msg -> ux_device_class_ccid_messages_rdr_to_pc_length = 7 + UX_DEVICE_CLASS_CCID_MESSAGE_HEADER_LENGTH;
  }

  /* USER CODE END USBD_CCID_get_parameters */

  return status;
}

/**
  * @brief  USBD_CCID_reset_parameters
  *         Reset slot parameters to default value
  * @param  slot: card slot
  * @param  io_msg: CCID message struct
  * @retval status
  */
UINT USBD_CCID_reset_parameters(ULONG slot, UX_DEVICE_CLASS_CCID_MESSAGES *io_msg)
{
  UINT status = UX_SUCCESS;

  /* USER CODE BEGIN USBD_CCID_reset_parameters */

  UX_PARAMETER_NOT_USED(slot);
  UX_DEVICE_CLASS_CCID_RDR_TO_PC_PARAMETERS_HEADER *rsp;
  UX_DEVICE_CLASS_CCID_RDR_TO_PC_PARAMETERS_T0 *t0;
  UX_DEVICE_CLASS_CCID_RDR_TO_PC_PARAMETERS_T1 *t1;

  /* Access to response */
  rsp = (UX_DEVICE_CLASS_CCID_RDR_TO_PC_PARAMETERS_HEADER *)io_msg -> ux_device_class_ccid_messages_rdr_to_pc;

  /* Parameters protocol */
  rsp -> bProtocolNum = usbx_ccid_protocol;

  if (rsp -> bProtocolNum == 0)
  {
    t0 = (UX_DEVICE_CLASS_CCID_RDR_TO_PC_PARAMETERS_T0 *)rsp;
    usbx_ccid_protocol_data.t0.bmFindexDindex    = 0x11;
    usbx_ccid_protocol_data.t0.bmTCCKST0         = 0x00; /* 0x00, 0x02  */
    usbx_ccid_protocol_data.t0.bGuardTimeT0      = 1;
    usbx_ccid_protocol_data.t0.bWaitingIntegerT0 = 1;
    usbx_ccid_protocol_data.t0.bClockStop        = 0;
    ux_utility_memory_copy(&t0 -> bmFindexDindex, &usbx_ccid_protocol_data.t0, 5);

    /* Update data length */
    UX_DEVICE_CLASS_CCID_MESSAGE_LENGTH_SET(rsp, 5);
    io_msg -> ux_device_class_ccid_messages_rdr_to_pc_length = 5 + UX_DEVICE_CLASS_CCID_MESSAGE_HEADER_LENGTH;
  }

  if (rsp -> bProtocolNum == 1)
  {
    t1 = (UX_DEVICE_CLASS_CCID_RDR_TO_PC_PARAMETERS_T1 *)rsp;
    usbx_ccid_protocol_data.t1.bmFindexDindex      = 0x11;
    usbx_ccid_protocol_data.t1.bmTCCKST1           = 0x10; /* 0x10, 0x11, 0x12, 0x13  */
    usbx_ccid_protocol_data.t1.bGuardTimeT1        = 1;
    usbx_ccid_protocol_data.t1.bmWaitingIntegersT1 = 0x12;
    usbx_ccid_protocol_data.t1.bClockStop          = 0;
    usbx_ccid_protocol_data.t1.bIFSC               = 0;
    usbx_ccid_protocol_data.t1.bNadValue           = 0;
    ux_utility_memory_copy(&t1 -> bmFindexDindex, &usbx_ccid_protocol_data.t1, 7);

    /* Update data length */
    UX_DEVICE_CLASS_CCID_MESSAGE_LENGTH_SET(rsp, 7);
    io_msg -> ux_device_class_ccid_messages_rdr_to_pc_length = 7 + UX_DEVICE_CLASS_CCID_MESSAGE_HEADER_LENGTH;
  }

  /* USER CODE END USBD_CCID_reset_parameters */

  return status;
}

/**
  * @brief  USBD_CCID_set_parameters
  *         Set slot parameters
  * @param  slot: card slot
  * @param  io_msg: CCID message struct
  * @retval status
  */
UINT USBD_CCID_set_parameters(ULONG slot, UX_DEVICE_CLASS_CCID_MESSAGES *io_msg)
{
  UINT status = UX_SUCCESS;

  /* USER CODE BEGIN USBD_CCID_set_parameters */

  UX_PARAMETER_NOT_USED(slot);
  UX_DEVICE_CLASS_CCID_PC_TO_RDR_SET_PARAMETERS_HEADER *cmd;
  UX_DEVICE_CLASS_CCID_RDR_TO_PC_PARAMETERS_HEADER *rsp;
  UX_DEVICE_CLASS_CCID_PC_TO_RDR_SET_PARAMETERS_T0 *cmd_t0;
  UX_DEVICE_CLASS_CCID_PC_TO_RDR_SET_PARAMETERS_T1 *cmd_t1;

  /* Access to command */
  cmd = (UX_DEVICE_CLASS_CCID_PC_TO_RDR_SET_PARAMETERS_HEADER *)io_msg -> ux_device_class_ccid_messages_pc_to_rdr;

  /* Access to response */
  rsp = (UX_DEVICE_CLASS_CCID_RDR_TO_PC_PARAMETERS_HEADER *)io_msg -> ux_device_class_ccid_messages_rdr_to_pc;

  if (cmd -> bProtocolNum == 0)
  {
    usbx_ccid_protocol = 0;
    cmd_t0 = (UX_DEVICE_CLASS_CCID_PC_TO_RDR_SET_PARAMETERS_T0 *)cmd;
    ux_utility_memory_copy(&usbx_ccid_protocol_data.t0, &cmd_t0 -> bmFindexDindex, 5);

    /* Update data length.  */
    UX_DEVICE_CLASS_CCID_MESSAGE_LENGTH_SET(rsp, 5);
    io_msg -> ux_device_class_ccid_messages_rdr_to_pc_length = 5 + UX_DEVICE_CLASS_CCID_MESSAGE_HEADER_LENGTH;

    status = UX_SUCCESS;
  }

  else if (cmd -> bProtocolNum == 1)
  {
    usbx_ccid_protocol = 1;
    cmd_t1 = (UX_DEVICE_CLASS_CCID_PC_TO_RDR_SET_PARAMETERS_T1 *)cmd;
    ux_utility_memory_copy(&usbx_ccid_protocol_data.t1, &cmd_t1 -> bmFindexDindex, 7);

    /* Update data length */
    UX_DEVICE_CLASS_CCID_MESSAGE_LENGTH_SET(rsp, 7);
    io_msg -> ux_device_class_ccid_messages_rdr_to_pc_length = 7 + UX_DEVICE_CLASS_CCID_MESSAGE_HEADER_LENGTH;

    status = UX_SUCCESS;
  }
  else
  {
    /* Response : invalid protocol number */
    rsp -> bError = 7;
    rsp -> bStatus |= UX_DEVICE_CLASS_CCID_SLOT_STATUS_CMD_FAILED;

    /* In error case, no data */
    UX_DEVICE_CLASS_CCID_MESSAGE_LENGTH_SET(rsp, 0);
    io_msg -> ux_device_class_ccid_messages_rdr_to_pc_length = UX_DEVICE_CLASS_CCID_MESSAGE_HEADER_LENGTH;

    status = UX_ERROR;
  }

  /* USER CODE END USBD_CCID_set_parameters */

  return status;
}

/**
  * @brief  USBD_CCID_escape
  *         Define and access extended features
  * @param  slot: card slot
  * @param  io_msg: CCID message struct
  * @retval status
  */
UINT USBD_CCID_escape(ULONG slot, UX_DEVICE_CLASS_CCID_MESSAGES *io_msg)
{
  UINT status = UX_SUCCESS;

  /* USER CODE BEGIN USBD_CCID_escape */

  UX_PARAMETER_NOT_USED(slot);
  UX_DEVICE_CLASS_CCID_PC_TO_RDR_ESCAPE_HEADER *cmd;
  UX_DEVICE_CLASS_CCID_RDR_TO_PC_ESCAPE_HEADER *rsp;
  UCHAR *cmd_data;
  UCHAR *rsp_data;
  ULONG length, i;

  /* Access to command */
  cmd = (UX_DEVICE_CLASS_CCID_PC_TO_RDR_ESCAPE_HEADER *)io_msg -> ux_device_class_ccid_messages_pc_to_rdr;

  /* Access to data */
  rsp = (UX_DEVICE_CLASS_CCID_RDR_TO_PC_ESCAPE_HEADER *)io_msg -> ux_device_class_ccid_messages_rdr_to_pc;
  cmd_data = io_msg -> ux_device_class_ccid_messages_pc_to_rdr + UX_DEVICE_CLASS_CCID_MESSAGE_HEADER_LENGTH;
  rsp_data = io_msg -> ux_device_class_ccid_messages_rdr_to_pc + UX_DEVICE_CLASS_CCID_MESSAGE_HEADER_LENGTH;

  /* Echo back data as example */
  length = UX_DEVICE_CLASS_CCID_MESSAGE_LENGTH_GET(cmd);

  for (i = 0; i < length; i ++)
  {
    rsp_data[i] = cmd_data[i];
  }

  /* Update data length */
  UX_DEVICE_CLASS_CCID_MESSAGE_LENGTH_SET(rsp, length);
  io_msg -> ux_device_class_ccid_messages_rdr_to_pc_length = length + UX_DEVICE_CLASS_CCID_MESSAGE_HEADER_LENGTH;

  /* USER CODE END USBD_CCID_escape */

  return status;
}

/**
  * @brief  USBD_CCID_icc_clock
  *         Stop or restart the clock
  * @param  slot: card slot
  * @param  io_msg: CCID message struct
  * @retval status
  */
UINT USBD_CCID_icc_clock(ULONG slot, UX_DEVICE_CLASS_CCID_MESSAGES *io_msg)
{
  UINT status = UX_SUCCESS;

  /* USER CODE BEGIN USBD_CCID_icc_clock */

  UX_PARAMETER_NOT_USED(slot);
  UX_DEVICE_CLASS_CCID_PC_TO_RDR_ICC_CLOCK_HEADER *cmd;
  UX_DEVICE_CLASS_CCID_RDR_TO_PC_SLOT_STATUS_HEADER *rsp;

  /* Access to command */
  cmd = (UX_DEVICE_CLASS_CCID_PC_TO_RDR_ICC_CLOCK_HEADER *)io_msg -> ux_device_class_ccid_messages_pc_to_rdr;

  /* Access to data */
  rsp = (UX_DEVICE_CLASS_CCID_RDR_TO_PC_SLOT_STATUS_HEADER *)io_msg -> ux_device_class_ccid_messages_rdr_to_pc;

  /* Clock command */
  if (cmd -> bClockCommand == 0)
  {
    status = UX_SUCCESS;
  }
  else if (cmd -> bClockCommand == 1)
  {
    /* Response : bClockCommand error */
    rsp -> bStatus |= UX_DEVICE_CLASS_CCID_SLOT_STATUS_CMD_FAILED;
    rsp -> bError = 7;
    status = UX_ERROR;
  }
  else
  {
    /* Response : Command not supported : CCID event */
    rsp -> bStatus = UX_DEVICE_CLASS_CCID_SLOT_STATUS(0, 1);
    rsp -> bError = 0;

    status = UX_FUNCTION_NOT_SUPPORTED;
  }

  /* USER CODE END USBD_CCID_icc_clock */

  return status;
}

/**
  * @brief  USBD_CCID_t0_apdu
  *         Changes parameters used for T=0 protocol APDU messages transportation
  * @param  slot: card slot
  * @param  io_msg: CCID message struct
  * @retval status
  */
UINT USBD_CCID_t0_apdu(ULONG slot, UX_DEVICE_CLASS_CCID_MESSAGES *io_msg)
{
  UINT status = UX_SUCCESS;

  /* USER CODE BEGIN USBD_CCID_t0_apdu */

  UX_PARAMETER_NOT_USED(slot);
  UX_DEVICE_CLASS_CCID_PC_TO_RDR_T0_APDU_HEADER *cmd;
  UX_DEVICE_CLASS_CCID_RDR_TO_PC_SLOT_STATUS_HEADER *rsp;

  /* Access to command */
  cmd = (UX_DEVICE_CLASS_CCID_PC_TO_RDR_T0_APDU_HEADER *)io_msg -> ux_device_class_ccid_messages_pc_to_rdr;

  /* Access to data */
  rsp = (UX_DEVICE_CLASS_CCID_RDR_TO_PC_SLOT_STATUS_HEADER *)io_msg -> ux_device_class_ccid_messages_rdr_to_pc;

  /* Accept no change case */
  if (cmd -> bmChanges == 0)
  {
    status = UX_SUCCESS;
  }
  else
  {
    /* Response : Command Not Supported : CCID event */
    rsp -> bStatus = UX_DEVICE_CLASS_CCID_SLOT_STATUS(0, 1);
    rsp -> bError = 0;

    status = UX_FUNCTION_NOT_SUPPORTED;
  }

  /* USER CODE END USBD_CCID_t0_apdu */

  return status;
}

/**
  * @brief  USBD_CCID_secure
  *         Allow entering the PIN for verification or modification
  * @param  slot: card slot
  * @param  io_msg: CCID message struct
  * @retval status
  */
UINT USBD_CCID_secure(ULONG slot, UX_DEVICE_CLASS_CCID_MESSAGES *io_msg)
{
  UINT status = UX_SUCCESS;

  /* USER CODE BEGIN USBD_CCID_secure */
  UX_PARAMETER_NOT_USED(slot);
  UX_PARAMETER_NOT_USED(io_msg);
  /* USER CODE END USBD_CCID_secure */

  return status;
}

/**
  * @brief  USBD_CCID_mechanical
  *         Manage motorized type CCID functionality
  * @param  slot: card slot
  * @param  io_msg: CCID message struct
  * @retval status
  */
UINT USBD_CCID_mechanical(ULONG slot, UX_DEVICE_CLASS_CCID_MESSAGES *io_msg)
{
  UINT status = UX_SUCCESS;

  /* USER CODE BEGIN USBD_CCID_mechanical */
  UX_PARAMETER_NOT_USED(slot);
  UX_PARAMETER_NOT_USED(io_msg);
  /* USER CODE END USBD_CCID_mechanical */

  return status;
}

/**
  * @brief  USBD_CCID_abort
  *         Tell the CCID to stop any current transfer at the specified slot
  * @param  slot: card slot
  * @param  io_msg: CCID message struct
  * @retval status
  */
UINT USBD_CCID_abort(ULONG slot, UX_DEVICE_CLASS_CCID_MESSAGES *io_msg)
{
  UINT status = UX_SUCCESS;

  /* USER CODE BEGIN USBD_CCID_abort */

  UX_PARAMETER_NOT_USED(slot);

  /* Control request Abort */
  if (io_msg == UX_NULL)
  {
    status = UX_SUCCESS;
  }
  else
  {
    status = UX_ERROR;
  }

  /* USER CODE END USBD_CCID_abort */

  return status;
}

/**
  * @brief  USBD_CCID_set_data_rate_and_clock_frequency
  *         Set the data rate and clock frequency of a specific slot
  * @param  slot: card slot
  * @param  io_msg: CCID message struct
  * @retval status
  */
UINT USBD_CCID_set_data_rate_and_clock_frequency(ULONG slot, UX_DEVICE_CLASS_CCID_MESSAGES *io_msg)
{
  UINT status = UX_SUCCESS;

  /* USER CODE BEGIN USBD_CCID_set_data_rate_and_clock_frequency */

  UX_PARAMETER_NOT_USED(slot);
  UX_DEVICE_CLASS_CCID_PC_TO_RDR_SET_DATA_RATE_AND_CLOCK_FREQUENCY *cmd;
  UX_DEVICE_CLASS_CCID_RDR_TO_PC_DATA_RATE_AND_CLOCK_FREQUENCY *rsp;
  ULONG data_rate;
  ULONG clock;

  /* Access to command */
  cmd = (UX_DEVICE_CLASS_CCID_PC_TO_RDR_SET_DATA_RATE_AND_CLOCK_FREQUENCY *)io_msg -> ux_device_class_ccid_messages_pc_to_rdr;

  /* Access to data */
  rsp = (UX_DEVICE_CLASS_CCID_RDR_TO_PC_DATA_RATE_AND_CLOCK_FREQUENCY *)io_msg -> ux_device_class_ccid_messages_rdr_to_pc;

  data_rate = UX_DEVICE_CLASS_CCID_PC_TO_RDR_CLOCK_FREQUENCY_GET(cmd);
  clock = UX_DEVICE_CLASS_CCID_PC_TO_RDR_DATA_RATE_GET(cmd);

  /* Accept default values */
  if (data_rate == USBD_CCID_DEFAULT_DATA_RATE && clock == USBD_CCID_DEFAULT_CLOCK_FREQ)
  {
    status = UX_SUCCESS;
  }
  else
  {
    /* Response : Command Not Supported : CCID event */
    rsp -> bStatus = UX_DEVICE_CLASS_CCID_SLOT_STATUS(0, 1);
    rsp -> bError = 0;

    /* In error case, no data */
    UX_DEVICE_CLASS_CCID_MESSAGE_LENGTH_SET(rsp, 0);
    io_msg -> ux_device_class_ccid_messages_rdr_to_pc_length = UX_DEVICE_CLASS_CCID_MESSAGE_HEADER_LENGTH;

    status = UX_FUNCTION_NOT_SUPPORTED;
  }

  /* USER CODE END USBD_CCID_set_data_rate_and_clock_frequency */

  return status;
}

/* USER CODE BEGIN 1 */

/**
  * @brief  Function implementing usbx_ccid_thread_entry.
  * @param  thread_input: Not used
  * @retval none
  */
VOID usbx_ccid_thread_entry(ULONG thread_input)
{
  UX_SLAVE_DEVICE *device;
  ULONG card_inserted = 0;

  /* Get the pointer to the device */
  device = &_ux_system_slave->ux_system_slave_device;

  while (1)
  {
    /* Check if the device state already configured
    and simulate card insertion and removal */
    if ((device->ux_slave_device_state == UX_DEVICE_CONFIGURED) && (ccid != UX_NULL) &&
        (card_inserted != ccid_controls.inserted))
    {
      card_inserted = ccid_controls.inserted;

      if (card_inserted)
      {
        /* Card insertion */
        ux_device_class_ccid_icc_insert(ccid, 0, 0);
      }
      else
      {
        /* Card removal */
        ux_device_class_ccid_icc_remove(ccid, 0);
      }
    }
    else
    {
      /* sleep for 10ms */
      tx_thread_sleep(MS_TO_TICK(10));
    }
  }
}

/* APDU related API */
/**
  * @brief  usbx_ccid_apdu_enable_chv
  *         Enable CHV
  * @param  apdu_cmd: pointer to apdu command
  * @param  apdu_rsp: pointer to apdu response
  * @retval none
  */
static VOID usbx_ccid_apdu_enable_chv(CCID_APDU_Command *apdu_cmd, CCID_APDU_Response *apdu_rsp)
{
  UINT i;

  /* Verify CHV */
  apdu_cmd->Lc = apdu_cmd->buffer[4];

  /* Check length */
  if (apdu_cmd->Lc != 8)
  {
    /* PIN Not Enabled */
    apdu_rsp->SW1 = 0x88;
    apdu_rsp->SW2 = 0x88;
    return;
  }

  for (i = 0; i < apdu_cmd->Lc; i ++)
  {
    CHV[i] = apdu_cmd->buffer[5 + i];
  }

  /* OK */
  apdu_rsp->SW1 = 0x90;
  apdu_rsp->SW2 = 0x00;
}

/**
  * @brief  usbx_ccid_apdu_select
  *         Select a file about the logical card model
  * @param  apdu_cmd: pointer to apdu command
  * @param  apdu_rsp: pointer to apdu response
  * @retval none
  */
static VOID usbx_ccid_apdu_select(CCID_APDU_Command *apdu_cmd, CCID_APDU_Response *apdu_rsp)
{
  /* Update actual length */
  apdu_rsp->Le = 0;

  /* Fill response */
  apdu_rsp->SW1 = 0x90;
  apdu_rsp->SW2 = 0x00;
}

/**
  * @brief  usbx_ccid_apdu_verify_change
  *         Change to the CHV or VERIFY CHV
  * @param  apdu_cmd: pointer to apdu command
  * @param  apdu_rsp: pointer to apdu response
  * @retval none
  */
static VOID  usbx_ccid_apdu_verify_change(CCID_APDU_Command *apdu_cmd, CCID_APDU_Response *apdu_rsp)
{
  UINT i;
  UCHAR *chv;

  /* Verify CHV */
  apdu_cmd->Lc = apdu_cmd->buffer[4];

  /* Check length */
  if (apdu_cmd->Lc != 8 && apdu_cmd->Lc != 16)
  {
    /* PIN Not Verified */
    apdu_rsp->SW1 = 0x63;
    apdu_rsp->SW2 = 0xC0;
    return;
  }

  /* Do verify if:
  - command verify
  - command change and Lc is 16 */
  if (apdu_cmd->Lc == 16 || apdu_cmd->INS == 0x20)
  {
    for (i = 0; i < 8; i ++)
    {
      if (CHV[i] != apdu_cmd->buffer[5 + i])
      {
        /* PIN Not Verified */
        apdu_rsp->SW1 = 0x63;
        apdu_rsp->SW2 = 0xC0;
        return;
      }
    }
  }

  /* Do change if:
  - command change */
  if (apdu_cmd->INS == 0x24)
  {
    chv = apdu_cmd->buffer + 5;
    if (apdu_cmd->Lc == 16)
    {
      chv += 8;
    }
    ux_utility_memory_copy(CHV, chv, 8);
  }

  /* Fill response */
  apdu_rsp->SW1 = 0x90;
  apdu_rsp->SW2 = 0x00;
}

/**
  * @brief  usbx_ccid_apdu_read_data
  *         Read data from a transparent EF
  * @param  apdu_cmd: pointer to apdu command
  * @param  apdu_rsp: pointer to apdu response
  * @retval none
  */
VOID  usbx_ccid_apdu_read_data(CCID_APDU_Command *apdu_cmd, CCID_APDU_Response *apdu_rsp)
{
  UINT i;
  USHORT length;

  if (apdu_cmd->length > 4)
  {
    /* Le is available */
    apdu_cmd->Le = &apdu_cmd->buffer[4];
  }
  else
  {
    /* Reset Le */
    *apdu_cmd->Le = 0;
  }

  /* Get the number of data expected in response */
  length = (*(apdu_cmd->Le)) ? (*(apdu_cmd->Le)) : 256;

  /* Fill response data */
  for (i = 0; i < length; i ++)
  {
    apdu_rsp->buffer[i] = ccid_controls.read_binary_pattern + i;
  }

  /* Update actual length */
  apdu_rsp->Le = length;

  /* Fill response */
  apdu_rsp->SW1 = 0x90;
  apdu_rsp->SW2 = 0x00;
}

/**
  * @brief  usbx_ccid_apdu_process
  *         Manage APDU command and response
  * @param  apdu_cmd: pointer to apdu command
  * @param  apdu_rsp: pointer to apdu response
  * @retval none
  */
VOID  usbx_ccid_apdu_process(CCID_APDU_Command *apdu_cmd, CCID_APDU_Response *apdu_rsp)
{
  switch (apdu_cmd->INS)
  {
    case 0x28: /* ENABLE CHV1  A0 28 00 01 08 CHV1 value */
      usbx_ccid_apdu_enable_chv(apdu_cmd, apdu_rsp);
      return;

    case 0xA4: /* SELECT  A0 A4 00 00 02 File ID */
      usbx_ccid_apdu_select(apdu_cmd, apdu_rsp);
      return;

    case 0x20: /* VERIFY  A0 20 00 xx 08 CHV Value */
    case 0x24: /* CHANGE  A0 24 00 xx 10 Old CHV, New CHV */
      usbx_ccid_apdu_verify_change(apdu_cmd, apdu_rsp);
      return;

    case 0xC0: /* GET RESPONSE A0 C0 00 00    xx */
    case 0xB0: /* READ BINARY  A0 B0 xx xx    xx */
      usbx_ccid_apdu_read_data(apdu_cmd, apdu_rsp);
      return;

    default:
      /* Command not supported */
      apdu_rsp->Le = 0;
      apdu_rsp->SW1 = 0x69;
      apdu_rsp->SW2 = 0x99;
      break;
  }
}

/* USER CODE END 1 */
