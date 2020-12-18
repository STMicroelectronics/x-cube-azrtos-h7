/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    ux_device_descriptors.c
  * @author  MCD Application Team
  * @brief   USBX Device descriptors file
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "ux_device_descriptors.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "ux_device_class_storage.h"
#include "main.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define  LANGUAGE_ID                                    0x0904

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */
#if defined ( __ICCARM__ ) /* IAR Compiler */
#pragma data_alignment=4
#endif /* defined ( __ICCARM__ ) */
__ALIGN_BEGIN uint8_t USBD_device_framework_full_speed[] __ALIGN_END =
{
  /* Device descriptor */
  0x12,                                               /* bLength */
  UX_DEVICE_DESCRIPTOR_ITEM,                          /* bDescriptorType */
  0x00,                                               /* bcdUSB */
  0x02,
  0x00,                                               /* bDeviceClass */
  0x00,                                               /* bDeviceSubClass */
  0x00,                                               /* bDeviceProtocol */
  USBD_MAX_EP0_SIZE,                                  /* bMaxPacketSize */
  LOBYTE(USBD_VID),                                   /* idVendor */
  HIBYTE(USBD_VID),                                   /* idVendor */
  LOBYTE(USBD_PID),                                   /* idVendor */
  HIBYTE(USBD_PID),                                   /* idVendor */
  0x00,                                               /* bcdDevice rel. 2.00 */
  0x02,
  USBD_IDX_MFC_STR,                                   /* Index of manufacturer string */
  USBD_IDX_PRODUCT_STR,                               /* Index of product string */
  USBD_IDX_SERIAL_STR,                                /* Index of serial number string */
  USBD_MAX_NUM_CONFIGURATION,                         /* bNumConfigurations */

  /*************** Configuration descriptor *************/
  0x09,                                               /* bLength: Configuration Descriptor size */
  UX_CONFIGURATION_DESCRIPTOR_ITEM,                   /* bDescriptorType: Configuration */
  USBD_STORAGE_CONFIG_DESC_SIZE,                      /* wTotalLength: Bytes returned */
  0x00,
  0x01,                                               /* bNumInterfaces: 1 interface */
  0x01,                                               /* bConfigurationValue: Configuration value */
  0x00,                                               /* iConfiguration: Index of string descriptor describing the configuration */
  0xC0,                                               /* bmAttributes: bus powered */
  0x32,                                               /* MaxPower 100 mA: this current is used for detecting Vbus */

  /************** Interface descriptor  ****************/
  /* 09 */
  0x09,                                               /* bLength: Interface Descriptor size */
  UX_INTERFACE_DESCRIPTOR_ITEM,                       /* bDescriptorType: Interface descriptor type */
  0x00,                                               /* bInterfaceNumber: Number of Interface */
  0x00,                                               /* bAlternateSetting: Alternate setting */
  0x02,                                               /* bNumEndpoints */
  0x08,                                               /* bInterfaceClass: MSC */
  0x06,                                               /* bInterfaceSubClass */
  0x50,                                               /* nInterfaceProtocol */
  0x00,                                               /* iInterface: Index of string descriptor */
  /******************** Endpoint descriptor (Bulk In) ********************/
  /* 18 */
  0x07,                                               /* bLength: Endpoint Descriptor size */
  UX_ENDPOINT_DESCRIPTOR_ITEM,                        /* bDescriptorType: */
  USBD_STORAGE_EPIN_ADDR,                             /* bEndpointAddress: Endpoint Address (IN) */
  0x02,                                               /* bmAttributes: Interrupt endpoint */
  USBD_STORAGE_EPIN_SIZE,                             /* wMaxPacketSize: 64 Byte max for Bulk */
  0x00,
  USBD_STORAGE_FS_BINTERVAL,                          /* bInterval: Polling Interval */
  /******************** Endpoint descriptor (Bulk Out) ********************/
  /* 25 */
  0x07,                                               /* bLength: Endpoint Descriptor size */
  UX_ENDPOINT_DESCRIPTOR_ITEM,                        /* bDescriptorType: */
  USBD_STORAGE_EPOUT_ADDR,                            /* bEndpointAddress: Endpoint Address (OUT) */
  0x02,                                               /* bmAttributes: Interrupt endpoint */
  USBD_STORAGE_FS_EPOUT_SIZE,                         /* wMaxPacketSize: 64 Byte max for Bulk */
  0x00,
  USBD_STORAGE_FS_BINTERVAL,                          /* bInterval: Polling Interval */
  /* 32 */
};

#if defined ( __ICCARM__ ) /* IAR Compiler */
#pragma data_alignment=4
#endif /* defined ( __ICCARM__ ) */
/* USB MSC device HS Configuration Descriptor */
__ALIGN_BEGIN uint8_t USBD_device_framework_high_speed[] __ALIGN_END =
{
  /* Device descriptor */
  0x12,                                               /* bLength */
  UX_DEVICE_DESCRIPTOR_ITEM,                          /* bDescriptorType */
  0x00,                                               /* bcdUSB */
  0x02,
  0x00,                                               /* bDeviceClass */
  0x00,                                               /* bDeviceSubClass */
  0x00,                                               /* bDeviceProtocol */
  USBD_MAX_EP0_SIZE,                                  /* bMaxPacketSize */
  LOBYTE(USBD_VID),                                   /* idVendor */
  HIBYTE(USBD_VID),                                   /* idVendor */
  LOBYTE(USBD_PID),                                   /* idVendor */
  HIBYTE(USBD_PID),                                   /* idVendor */
  0x00,                                               /* bcdDevice rel. 2.00 */
  0x02,
  USBD_IDX_MFC_STR,                                   /* Index of manufacturer string */
  USBD_IDX_PRODUCT_STR,                               /* Index of product string */
  USBD_IDX_SERIAL_STR,                                /* Index of serial number string */
  USBD_MAX_NUM_CONFIGURATION,                         /* bNumConfigurations */

  /* Configuration descriptor */
  0x09,                                               /* bLength: Configuration Descriptor size */
  UX_CONFIGURATION_DESCRIPTOR_ITEM,                   /* bDescriptorType: Configuration */
  USBD_STORAGE_CONFIG_DESC_SIZE,                      /* wTotalLength: Bytes returned */
  0x00,
  0x01,                                               /* bNumInterfaces: 1 interface */
  0x01,                                               /* bConfigurationValue: Configuration value */
  0x00,                                               /* iConfiguration: Index of string descriptor describing the configuration */
  0xC0,                                               /* bmAttributes: bus powered */
  0x32,                                               /* MaxPower 100 mA: this current is used for detecting Vbus */

  /************** Interface descriptor  ****************/
  /* 09 */
  0x09,                                               /* bLength: Interface Descriptor size */
  UX_INTERFACE_DESCRIPTOR_ITEM,                       /* bDescriptorType: Interface descriptor type */
  0x00,                                               /* bInterfaceNumber: Number of Interface */
  0x00,                                               /* bAlternateSetting: Alternate setting */
  0x02,                                               /* bNumEndpoints */
  0x08,                                               /* bInterfaceClass: MSC */
  0x06,                                               /* bInterfaceSubClass */
  0x50,                                               /* nInterfaceProtocol */
  0x00,                                               /* iInterface: Index of string descriptor */
  /******************** Endpoint descriptor (Bulk In) ********************/
  /* 27 */
  0x07,                                               /* bLength: Endpoint Descriptor size */
  UX_ENDPOINT_DESCRIPTOR_ITEM,                        /* bDescriptorType: */
  USBD_STORAGE_EPIN_ADDR,                             /* bEndpointAddress: Endpoint Address (IN) */
  0x02,                                               /* bmAttributes: Interrupt endpoint */
  USBD_STORAGE_EPIN_SIZE,                             /* wMaxPacketSize: 512 Bytes max for Bulk */
  0x00,
  USBD_STORAGE_HS_BINTERVAL,                          /* bInterval: Polling Interval */
  /* 34 */
  /******************** Endpoint descriptor (Bulk Out) ********************/
  /* 27 */
  0x08,                                               /* bLength: Endpoint Descriptor size */
  UX_ENDPOINT_DESCRIPTOR_ITEM,                        /* bDescriptorType: */
  USBD_STORAGE_EPOUT_ADDR,                            /* bEndpointAddress: Endpoint Address (OUT) */
  0x02,                                               /* bmAttributes: Interrupt endpoint */
  LOBYTE(USBD_STORAGE_HS_EPOUT_SIZE),                 /* wMaxPacketSize: 512 Bytes max for Bulk */
  HIBYTE(USBD_STORAGE_HS_EPOUT_SIZE),
  0x00,
  USBD_STORAGE_HS_BINTERVAL,                          /* bInterval: Polling Interval */
  /* 35 */
};

/* String Device Framework :
 Byte 0 and 1 : Word containing the language ID : 0x0904 for US
 Byte 2       : Byte containing the index of the descriptor
 Byte 3       : Byte containing the length of the descriptor string
*/
#if defined ( __ICCARM__ ) /* IAR Compiler */
#pragma data_alignment=4
#endif /* defined ( __ICCARM__ ) */
__ALIGN_BEGIN UCHAR USBD_string_framework[STRING_FRAMEWORK_MAX_LENGTH]
__ALIGN_END = {0} ;

/* Multiple languages are supported on the device, to add
   a language besides english, the unicode language code must
   be appended to the language_id_framework array and the length
   adjusted accordingly. */

#if defined ( __ICCARM__ ) /* IAR Compiler */
#pragma data_alignment=4
#endif /* defined ( __ICCARM__ ) */
UCHAR USBD_language_id_framework[] =
{

  /* English. */
  0x09, 0x04
};
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */

static void USBD_Desc_GetString(uint8_t *desc, uint8_t *Buffer, uint16_t *len);

static uint8_t USBD_Desc_GetLen(uint8_t *buf);

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/* USER CODE BEGIN 1 */
/**
  * @brief  USBD_Get_Device_Framework_Speed
  *         Return the device speed descriptor
  * @param Speed : HIGH or FULL SPEED flag
  * @param length : length of HIGH or FULL SPEED array
  * @retval Pointer to descriptor buffer
  */
uint8_t *USBD_Get_Device_Framework_Speed(uint8_t Speed, ULONG *Length)
{
  if (FULL_SPEED == Speed)
  {
    /* Get the length of USBD_device_framework_full_speed */
    *Length = sizeof(USBD_device_framework_full_speed);

    return USBD_device_framework_full_speed;
  }
  else
  {
    /* Get the length of USBD_device_framework_high_speed */
    *Length = sizeof(USBD_device_framework_high_speed);

    return USBD_device_framework_high_speed;
  }
}

/**
  * @brief  USBD_Get_String_Framework
  *         Return the language_id_framework
  * @param  Length : Length of String_Framework
  * @retval Pointer to language_id_framework buffer
  */
uint8_t *USBD_Get_String_Framework(ULONG *Length)
{
  uint16_t count = 0;
  uint16_t len = 0 ;

  /* Set the Manufacturer language Id and index in USBD_string_framework */
  USBD_string_framework[count++] = LANGUAGE_ID >> 8;
  USBD_string_framework[count++] = LANGUAGE_ID & 0xFF;
  USBD_string_framework[count++] = USBD_IDX_MFC_STR;

  /* Set the Manufacturer string in string_framework */
  USBD_Desc_GetString((uint8_t *)USBD_MANUFACTURER_STRING,
                      USBD_string_framework + count, &len);

  /* Set the Product language Id and index in USBD_string_framework */
  count += len + 1 ;
  USBD_string_framework[count++] = LANGUAGE_ID >> 8;
  USBD_string_framework[count++] = LANGUAGE_ID & 0xFF;
  USBD_string_framework[count++] = USBD_IDX_PRODUCT_STR;

  /* Set the Product string in USBD_string_framework */
  USBD_Desc_GetString((uint8_t *)USBD_PRODUCT_STRING,
                      USBD_string_framework + count, &len);

  /* Set Serial language Id and index in string_framework */
  count += len + 1;
  USBD_string_framework[count++] = LANGUAGE_ID >> 8;
  USBD_string_framework[count++] = LANGUAGE_ID & 0xFF;
  USBD_string_framework[count++] = USBD_IDX_SERIAL_STR;

  /* Set the Serial number in USBD_string_framework */
  USBD_Desc_GetString((uint8_t *)USBD_SERIAL_NUMBER,
                      USBD_string_framework + count, &len);

  /* Get the length of USBD_string_framework */
  *Length = strlen((const char *)USBD_string_framework);

  return USBD_string_framework;
}

/**
  * @brief  USBD_Get_String_Framework
  *         Return the language_id_framework
  * @param  Length : Length of Language_Id_Framework
  * @retval Pointer to language_id_framework buffer
  */
uint8_t *USBD_Get_Language_Id_Framework(ULONG *Length)
{
  /* Get the length of USBD_language_id_framework */
  *Length = sizeof(USBD_language_id_framework);

  return USBD_language_id_framework;
}

/**
  * @brief  USBD_Desc_GetString
  *         Convert Ascii string into unicode one
  * @param  desc : descriptor buffer
  * @param  unicode : Formatted string buffer (unicode)
  * @param  len : descriptor length
  * @retval None
  */
static void USBD_Desc_GetString(uint8_t *desc, uint8_t *unicode, uint16_t *len)
{
  uint8_t idx = 0U;
  uint8_t *pdesc;

  if (desc == NULL)
  {
    return;
  }

  pdesc = desc;
  *len = (uint16_t)USBD_Desc_GetLen(pdesc);

  unicode[idx++] = *(uint8_t *)len;

  while (*pdesc != (uint8_t)'\0')
  {
    unicode[idx++] = *pdesc;
    pdesc++;

  }
}

/**
  * @brief  USBD_Desc_GetLen
  *         return the string length
  * @param  buf : pointer to the ascii string buffer
  * @retval string length
  */
static uint8_t USBD_Desc_GetLen(uint8_t *buf)
{
  uint8_t  len = 0U;
  uint8_t *pbuff = buf;

  while (*pbuff != (uint8_t)'\0')
  {
    len++;
    pbuff++;
  }

  return len;
}

/* USER CODE END 1 */
