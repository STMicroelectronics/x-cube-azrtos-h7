/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    ux_device_video.c
  * @author  MCD Application Team
  * @brief   USBX Device Video applicative source file
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
#include "ux_device_video.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "stream1.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
#define UVC_PLAY_STATUS_STOP       0x00U
#define UVC_PLAY_STATUS_READY      0x01U
#define UVC_PLAY_STATUS_STREAMING  0x02U
#define USBD_VIDEO_IMAGE_LAPS      80U
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */
uint32_t payload_count;
ULONG uvc_state;
UX_DEVICE_CLASS_VIDEO *video;
UX_DEVICE_CLASS_VIDEO_STREAM *stream_write;

UCHAR video_frame_buffer[1024];

/* Video Probe data structure */
static USBD_VideoControlTypeDef video_Probe_Control =
{
  .bmHint = 0x0000U,
  .bFormatIndex = 0x01U,
  .bFrameIndex = 0x01U,
  .dwFrameInterval = UVC_INTERVAL(UVC_CAM_FPS_FS),
  .wKeyFrameRate = 0x0000U,
  .wPFrameRate = 0x0000U,
  .wCompQuality = 0x0000U,
  .wCompWindowSize = 0x0000U,
  .wDelay = 0x0000U,
  .dwMaxVideoFrameSize = 0x0000U,
  .dwMaxPayloadTransferSize = 0x00000000U,
  .dwClockFrequency = 0x00000000U,
  .bmFramingInfo = 0x00U,
  .bPreferedVersion = 0x00U,
  .bMinVersion = 0x00U,
  .bMaxVersion = 0x00U,
};

/* Video Commit data structure */
static USBD_VideoControlTypeDef video_Commit_Control =
{
  .bmHint = 0x0000U,
  .bFormatIndex = 0x01U,
  .bFrameIndex = 0x01U,
  .dwFrameInterval = UVC_INTERVAL(UVC_CAM_FPS_FS),
  .wKeyFrameRate = 0x0000U,
  .wPFrameRate = 0x0000U,
  .wCompQuality = 0x0000U,
  .wCompWindowSize = 0x0000U,
  .wDelay = 0x0000U,
  .dwMaxVideoFrameSize = 0x0000U,
  .dwMaxPayloadTransferSize = 0x00000000U,
  .dwClockFrequency = 0x00000000U,
  .bmFramingInfo = 0x00U,
  .bPreferedVersion = 0x00U,
  .bMinVersion = 0x00U,
  .bMaxVersion = 0x00U,
};
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */
static VOID video_write_payload(UX_DEVICE_CLASS_VIDEO_STREAM *stream);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  USBD_VIDEO_Activate
  *         This function is called when insertion of a Video device.
  * @param  video_instance: Pointer to the video class instance.
  * @retval none
  */
VOID USBD_VIDEO_Activate(VOID *video_instance)
{
  /* USER CODE BEGIN USBD_VIDEO_Activate */

  /* Save the video instance */
  video = (UX_DEVICE_CLASS_VIDEO*) video_instance;

  /* Get the streams instances */
  ux_device_class_video_stream_get(video, 0, &stream_write);

  /* USER CODE END USBD_VIDEO_Activate */

  return;
}

/**
  * @brief  USBD_VIDEO_Deactivate
  *         This function is called when extraction of a Video device.
  * @param  video_instance: Pointer to the video class instance.
  * @retval none
  */
VOID USBD_VIDEO_Deactivate(VOID *video_instance)
{
  /* USER CODE BEGIN USBD_VIDEO_Deactivate */
  UX_PARAMETER_NOT_USED(video_instance);

  /* Reset the video instance */
  video = UX_NULL;

  /* Reset the video streams */
  stream_write = UX_NULL;

  /* USER CODE END USBD_VIDEO_Deactivate */

  return;
}

/**
  * @brief  USBD_VIDEO_StreamChange
  *         This function is invoked to inform application that the
  *         alternate setting are changed.
  * @param  video_stream: Pointer to video class stream instance.
  * @param  alternate_setting: interface alternate setting.
  * @retval none
  */
VOID USBD_VIDEO_StreamChange(UX_DEVICE_CLASS_VIDEO_STREAM *video_stream,
                             ULONG alternate_setting)
{
  /* USER CODE BEGIN USBD_VIDEO_StreamChange */

  /* Stop video payload loop back if stream closed */
  if (alternate_setting == 0U)
  {
    /* Update State machine */
    uvc_state = UVC_PLAY_STATUS_STOP;

    img_count--;

    return;
  }

  /* Update State machine */
  uvc_state = UVC_PLAY_STATUS_READY;

  /* Write buffers until achieve threadshold */
  video_write_payload(video_stream);

  /* Start sending valid payloads in the Video class */
  ux_device_class_video_transmission_start(video_stream);

  /* USER CODE END USBD_VIDEO_StreamChange */

  return;
}

/**
  * @brief  USBD_VIDEO_StreamPayloadDone
  *         This function is invoked when stream data payload received.
  * @param  video_stream: Pointer to video class stream instance.
  * @param  length: transfer length.
  * @retval none
  */
VOID USBD_VIDEO_StreamPayloadDone(UX_DEVICE_CLASS_VIDEO_STREAM *video_stream,
                                  ULONG length)
{
  /* USER CODE BEGIN USBD_VIDEO_StreamPayloadDone */

  /* Check length is not NULL */
  if (length != 0U)
  {
    payload_count = 0;

    /* Update state machine */
    uvc_state = UVC_PLAY_STATUS_STREAMING;

    /* Prepare next payload */
    video_write_payload(video_stream);
  }
  else
  {
    payload_count++ ;
  }

  if (payload_count == USBD_VIDEO_IMAGE_LAPS)
  {
    /* Update state machine */
    uvc_state = UVC_PLAY_STATUS_READY;

    /* Prepare next payload */
    video_write_payload(video_stream);
  }
  /* USER CODE END USBD_VIDEO_StreamPayloadDone */

  return;
}

/**
  * @brief  USBD_VIDEO_StreamRequest
  *         This function is invoked to manage the UVC class requests.
  * @param  video_stream: Pointer to video class stream instance.
  * @param  transfer: Pointer to the transfer request.
  * @retval status
  */
UINT USBD_VIDEO_StreamRequest(UX_DEVICE_CLASS_VIDEO_STREAM *video_stream,
                              UX_SLAVE_TRANSFER *transfer)
{
   UINT status  = UX_SUCCESS;

  /* USER CODE BEGIN USBD_VIDEO_StreamRequest */
  UCHAR *data, bRequest;
  USHORT wValue_CS, wLength;

  /* Decode setup packet */
  bRequest = transfer -> ux_slave_transfer_request_setup[UX_SETUP_REQUEST];
  wValue_CS = transfer -> ux_slave_transfer_request_setup[UX_SETUP_VALUE + 1];
  wLength = ux_utility_short_get(transfer -> ux_slave_transfer_request_setup + UX_SETUP_LENGTH);
  data = transfer -> ux_slave_transfer_request_data_pointer;

  /* Check CS */
  switch(wValue_CS)
  {
    case UX_DEVICE_CLASS_VIDEO_VS_PROBE_CONTROL:

      switch(bRequest)
      {
        case UX_DEVICE_CLASS_VIDEO_SET_CUR:

          status = UX_SUCCESS;

          break;

        case UX_DEVICE_CLASS_VIDEO_GET_DEF:
        case UX_DEVICE_CLASS_VIDEO_GET_CUR:
        case UX_DEVICE_CLASS_VIDEO_GET_MIN:
        case UX_DEVICE_CLASS_VIDEO_GET_MAX:

          /* Update bPreferedVersion, bMinVersion and bMaxVersion which must be set only by Device */
          video_Probe_Control.bPreferedVersion = 0x00U;
          video_Probe_Control.bMinVersion = 0x00U;
          video_Probe_Control.bMaxVersion = 0x00U;
          video_Probe_Control.dwMaxVideoFrameSize = UVC_MAX_FRAME_SIZE;
          video_Probe_Control.dwClockFrequency = 0x02DC6C00U;

          /* Check device speed */
          if(_ux_system_slave->ux_system_slave_speed == UX_FULL_SPEED_DEVICE)
          {
            video_Probe_Control.dwFrameInterval = (UVC_INTERVAL(UVC_CAM_FPS_FS));
            video_Probe_Control.dwMaxPayloadTransferSize = USBD_VIDEO_EPIN_FS_MPS;
          }
          else
          {
            video_Probe_Control.dwFrameInterval = (UVC_INTERVAL(UVC_CAM_FPS_HS));
            video_Probe_Control.dwMaxPayloadTransferSize = USBD_VIDEO_EPIN_HS_MPS;
          }

          /* Copy data for transfer */
          ux_utility_memory_copy(data, (VOID *)&video_Probe_Control, sizeof(USBD_VideoControlTypeDef));

          /* Transfer request */
          status = ux_device_stack_transfer_request(transfer, UX_MIN(wLength, sizeof(USBD_VideoControlTypeDef)),
                                                    UX_MIN(wLength, sizeof(USBD_VideoControlTypeDef)));

          break;

        default:
          break;
      }
      break;

    case UX_DEVICE_CLASS_VIDEO_VS_COMMIT_CONTROL:

      /* Check request length */
      if (wLength < 26)
      {
        break;
      }

      switch(bRequest)
      {
        case UX_DEVICE_CLASS_VIDEO_SET_CUR:

          status = UX_SUCCESS;

          break;

        case UX_DEVICE_CLASS_VIDEO_GET_CUR:

          /* Check device speed */
          if(_ux_system_slave->ux_system_slave_speed == UX_FULL_SPEED_DEVICE)
          {
            video_Commit_Control.dwFrameInterval = (UVC_INTERVAL(UVC_CAM_FPS_FS));
            video_Commit_Control.dwMaxPayloadTransferSize = USBD_VIDEO_EPIN_FS_MPS;
          }
          else
          {
            video_Commit_Control.dwFrameInterval = (UVC_INTERVAL(UVC_CAM_FPS_HS));
            video_Commit_Control.dwMaxPayloadTransferSize = USBD_VIDEO_EPIN_HS_BINTERVAL;
          }

          /* Copy data for transfer */
          ux_utility_memory_copy(data, (VOID *)&video_Commit_Control, 26);

          /* Send transfer request */
          status = ux_device_stack_transfer_request(transfer,
                                                    UX_MIN(wLength, sizeof(USBD_VideoControlTypeDef)),
                                                    UX_MIN(wLength, sizeof(USBD_VideoControlTypeDef)));

          break;

        default:
          break;

      }

      break;

      default:
        status = UX_ERROR;
        break;
  }

  /* USER CODE END USBD_VIDEO_StreamRequest */

  return status;
}

/**
  * @brief  USBD_VIDEO_StreamGetMaxPayloadBufferSize
  *         Get video stream max payload buffer size.
  * @param  none
  * @retval max payload
  */
ULONG USBD_VIDEO_StreamGetMaxPayloadBufferSize(VOID)
{
  ULONG max_playload = 0U;

  /* USER CODE BEGIN USBD_VIDEO_StreamGetMaxPayloadBufferSize */

  max_playload = USBD_VIDEO_EPIN_FS_MPS;

  /* USER CODE END USBD_VIDEO_StreamGetMaxPayloadBufferSize */

  return max_playload;
}

/* USER CODE BEGIN 1 */

/**
  * @brief  video_write_payload
            Manage the UVC data packets.
  * @param  stream : Video class stream instance.
  * @retval none
  */
VOID video_write_payload(UX_DEVICE_CLASS_VIDEO_STREAM *stream)
{
  ULONG buffer_length;
  UCHAR *buffer;
  ULONG usbd_video_ep_mps = stream->ux_device_class_video_stream_endpoint->ux_slave_endpoint_descriptor.wMaxPacketSize;
  static ULONG length;
  static uint8_t packet_index = 0;
  const uint8_t *(*ImagePtr)= tImagesList;
  uint32_t packet_count = (tImagesSizes[img_count])/(( uint16_t)(usbd_video_ep_mps - 2));
  uint32_t packet_remainder =(tImagesSizes[img_count])%(( uint16_t)(usbd_video_ep_mps - 2));
  static uint8_t  payload_header[2] = {0x02U, 0x00U};
  static uint8_t *Pcktdata = video_frame_buffer;

  /* Get payload buffer */
  ux_device_class_video_write_payload_get(stream, &buffer, &buffer_length);

  /* Check UVC state*/
  switch(uvc_state)
  {
    case UVC_PLAY_STATUS_READY:

      length = 2U;

      /* Add the packet header */
      video_frame_buffer[0] = payload_header[0];
      video_frame_buffer[1] = payload_header[1];

      /* Copy video buffer in video frame buffer */
      ux_utility_memory_copy(buffer, video_frame_buffer, length);

      /* Commit payload buffer */
      ux_device_class_video_write_payload_commit(stream, length);

      break;

    case UVC_PLAY_STATUS_STREAMING:

      /* Reset video frame buffer */
      ux_utility_memory_set(video_frame_buffer, 0, usbd_video_ep_mps);

      /* Check if end of current image has been reached */
      if (packet_index < packet_count)
      {
        /* Set the current packet size */
        length = (uint16_t)usbd_video_ep_mps;

        /* Get the pointer to the next packet to be transmitted */
        Pcktdata = (uint8_t*)(*(ImagePtr + img_count) + packet_index * ((uint16_t)(usbd_video_ep_mps - 2U)));
      }
      else if(packet_index == packet_count)
      {
        /* Get the pointer to the next packet to be transmitted */
        Pcktdata =(uint8_t*)(*(ImagePtr + img_count)+packet_index * ((uint16_t)(usbd_video_ep_mps - 2U)));

        /* Set the current packet size */
        length = (packet_remainder + 2);
      }
      else
      {
        /* New image to be started, send only the packet header */
        length = 0;
      }

      if (length > 2U)
      {
        /* Check if this is the first packet in current image */
        if(packet_index == 0U)
        {
          /* Set the packet start index */
          payload_header[1] ^= 0x01U;
        }

        /* Copy the packet in the buffer */
        ux_utility_memory_copy((video_frame_buffer + 2U), Pcktdata, (length - 2U));

      }

      /* Increment the packet count and check if it reached the end of current image buffer */
      if (packet_index++ >= (packet_count + 1))
      {
        /* Reset the packet count to zero */
        packet_index = 0U;

        /* Move to the next image in the images table */
        img_count++;

        ux_utility_delay_ms(USBD_VIDEO_IMAGE_LAPS);

        /* Check if images count has been reached, then reset to zero
           (go back to first image in circular loop) */
        if (img_count == IMG_NBR)
        {
          img_count = 0U;
        }
      }
      else
      {
        /* Add the packet header */
        video_frame_buffer[0] = payload_header[0];
        video_frame_buffer[1] = payload_header[1];

        /* Copy video buffer in video frame buffer */
        ux_utility_memory_copy(buffer, video_frame_buffer, length);

        /* Commit payload buffer */
        ux_device_class_video_write_payload_commit(stream, length);
      }
      break;

    case UVC_PLAY_STATUS_STOP:
    default:
      return;
  }
}

/* USER CODE END 1 */
