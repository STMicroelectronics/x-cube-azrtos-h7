/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    ux_host_audio.c
  * @author  MCD Application Team
  * @brief   USBX Host Audio applicative source file
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
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
#include "ux_host_audio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "app_filex.h"
#include "app_usbx_host.h"
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
extern UX_HOST_CLASS_AUDIO *audio_playback;
extern TX_SEMAPHORE data_send_semaphore;

UINT file_index;
FileListTypeDef FileList;
audio_demo_playback_stateTypeDef audio_demo_playback_state;
audio_demo_playback_stateTypeDef audio_demo_playback_prev_state;

UX_HOST_CLASS_AUDIO_CONTROL audio_speaker_control1;
UX_HOST_CLASS_AUDIO_CONTROL audio_speaker_control2;
UX_HOST_CLASS_AUDIO_CONTROL audio_speaker_mute;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/* USER CODE BEGIN 1 */
/**
  * @brief  Function implementing audio_playback_thread_entry.
  * @param  thread_input: not used
  * @retval none
  */
VOID audio_playback_thread_entry(ULONG thread_input)
{
  UINT status;
  SHORT min, max;
  ULONG duration;
  WAV_InfoTypedef WavPlayInfo;
  static UINT double_buffer_index;
  UCHAR audio_play_feature_unit_id;
  UCHAR audio_frame_length;
  ULONG audio_requested_length;
  ULONG audio_data_offset;
  ULONG audio_buffer_size;
  UCHAR *local_frame_buffer1;
  UCHAR *local_frame_buffer2;
  UCHAR *actual_frame_buffer;
  UX_HOST_CLASS_AUDIO_SAMPLING audio_play_sampling;
  UX_HOST_CLASS_AUDIO_TRANSFER_REQUEST transfer_request1;
  UX_HOST_CLASS_AUDIO_TRANSFER_REQUEST transfer_request2;
  UX_HOST_CLASS_AUDIO_TRANSFER_REQUEST *actual_transfer_request;
  UX_HOST_CLASS_AUDIO_TRANSFER_REQUEST *previous_transfer_request;

  UX_PARAMETER_NOT_USED(thread_input);

  while(1)
  {
    /* Start if the audio device speaker is connected */
    if ((audio_playback != NULL) &&
        (audio_playback->ux_host_class_audio_state == UX_HOST_CLASS_INSTANCE_LIVE))
    {

      switch(audio_demo_playback_state)
      {
        case AUDIO_DEMO_PLAYBACK_IDLE:

          /* Select Audio Streaming alternate 0 */
          ux_host_stack_interface_setting_select(audio_playback->ux_host_class_audio_streaming_interface);

          /* Update audio demo state machine */
          audio_demo_playback_state = AUDIO_DEMO_PLAYBACK_EXPLORE;
          audio_demo_playback_prev_state = AUDIO_DEMO_PLAYBACK_IDLE;

          break;

        case AUDIO_DEMO_PLAYBACK_EXPLORE:

          USBH_UsrLog("SD Parsing ......... \n");

          /* Parse SD media */
          SD_StorageParse(&FileList);

          USBH_UsrLog("\nTo start Playback, please press the user button.\n");

          /* Update audio demo state machine */
          audio_demo_playback_state = AUDIO_DEMO_PLAYBACK_WAIT;
          audio_demo_playback_prev_state = AUDIO_DEMO_PLAYBACK_EXPLORE;

          break;

        case AUDIO_DEMO_PLAYBACK_CONFIG:

          /* Find the right feature unit for audio playback stream */
          audio_play_feature_unit_id = audio_playback->ux_host_class_audio_feature_unit_id;

          /* Read the static volume values for speaker channel 1 and 2 */
          audio_speaker_control1.ux_host_class_audio_control_entity = audio_play_feature_unit_id;
          audio_speaker_control1.ux_host_class_audio_control_size = 2;
          audio_speaker_control1.ux_host_class_audio_control = UX_HOST_CLASS_AUDIO_VOLUME_CONTROL;
          audio_speaker_control1.ux_host_class_audio_control_channel = 1;

          audio_speaker_control2.ux_host_class_audio_control_entity = audio_play_feature_unit_id;
          audio_speaker_control2.ux_host_class_audio_control_size = 2;
          audio_speaker_control2.ux_host_class_audio_control = UX_HOST_CLASS_AUDIO_VOLUME_CONTROL;
          audio_speaker_control2.ux_host_class_audio_control_channel = 2;

          ux_host_class_audio_entity_control_get(audio_playback, &audio_speaker_control1);
          ux_host_class_audio_entity_control_get(audio_playback, &audio_speaker_control2);

          /* Set the current volume values for speaker channel 1 and 2 */
          min = (SHORT)audio_speaker_control1.ux_host_class_audio_control_min;
          max = (SHORT)audio_speaker_control1.ux_host_class_audio_control_max;
          audio_speaker_control1.ux_host_class_audio_control_cur = (ULONG)((min + max) / 2);
          ux_host_class_audio_entity_control_value_set(audio_playback, &audio_speaker_control1);
          ux_host_class_audio_entity_control_value_get(audio_playback, &audio_speaker_control1);

          min = (SHORT)audio_speaker_control2.ux_host_class_audio_control_min;
          max = (SHORT)audio_speaker_control2.ux_host_class_audio_control_max;
          audio_speaker_control2.ux_host_class_audio_control_cur = (ULONG)((min + max) / 2);
          ux_host_class_audio_entity_control_value_set(audio_playback, &audio_speaker_control2);
          ux_host_class_audio_entity_control_value_get(audio_playback, &audio_speaker_control2);

          audio_speaker_mute.ux_host_class_audio_control_entity = audio_play_feature_unit_id;
          audio_speaker_mute.ux_host_class_audio_control_size = 1;
          audio_speaker_mute.ux_host_class_audio_control = UX_HOST_CLASS_AUDIO_MUTE_CONTROL;
          audio_speaker_mute.ux_host_class_audio_control_channel = 0;
          audio_speaker_mute.ux_host_class_audio_control_cur = 0;
          ux_host_class_audio_entity_control_value_set(audio_playback, &audio_speaker_mute);

          /* Update audio demo state machine */
          audio_demo_playback_state = AUDIO_DEMO_PLAYBACK_START;
          audio_demo_playback_prev_state = AUDIO_DEMO_PLAYBACK_CONFIG;

          break;

        case AUDIO_DEMO_PLAYBACK_START:

          /* Get audio Wav file information from header  */
          AUDIO_GetFileInfo(file_index, &FileList, &WavPlayInfo);

          /* Print wav name */
          USBH_UsrLog("Playing file (%d/%d): %s", file_index+1, FileList.ptr, (char *)FileList.file[file_index].name);

          /* Print wav sample rate */
          USBH_UsrLog("Sample rate : %lu Hz", WavPlayInfo.SampleRate);

          /* Print wav channel number */
          USBH_UsrLog("Channels number : %d", WavPlayInfo.NbrChannels);

          /* Print wav duration */
          duration = WavPlayInfo.FileSize / WavPlayInfo.ByteRate;
          USBH_UsrLog("File Size : %lu MB [%02lu:%02lu]", WavPlayInfo.FileSize / 1024, duration / 60, duration % 60);

          /* Get audio frame length */
          if (ux_host_class_audio_speed_get(audio_playback) == UX_HIGH_SPEED_DEVICE)
          {
            audio_frame_length = ((WavPlayInfo.SampleRate * WavPlayInfo.NbrChannels * (WavPlayInfo.BitPerSample / 8)) / 8000);
          }
          else
          {
            audio_frame_length = ((WavPlayInfo.SampleRate * WavPlayInfo.NbrChannels * (WavPlayInfo.BitPerSample / 8)) / 1000);
          }

          /* Get audio buffer size */
          audio_buffer_size = audio_frame_length * 8U;

          /* Allocate local buffer */
          local_frame_buffer1 = ux_utility_memory_allocate(UX_SAFE_ALIGN, UX_CACHE_SAFE_MEMORY, audio_buffer_size);
          local_frame_buffer2 = ux_utility_memory_allocate(UX_SAFE_ALIGN, UX_CACHE_SAFE_MEMORY, audio_buffer_size);

          /* Reset local buffer memory */
          ux_utility_memory_set(local_frame_buffer1, 0, audio_buffer_size);
          ux_utility_memory_set(local_frame_buffer1, 0, audio_buffer_size);

          audio_data_offset = 0U;

          /* Check chunk size (not too small) */
          if (WavPlayInfo.SubChunk2Size < (audio_buffer_size * 3U))
          {
            Error_Handler();
          }

          audio_data_offset += sizeof(WAV_InfoTypedef);

          /* Update audio demo state machine */
          audio_demo_playback_state = AUDIO_DEMO_PLAYBACK_PLAY;
          audio_demo_playback_prev_state = AUDIO_DEMO_PLAYBACK_START;

          break;

        case AUDIO_DEMO_PLAYBACK_PLAY:

          /* Fill audio sampling struct */
          audio_play_sampling.ux_host_class_audio_sampling_channels = WavPlayInfo.NbrChannels ;
          audio_play_sampling.ux_host_class_audio_sampling_frequency = WavPlayInfo.SampleRate;
          audio_play_sampling.ux_host_class_audio_sampling_resolution = WavPlayInfo.BitPerSample;


          /* Set the alternate setting interface of a stereo USB speaker */
          if (ux_host_class_audio_streaming_sampling_set(audio_playback, &audio_play_sampling) != UX_SUCCESS)
          {
            USBH_UsrLog("\nThe Frequency or the resolution of this wav are not supported by device \n");

            /* Close file */
            AUDIO_EndOfWAVFile();

            if (file_index+1 < FileList.ptr)
            {
              USBH_UsrLog("\nPress the user button, To start Playback.\n");
            }

            /* Update audio demo state machine */
            audio_demo_playback_state = AUDIO_DEMO_PLAYBACK_WAIT;
            audio_demo_playback_prev_state = AUDIO_DEMO_PLAYBACK_PLAY;

            break;
          }

          /* Prepare the 2 audio transfer_requests */
          transfer_request1.ux_host_class_audio_transfer_request_completion_function = playback_transfer_request_completion;
          transfer_request2.ux_host_class_audio_transfer_request_completion_function = playback_transfer_request_completion;
          transfer_request1.ux_host_class_audio_transfer_request_class_instance = audio_playback;
          transfer_request2.ux_host_class_audio_transfer_request_class_instance = audio_playback;
          transfer_request1.ux_host_class_audio_transfer_request_next_audio_transfer_request = &transfer_request2;
          transfer_request2.ux_host_class_audio_transfer_request_next_audio_transfer_request = UX_NULL;

          /* Set audio packet size */
          transfer_request1.ux_host_class_audio_transfer_request_packet_size = ux_host_class_audio_packet_size_get(audio_playback);
          transfer_request2.ux_host_class_audio_transfer_request_packet_size = ux_host_class_audio_packet_size_get(audio_playback);

          /* Read the WAV in blocks */
          AUDIO_ReadFromWAVFile(local_frame_buffer1, audio_buffer_size, &audio_requested_length, &audio_data_offset, &WavPlayInfo);

          /* Set the buffer and length in the audio transfer_request */
          transfer_request1.ux_host_class_audio_transfer_request_data_pointer = local_frame_buffer1;
          transfer_request1.ux_host_class_audio_transfer_request_requested_length = audio_requested_length;

          /* Read the WAV in blocks */
          AUDIO_ReadFromWAVFile(local_frame_buffer2, audio_buffer_size, &audio_requested_length, &audio_data_offset, &WavPlayInfo);

          /* Set the buffer and length in the audio transfer_request */
          transfer_request2.ux_host_class_audio_transfer_request_data_pointer = local_frame_buffer2;
          transfer_request2.ux_host_class_audio_transfer_request_requested_length = audio_requested_length;

          /* Send the first 4 transfer_requests */
          status = ux_host_class_audio_write(audio_playback, &transfer_request1);
          status = ux_host_class_audio_write(audio_playback, &transfer_request2);

          /* Update audio demo state machine */
          audio_demo_playback_prev_state = AUDIO_DEMO_PLAYBACK_PLAY;

          /* First sent should be request 1 */
          double_buffer_index = 0;

          do
          {
            /* Wait for the transfer to be completed */
            status = tx_semaphore_get(&data_send_semaphore, TX_WAIT_FOREVER);

            if (double_buffer_index == 0)
            {
              actual_frame_buffer = local_frame_buffer1;
              actual_transfer_request = &transfer_request1;
              double_buffer_index = 1;
              previous_transfer_request = &transfer_request2;
            }
            else
            {
              actual_frame_buffer = local_frame_buffer2;
              actual_transfer_request = &transfer_request2;
              double_buffer_index = 0;
              previous_transfer_request = &transfer_request1;
            }

            /* Read the WAV in blocks */
            AUDIO_ReadFromWAVFile(actual_frame_buffer, audio_buffer_size, &audio_requested_length,
                                  &audio_data_offset, &WavPlayInfo);

            if (audio_requested_length != audio_buffer_size)
            {
              /* Close file */
              AUDIO_EndOfWAVFile();

              /* Stop the audio streaming and select alt interface 0 */
              if (ux_host_class_audio_stop(audio_playback) != UX_SUCCESS)
              {
                Error_Handler();
              }

              /* Free allocated file */
              ux_utility_memory_free(local_frame_buffer1);
              ux_utility_memory_free(local_frame_buffer2);

              ux_utility_memory_set(&audio_play_sampling, 0, sizeof(UX_HOST_CLASS_AUDIO_SAMPLING));
              ux_utility_memory_set(&transfer_request1, 0, sizeof(UX_HOST_CLASS_AUDIO_TRANSFER_REQUEST));
              ux_utility_memory_set(&transfer_request2, 0, sizeof(UX_HOST_CLASS_AUDIO_TRANSFER_REQUEST));
              ux_utility_memory_set(actual_transfer_request, 0, sizeof(UX_HOST_CLASS_AUDIO_TRANSFER_REQUEST));
              ux_utility_memory_set(previous_transfer_request, 0, sizeof(UX_HOST_CLASS_AUDIO_TRANSFER_REQUEST));

              audio_frame_length = 0U;
              audio_buffer_size = 0U;

              if (file_index+1 > FileList.ptr)
              {
                USBH_UsrLog("\nNo Wav file to play.\n");
              }

              /* Update audio demo state machine */
              audio_demo_playback_state = AUDIO_DEMO_PLAYBACK_WAIT;

              break;
            }

            /* Prepare next transfer request */
            previous_transfer_request->ux_host_class_audio_transfer_request_next_audio_transfer_request = actual_transfer_request;

            /* The next transfer request is null */
            actual_transfer_request->ux_host_class_audio_transfer_request_next_audio_transfer_request = UX_NULL;

            /* Send the Audio transfer request */
            status = ux_host_class_audio_write(audio_playback, actual_transfer_request);

          }while (status == UX_SUCCESS);

          break;

        case AUDIO_DEMO_PLAYBACK_NEXT:

          /* Increment file index */
          file_index ++;

          /* Check if there is an other wav file in SD to play */
          if (file_index >= FileList.ptr)
          {
            USBH_UsrLog("\nNo Wav file to play.\n");

            /* Update audio demo state machine */
            audio_demo_playback_state = AUDIO_DEMO_PLAYBACK_WAIT;
          }

          /* Update audio demo state machine */
          audio_demo_playback_state = AUDIO_DEMO_PLAYBACK_START;

          break;

        case AUDIO_DEMO_PLAYBACK_WAIT:
        default:

          /* Sleep thread for 10 ms */
          tx_thread_sleep(MS_TO_TICK(10));

          break;
      }
    }
    else
    {
      /* Sleep thread for 10 ms */
      tx_thread_sleep(MS_TO_TICK(10));
    }
  }
}

/**
  * @brief  playback_transfer_request_completion
  *         This function receives a completion call back on a isoch transfer request.
  * @param  transfer_request: Pointer to transfer request
  * @retval none
  */
VOID playback_transfer_request_completion(UX_HOST_CLASS_AUDIO_TRANSFER_REQUEST *transfer_request)
{
  /* Release the semaphore */
  tx_semaphore_put(&data_send_semaphore);
}
/* USER CODE END 1 */
