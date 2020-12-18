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
#include "ux_device_class_cdc_acm.h"
#include "main.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define  LANGUAGE_ID                                    0x0904
#define  MANUFACTURER_INDEX                             0x01
#define  PRODUCT_INDEX                                  0x02
#define  SERIAL_INDEX                                   0x03

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
  0x12,                                                         /* bLength */
  USB_DESC_TYPE_DEVICE,                                         /* bDescriptorType */
  0x00,                                                         /* bcdUSB */
  0x02,
  0x02,                                                         /* bDeviceClass */
  0x02,                                                         /* bDeviceSubClass */
  0x00,                                                         /* bDeviceProtocol */
  USB_MAX_EP0_SIZE,                                             /* bMaxPacketSize */
  LOBYTE(USBD_VID),                                             /* idVendor */
  HIBYTE(USBD_VID),                                             /* idVendor */
  LOBYTE(USBD_PID),                                             /* idProduct */
  HIBYTE(USBD_PID),                                             /* idProduct */
  0x00,                                                         /* bcdDevice rel. 2.00 */
  0x02,
  USBD_IDX_MFC_STR,                                             /* Index of manufacturer  string */
  USBD_IDX_PRODUCT_STR,                                         /* Index of product string */
  USBD_IDX_SERIAL_STR,                                          /* Index of serial number string */
  USBD_MAX_NUM_CONFIGURATION,                                   /* bNumConfigurations */

  /* Configuration Descriptor */
  0x09,                                                         /* bLength: Configuration Descriptor size */
  USB_DESC_TYPE_CONFIGURATION,                                  /* bDescriptorType: Configuration */
  USB_CDC_CONFIG_DESC_SIZ,                                      /* wTotalLength:no of returned bytes */
  0x00,
  0x02,                                                         /* bNumInterfaces: 2 interface */
  0x01,                                                         /* bConfigurationValue: Configuration value */
  0x00,                                                         /* iConfiguration: Index of string descriptor describing the configuration */
  0xC0,                                                         /* bmAttributes: self powered */
  0x32,                                                         /* MaxPower 100 mA */

  /* Interface association descriptor. 8 bytes. */
  0x08, 0x0b, 0x00,
  0x02, 0x02, 0x02, 0x00, 0x00,
  /*--------------------------------------------------------------------------*/

  /*Interface Descriptor */
  0x09,                                                         /* bLength: Interface Descriptor size */
  USB_DESC_TYPE_INTERFACE,                                      /* bDescriptorType: Interface */
  /* Interface descriptor type */
  0x00,                                                         /* bInterfaceNumber: Number of Interface */
  0x00,                                                         /* bAlternateSetting: Alternate setting */
  0x01,                                                         /* bNumEndpoints: One endpoints used */
  0x02,                                                         /* bInterfaceClass: Communication Interface Class */
  0x02,                                                         /* bInterfaceSubClass: Abstract Control Model */
  0x01,                                                         /* bInterfaceProtocol: Common AT commands */
  0x00,                                                         /* iInterface: */

  /*Header Functional Descriptor*/
  0x05,                                                         /* bLength: Endpoint Descriptor size */
  0x24,                                                         /* bDescriptorType: CS_INTERFACE */
  0x00,                                                         /* bDescriptorSubtype: Header Func Desc */
  0x10,                                                         /* bcdCDC: spec release number */
  0x01,

  /*Call Management Functional Descriptor*/
  0x05,                                                         /* bFunctionLength */
  0x24,                                                         /* bDescriptorType: CS_INTERFACE */
  0x01,                                                         /* bDescriptorSubtype: Call Management Func Desc */
  0x00,                                                         /* bmCapabilities: D0+D1 */
  0x01,                                                         /* bDataInterface: 1 */

  /*ACM Functional Descriptor*/
  0x04,                                                         /* bFunctionLength */
  0x24,                                                         /* bDescriptorType: CS_INTERFACE */
  0x02,                                                         /* bDescriptorSubtype: Abstract Control Management desc */
  0x02,                                                         /* bmCapabilities */

  /*Union Functional Descriptor*/
  0x05,                                                         /* bFunctionLength */
  0x24,                                                         /* bDescriptorType: CS_INTERFACE */
  0x06,                                                         /* bDescriptorSubtype: Union func desc */
  0x00,                                                         /* bMasterInterface: Communication class interface */
  0x01,                                                         /* bSlaveInterface0: Data Class Interface */

  /*Endpoint 2 Descriptor*/
  0x07,                                                         /* bLength: Endpoint Descriptor size */
  USB_DESC_TYPE_ENDPOINT,                                       /* bDescriptorType: Endpoint */
  CDC_CMD_EP,                                                   /* bEndpointAddress */
  0x03,                                                         /* bmAttributes: Interrupt */
  LOBYTE(CDC_CMD_PACKET_SIZE),                                  /* wMaxPacketSize: */
  HIBYTE(CDC_CMD_PACKET_SIZE),
  CDC_FS_BINTERVAL,                                             /* bInterval: */
  /*--------------------------------------------------------------------------*/

  /*Data class interface descriptor*/
  0x09,                                                         /* bLength: Endpoint Descriptor size */
  USB_DESC_TYPE_INTERFACE,                                      /* bDescriptorType: */
  0x01,                                                         /* bInterfaceNumber: Number of Interface */
  0x00,                                                         /* bAlternateSetting: Alternate setting */
  0x02,                                                         /* bNumEndpoints: Two endpoints used */
  0x0A,                                                         /* bInterfaceClass: CDC */
  0x00,                                                         /* bInterfaceSubClass: */
  0x00,                                                         /* bInterfaceProtocol: */
  0x00,                                                         /* iInterface: */

  /*Endpoint OUT Descriptor*/
  0x07,                                                         /* bLength: Endpoint Descriptor size */
  USB_DESC_TYPE_ENDPOINT,                                       /* bDescriptorType: Endpoint */
  CDC_OUT_EP,                                                   /* bEndpointAddress */
  0x02,                                                         /* bmAttributes: Bulk */
  LOBYTE(CDC_DATA_FS_MAX_PACKET_SIZE),                          /* wMaxPacketSize: */
  HIBYTE(CDC_DATA_FS_MAX_PACKET_SIZE),
  0x00,                                                         /* bInterval: ignore for Bulk transfer */

  /*Endpoint IN Descriptor*/
  0x07,                                                         /* bLength: Endpoint Descriptor size */
  USB_DESC_TYPE_ENDPOINT,                                       /* bDescriptorType: Endpoint */
  CDC_IN_EP,                                                    /* bEndpointAddress */
  0x02,                                                         /* bmAttributes: Bulk */
  LOBYTE(CDC_DATA_FS_MAX_PACKET_SIZE),                          /* wMaxPacketSize: */
  HIBYTE(CDC_DATA_FS_MAX_PACKET_SIZE),
  0x00                                                          /* bInterval: ignore for Bulk transfer */
};

#if defined ( __ICCARM__ ) /* IAR Compiler */
#pragma data_alignment=4
#endif /* defined ( __ICCARM__ ) */
/* USB HID device HS Configuration Descriptor */
__ALIGN_BEGIN uint8_t USBD_device_framework_high_speed[] __ALIGN_END =
{
  /* Device descriptor */
  0x12,                                                         /* bLength */
  USB_DESC_TYPE_DEVICE,                                         /* bDescriptorType */
  0x00,                                                         /* bcdUSB */
  0x02,
  0x02,                                                         /* bDeviceClass */
  0x02,                                                         /* bDeviceSubClass */
  0x00,                                                         /* bDeviceProtocol */
  USB_MAX_EP0_SIZE,                                             /* bMaxPacketSize */
  LOBYTE(USBD_VID),                                             /* idVendor */
  HIBYTE(USBD_VID),                                             /* idVendor */
  LOBYTE(USBD_PID),                                             /* idProduct */
  HIBYTE(USBD_PID),                                             /* idProduct */
  0x00,                                                         /* bcdDevice rel. 2.00 */
  0x02,
  USBD_IDX_MFC_STR,                                             /* Index of manufacturer string */
  USBD_IDX_PRODUCT_STR,                                         /* Index of product string */
  USBD_IDX_SERIAL_STR,                                          /* Index of serial number string */
  USBD_MAX_NUM_CONFIGURATION,                                   /* bNumConfigurations */

  /* Device qualifier descriptor */
  USB_LEN_DEV_QUALIFIER_DESC,
  USB_DESC_TYPE_DEVICE_QUALIFIER,
  0x00,
  0x02,
  0x00,
  0x00,
  0x00,
  0x40,
  0x01,
  0x00,

  /*Configuration Descriptor*/
  0x09,                                                         /* bLength: Configuration Descriptor size */
  USB_DESC_TYPE_CONFIGURATION,                                  /* bDescriptorType: Configuration */
  USB_CDC_CONFIG_DESC_SIZ,                                      /* wTotalLength:no of returned bytes */
  0x00,
  0x02,                                                         /* bNumInterfaces: 2 interface */
  0x01,                                                         /* bConfigurationValue: Configuration value */
  0x00,                                                         /* iConfiguration: Index of string descriptor describing the configuration */
  0xC0,                                                         /* bmAttributes: self powered */
  0x32,                                                         /* MaxPower 100 mA */

  /* Interface association descriptor. 8 bytes. */
  0x08, 0x0b, 0x00,
  0x02, 0x02, 0x02, 0x00, 0x00,
  /*--------------------------------------------------------------------------*/

  /*Interface Descriptor */
  0x09,                                                         /* bLength: Interface Descriptor size */
  USB_DESC_TYPE_INTERFACE,                                      /* bDescriptorType: Interface */
  /* Interface descriptor type */
  0x00,                                                         /* bInterfaceNumber: Number of Interface */
  0x00,                                                         /* bAlternateSetting: Alternate setting */
  0x01,                                                         /* bNumEndpoints: One endpoints used */
  0x02,                                                         /* bInterfaceClass: Communication Interface Class */
  0x02,                                                         /* bInterfaceSubClass: Abstract Control Model */
  0x01,                                                         /* bInterfaceProtocol: Common AT commands */
  0x00,                                                         /* iInterface: */

  /*Header Functional Descriptor*/
  0x05,                                                         /* bLength: Endpoint Descriptor size */
  0x24,                                                         /* bDescriptorType: CS_INTERFACE */
  0x00,                                                         /* bDescriptorSubtype: Header Func Desc */
  0x10,                                                         /* bcdCDC: spec release number */
  0x01,

  /*Call Management Functional Descriptor*/
  0x05,                                                         /* bFunctionLength */
  0x24,                                                         /* bDescriptorType: CS_INTERFACE */
  0x01,                                                         /* bDescriptorSubtype: Call Management Func Desc */
  0x00,                                                         /* bmCapabilities: D0+D1 */
  0x01,                                                         /* bDataInterface: 1 */

  /*ACM Functional Descriptor*/
  0x04,                                                         /* bFunctionLength */
  0x24,                                                         /* bDescriptorType: CS_INTERFACE */
  0x02,                                                         /* bDescriptorSubtype: Abstract Control Management desc */
  0x02,                                                         /* bmCapabilities */

  /*Union Functional Descriptor*/
  0x05,                                                         /* bFunctionLength */
  0x24,                                                         /* bDescriptorType: CS_INTERFACE */
  0x06,                                                         /* bDescriptorSubtype: Union func desc */
  0x00,                                                         /* bMasterInterface: Communication class interface */
  0x01,                                                         /* bSlaveInterface0: Data Class Interface */

  /*Endpoint 2 Descriptor*/
  0x07,                                                         /* bLength: Endpoint Descriptor size */
  USB_DESC_TYPE_ENDPOINT,                                       /* bDescriptorType: Endpoint */
  CDC_CMD_EP,                                                   /* bEndpointAddress */
  0x03,                                                         /* bmAttributes: Interrupt */
  LOBYTE(CDC_CMD_PACKET_SIZE),                                  /* wMaxPacketSize: */
  HIBYTE(CDC_CMD_PACKET_SIZE),
  CDC_HS_BINTERVAL,                                             /* bInterval: */
  /*--------------------------------------------------------------------------*/

  /*Data class interface descriptor*/
  0x09,                                                         /* bLength: Endpoint Descriptor size */
  USB_DESC_TYPE_INTERFACE,                                      /* bDescriptorType: */
  0x01,                                                         /* bInterfaceNumber: Number of Interface */
  0x00,                                                         /* bAlternateSetting: Alternate setting */
  0x02,                                                         /* bNumEndpoints: Two endpoints used */
  0x0A,                                                         /* bInterfaceClass: CDC */
  0x00,                                                         /* bInterfaceSubClass: */
  0x00,                                                         /* bInterfaceProtocol: */
  0x00,                                                         /* iInterface: */

  /*Endpoint OUT Descriptor*/
  0x07,                                                         /* bLength: Endpoint Descriptor size */
  USB_DESC_TYPE_ENDPOINT,                                       /* bDescriptorType: Endpoint */
  CDC_OUT_EP,                                                   /* bEndpointAddress */
  0x02,                                                         /* bmAttributes: Bulk */
  LOBYTE(CDC_DATA_HS_MAX_PACKET_SIZE),                          /* wMaxPacketSize: */
  HIBYTE(CDC_DATA_HS_MAX_PACKET_SIZE),
  0x00,                                                         /* bInterval: ignore for Bulk transfer */

  /*Endpoint IN Descriptor*/
  0x07,                                                         /* bLength: Endpoint Descriptor size */
  USB_DESC_TYPE_ENDPOINT,                                       /* bDescriptorType: Endpoint */
  CDC_IN_EP,                                                    /* bEndpointAddress */
  0x02,                                                         /* bmAttributes: Bulk */
  LOBYTE(CDC_DATA_HS_MAX_PACKET_SIZE),                          /* wMaxPacketSize: */
  HIBYTE(CDC_DATA_HS_MAX_PACKET_SIZE),
  0x00                                                          /* bInterval: ignore for Bulk transfer */
};


/* String Device Framework :
 Byte 0 and 1 : Word containing the language ID : 0x0904 for US
 Byte 2       : Byte containing the index of the descriptor
 Byte 3       : Byte containing the length of the descriptor string
*/
#if defined ( __ICCARM__ ) /* IAR Compiler */
#pragma data_alignment=4
#endif /* defined ( __ICCARM__ ) */
__ALIGN_BEGIN UCHAR USBD_string_framework[STRING_FRAMWORK_MAX_SIZE] __ALIGN_END;

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
  * @param  length : length of HIGH or FULL SPEED array
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
  USBD_string_framework[count++] = MANUFACTURER_INDEX;

  /* Set the Manufacturer string in string_framework */
  USBD_Desc_GetString((uint8_t *)USBD_MANUFACTURER_STRING, USBD_string_framework + count, &len);

  /* Set the Product language Id and index in USBD_string_framework */
  count += len + 1;
  USBD_string_framework[count++] = LANGUAGE_ID >> 8;
  USBD_string_framework[count++] = LANGUAGE_ID & 0xFF;
  USBD_string_framework[count++] = PRODUCT_INDEX;

  /* Set the Product string in USBD_string_framework */
  USBD_Desc_GetString((uint8_t *)USBD_PRODUCT_STRING, USBD_string_framework + count, &len);

  /* Set Serial language Id and index in string_framework */
  count += len + 1;
  USBD_string_framework[count++] = LANGUAGE_ID >> 8;
  USBD_string_framework[count++] = LANGUAGE_ID & 0xFF;
  USBD_string_framework[count++] = SERIAL_INDEX;

  /* Set the Serial number in USBD_string_framework */
  USBD_Desc_GetString((uint8_t *)USBD_SERIAL_NUMBER, USBD_string_framework + count, &len);

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
