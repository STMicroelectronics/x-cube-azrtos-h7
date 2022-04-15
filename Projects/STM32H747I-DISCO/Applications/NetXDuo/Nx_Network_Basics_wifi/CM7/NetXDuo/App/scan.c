/**
  ******************************************************************************
  * @file    scan.c
  * @author  MCD Application Team
  * @brief   Scan cmd.
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

/* Includes ------------------------------------------------------------------*/
#include "msg.h"
#include <string.h>
#include <stdio.h>
#include <inttypes.h>
#include "whd_types.h"
#include "whd.h"
#include "whd_wifi_api.h"

#define NET_STATE_TRANSITION_TIMEOUT 60000

#define MAX_SCAN_AP                  100

extern whd_interface_t *Ifp;

int32_t scan_cmd(int32_t argc, char **argv);


static int32_t whd_wifi_get_scan_result(whd_sync_scan_result_t *scan_bss_array,
                                       uint8_t scan_bss_array_size);
static const char *wifi_security_to_string(uint32_t sec);


/**
  * @brief Cypress wifi get scan result
  * @param  scan_bss_array       bss array buffer
  * @param  scan_bss_array_size  bss array size
  * @return int32_t              number of the bss got
  */
static int32_t whd_wifi_get_scan_result(whd_sync_scan_result_t *scan_bss_array,
                                       uint8_t scan_bss_array_size)
{
  int32_t ret;
  uint8_t number = scan_bss_array_size;

  if ((NULL == scan_bss_array) || (0 == number))
  {
    ret = WHD_BADARG;
  }

  /* get real mc_wifi scan results data */
  ret = whd_wifi_scan_synch(*Ifp, scan_bss_array, number);

  return ret;
}

/**
  * @brief  Convert wifi security enum value to string
  * @param  sec is an unsigned integer
  * @retval a constant string, for instance "Open" or "WPA2-AES"
  */
static const char *wifi_security_to_string(uint32_t sec)
{
  const char *s;
  if (sec == WHD_SECURITY_OPEN)
  {
    s =  "Open";
  }
  else if (sec == WHD_SECURITY_WEP_SHARED)
  {
    s = "WEP-shared";
  }
  else if (sec == WHD_SECURITY_WPA_TKIP_PSK)
  {
    s = "WPA-TKIP";
  }
  else if (sec == WHD_SECURITY_WPA_MIXED_PSK)
  {
    s = "WPA-Mixed";
  }
  else if (sec == WHD_SECURITY_WPA2_AES_PSK)
  {
    s = "WPA2-AES";
  }
  else if (sec == WHD_SECURITY_WPA2_TKIP_PSK)
  {
    s = "WPA2-TKIP";
  }
  else if (sec == WHD_SECURITY_WPA2_MIXED_PSK)
  {
    s = "WPA2_Mixed";
  }
  else if (sec == WHD_SECURITY_WPA2_FBT_PSK)
  {
    s = "WPA2-FBT";
  }
  else if (sec == WHD_SECURITY_WPA3_SAE)
  {
    s = "WPA3";
  }
  else if (sec == WHD_SECURITY_WPA3_WPA2_PSK)
  {
    s = "WPA3-WPA2";
  }
  else if (sec == WHD_SECURITY_WPA_TKIP_ENT)
  {
    s = "WPA-TKIP-Ent";
  }
  else if (sec == WHD_SECURITY_WPA_AES_ENT)
  {
    s = "WPA-AES-Ent";
  }
  else if (sec == WHD_SECURITY_WPA2_TKIP_ENT)
  {
    s = "WPA2-TKIP-Ent";
  }
  else if (sec == WHD_SECURITY_WPA2_AES_ENT)
  {
    s = "WPA2-AES-Ent";
  }
  else if (sec == WHD_SECURITY_WPA2_MIXED_ENT)
  {
    s = "WPA2-Mixed-Ent";
  }
  else if (sec == WHD_SECURITY_WPA2_FBT_ENT)
  {
    s = "WPA-FBT-Ent";
  }
  else if (sec == WHD_SECURITY_IBSS_OPEN)
  {
    s = "IBS";
  }
  else if (sec == WHD_SECURITY_WPS_OPEN)
  {
    s = "WPS";
  }
  else if (sec == WHD_SECURITY_WPS_SECURE)
  {
    s = "WPS-AES";
  }
  else
  {
    s = "Unknown";
  }
  return s;
}

int32_t scan_cmd(int32_t argc, char **argv)
{
  UNUSED(argc);
  UNUSED(argv);
  int32_t ret;
  static whd_sync_scan_result_t APs[MAX_SCAN_AP] = {0};

  memset(APs, 0, sizeof(APs));
  ret = whd_wifi_get_scan_result(APs, MAX_SCAN_AP);

  if (ret > 0)
  {
    MSG_INFO("######### Scan %"PRIi32" BSS ##########\n", ret);
    for (int32_t i = 0; i < ret; i++)
    {
#define AP_SSID_LENGTH sizeof(((whd_sync_scan_result_t *)0)->SSID.value)

     /* Ensure that the string ends with '\0'. */
     APs[i].SSID.value[AP_SSID_LENGTH - 1] = '\0';

      MSG_INFO("\t%2"PRIi32"\t%40s ch %2"PRIu32" rss %"PRIi32" Security %10s country %4s"
               " bssid %02x.%02x.%02x.%02x.%02x.%02x\n",
               i, APs[i].SSID.value, (uint32_t)APs[i].channel, (int32_t)APs[i].signal_strength,
               wifi_security_to_string(APs[i].security), ".CN", /* NOT SUPPORT for WHD */
               APs[i].BSSID.octet[0], APs[i].BSSID.octet[1], APs[i].BSSID.octet[2], APs[i].BSSID.octet[3], APs[i].BSSID.octet[4], APs[i].BSSID.octet[5]);
    }
    MSG_INFO("######### End of Scan ##########\n");
  }
  else
  {
    MSG_ERROR("Scan operation failed (%"PRIi32")!", ret);
  }
  return 0;
}
