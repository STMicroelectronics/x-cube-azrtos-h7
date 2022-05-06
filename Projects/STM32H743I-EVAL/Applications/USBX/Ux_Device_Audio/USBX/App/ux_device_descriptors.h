/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    ux_device_descriptors.h
  * @author  MCD Application Team
  * @brief   USBX Device descriptor header file
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2020-2022 STMicroelectronics.
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
#ifndef __UX_DEVICE_DESCRIPTORS_H__
#define __UX_DEVICE_DESCRIPTORS_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "ux_api.h"
#include "ux_stm32_config.h"
#include "ux_device_class_audio.h"
#include "ux_device_class_audio20.h"
/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private defines -----------------------------------------------------------*/
#define USBD_MAX_NUM_CONFIGURATION                     1U
#define USBD_MAX_SUPPORTED_CLASS                       3U
#define USBD_MAX_CLASS_ENDPOINTS                       9U
#define USBD_MAX_CLASS_INTERFACES                      9U

#define USBD_AUDIO_CLASS_ACTIVATED                     1U

#define USBD_CONFIG_MAXPOWER                           25U
#define USBD_COMPOSITE_USE_IAD                         1U
#define USBD_DEVICE_FRAMEWORK_BUILDER_ENABLED          1U
/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Enum Class Type */
typedef enum
{
  CLASS_TYPE_NONE     = 0,
  CLASS_TYPE_HID      = 1,
  CLASS_TYPE_CDC_ACM  = 2,
  CLASS_TYPE_MSC      = 3,
  CLASS_TYPE_CDC_ECM  = 4,
  CLASS_TYPE_DFU      = 5,
  CLASS_TYPE_PIMA_MTP = 6,
  CLASS_TYPE_VIDEO    = 7,
  CLASS_TYPE_AUDIO    = 8,
} USBD_CompositeClassTypeDef;

/* USB Endpoint handle structure */
typedef struct
{
  uint32_t status;
  uint32_t total_length;
  uint32_t rem_length;
  uint32_t maxpacket;
  uint16_t is_used;
  uint16_t bInterval;
} USBD_EndpointTypeDef;

/* USB endpoint handle structure */
typedef struct
{
  uint8_t                     add;
  uint8_t                     type;
  uint16_t                    size;
  uint8_t                     is_used;
} USBD_EPTypeDef;

/* USB Composite handle structure */
typedef struct
{
  USBD_CompositeClassTypeDef   ClassType;
  uint32_t                     ClassId;
  uint32_t                     Active;
  uint32_t                     NumEps;
  USBD_EPTypeDef               Eps[USBD_MAX_CLASS_ENDPOINTS];
  uint32_t                     NumIf;
  uint8_t                      Ifs[USBD_MAX_CLASS_INTERFACES];
} USBD_CompositeElementTypeDef;

/* USB Device handle structure */
typedef struct _USBD_DevClassHandleTypeDef
{
  uint8_t                 Speed;
  uint32_t                classId;
  uint32_t                NumClasses;
  USBD_CompositeElementTypeDef tclasslist[USBD_MAX_SUPPORTED_CLASS];
  uint32_t                CurrDevDescSz;
  uint32_t                CurrConfDescSz;
} USBD_DevClassHandleTypeDef;

/* USB Device endpoint direction */
typedef enum
{
  OUT   = 0x00,
  IN    = 0x80,
} USBD_EPDirectionTypeDef;

/* USB Device descriptors structure */
typedef struct
{
  uint8_t           bLength;
  uint8_t           bDescriptorType;
  uint16_t          bcdUSB;
  uint8_t           bDeviceClass;
  uint8_t           bDeviceSubClass;
  uint8_t           bDeviceProtocol;
  uint8_t           bMaxPacketSize;
  uint16_t          idVendor;
  uint16_t          idProduct;
  uint16_t          bcdDevice;
  uint8_t           iManufacturer;
  uint8_t           iProduct;
  uint8_t           iSerialNumber;
  uint8_t           bNumConfigurations;
} __PACKED USBD_DeviceDescTypedef;

/* USB Iad descriptors structure */
typedef struct
{
  uint8_t           bLength;
  uint8_t           bDescriptorType;
  uint8_t           bFirstInterface;
  uint8_t           bInterfaceCount;
  uint8_t           bFunctionClass;
  uint8_t           bFunctionSubClass;
  uint8_t           bFunctionProtocol;
  uint8_t           iFunction;
} __PACKED USBD_IadDescTypedef;

/* USB interface descriptors structure */
typedef struct
{
  uint8_t           bLength;
  uint8_t           bDescriptorType;
  uint8_t           bInterfaceNumber;
  uint8_t           bAlternateSetting;
  uint8_t           bNumEndpoints;
  uint8_t           bInterfaceClass;
  uint8_t           bInterfaceSubClass;
  uint8_t           bInterfaceProtocol;
  uint8_t           iInterface;
} __PACKED USBD_IfDescTypedef;

/* USB endpoint descriptors structure */
typedef struct
{
  uint8_t           bLength;
  uint8_t           bDescriptorType;
  uint8_t           bEndpointAddress;
  uint8_t           bmAttributes;
  uint16_t          wMaxPacketSize;
  uint8_t           bInterval;
} __PACKED USBD_EpDescTypedef;

/* USB Config descriptors structure */
typedef struct
{
  uint8_t   bLength;
  uint8_t   bDescriptorType;
  uint16_t  wDescriptorLength;
  uint8_t   bNumInterfaces;
  uint8_t   bConfigurationValue;
  uint8_t   iConfiguration;
  uint8_t   bmAttributes;
  uint8_t   bMaxPower;
} __PACKED USBD_ConfigDescTypedef;

/* USB Qualifier descriptors structure */
typedef struct
{
  uint8_t           bLength;
  uint8_t           bDescriptorType;
  uint16_t          bcdDevice;
  uint8_t           Class;
  uint8_t           SubClass;
  uint8_t           Protocol;
  uint8_t           bMaxPacketSize;
  uint8_t           bNumConfigurations;
  uint8_t           bReserved;
} __PACKED USBD_DevQualiDescTypedef;

#if USBD_AUDIO_CLASS_ACTIVATED == 1
/* Class-Specific AC Interface Header Descriptor */
typedef struct
{
  uint8_t           bLength;
  uint8_t           bDescriptorType;
  uint8_t           bDescriptorSubtype;
  uint16_t          bcdADC;
  uint8_t           bCategory;
  uint16_t          wTotalLength;
  uint8_t           bmControls;
} __PACKED USBD_AUDIOCCSIfDescTypeDef;

/* Clock Source Descriptor */
typedef struct
{
  uint8_t           bLength;
  uint8_t           bDescriptorType;
  uint8_t           bDescriptorSubtype;
  uint8_t           bClockID;
  uint8_t           bmAttributes;
  uint8_t           bmControls;
  uint8_t           bAssocTerminal;
  uint8_t           iClockSource;
} __PACKED USBD_AUDIOClockSourceDescTypeDef;

/* Input Terminal Descriptor */
typedef struct
{
  uint8_t           bLength;
  uint8_t           bDescriptorType;
  uint8_t           bDescriptorSubtype;
  uint8_t           bTerminalID;
  uint16_t          wTerminalType;
  uint8_t           bAssocTerminal;
  uint8_t           bCSourceID;
  uint8_t           bNrChannels;
  uint32_t          bmChannelConfig;
  uint8_t           iChannelNames;
  uint16_t          bmControls;
  uint8_t           iTerminal;
} __PACKED USBD_AUDIOInputTerminalDescTypeDef;

/* Feature Unit Descriptor */
typedef struct
{
  uint8_t           bLength;
  uint8_t           bDescriptorType;
  uint8_t           bDescriptorSubtype;
  uint8_t           bUnitID;
  uint8_t           bSourceID;
  uint32_t          bmaControls[3];
  uint8_t           iFeature;
} __PACKED USBD_AUDIOFeatureUnitPlayDescTypeDef;

/* Output Terminal Descriptor */
typedef struct
{
  uint8_t           bLength;
  uint8_t           bDescriptorType;
  uint8_t           bDescriptorSubtype;
  uint8_t           bTerminalID;
  uint16_t          wTerminalType;
  uint8_t           bAssocTerminal;
  uint8_t           bSourceID;
  uint8_t           bCSourceID;
  uint16_t          bmaControls;
  uint8_t           iTerminal;
} __PACKED USBD_AUDIOOutputTerminalDescTypeDef;

/* Type I Format Type Descriptor */
typedef struct
{
  uint8_t           bLength;
  uint8_t           bDescriptorType;
  uint8_t           bDescriptorSubtype;
  uint8_t           bFormatType;
  uint8_t           bSubslotSize;
  uint8_t           bBitResolution;
} __PACKED USBD_AUDIOSFormatIfDescTypeDef;

/* Class-Specific AS Interface Descriptor */
typedef struct
{
  uint8_t           bLength;
  uint8_t           bDescriptorType;
  uint8_t           bDescriptorSubtype;
  uint8_t           bTerminalLink;
  uint8_t           bmControls;
  uint8_t           bFormatType;
  uint32_t          bmFormats;
  uint8_t           bNrChannels;
  uint32_t          bmChannelConfig;
  uint8_t           iChannelNames;
} __PACKED USBD_AUDIOSCSIfDescTypeDef;

/* Class-Specific AS Isochronous Audio Data Endpoint Descriptor */
typedef struct
{
  uint8_t           bLength;
  uint8_t           bDescriptorType;
  uint8_t           bDescriptorSubtype;
  uint8_t           bmAttributes;
  uint8_t           bmControls;
  uint8_t           bLockDelayUnits;
  uint16_t          wLockDelay;
} __PACKED USBD_AUDIOSCSEpDescTypeDef;

#endif /* USBD_AUDIO_CLASS_ACTIVATED */

/* Private defines -----------------------------------------------------------*/
/* USER CODE BEGIN Private_defines */

/* USER CODE END Private_defines */

/* Exported functions prototypes ---------------------------------------------*/
/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

uint8_t *USBD_Get_Device_Framework_Speed(uint8_t Speed, ULONG *Length);
uint8_t *USBD_Get_String_Framework(ULONG *Length);
uint8_t *USBD_Get_Language_Id_Framework(ULONG *Length);

/* Private defines -----------------------------------------------------------*/
/* USER CODE BEGIN Private_defines */

/* USER CODE END Private_defines */

#define USBD_VID                                       0x0483
#define USBD_PID                                       0x5730
#define USBD_LANGID_STRING                             1033
#define USBD_MANUFACTURER_STRING                       "STMicroelectronics"
#define USBD_PRODUCT_STRING                            "STM32 AUDIO20 STREAMING"
#define USBD_SERIAL_NUMBER                             "AUDIO001"

#define USB_DESC_TYPE_INTERFACE                        0x04U
#define USB_DESC_TYPE_ENDPOINT                         0x05U
#define USB_DESC_TYPE_CONFIGURATION                    0x02U
#define USB_DESC_TYPE_IAD                              0x0BU

#define USBD_EP_TYPE_CTRL                              0x00U
#define USBD_EP_TYPE_ISOC                              0x01U
#define USBD_EP_TYPE_BULK                              0x02U
#define USBD_EP_TYPE_INTR                              0x03U

#define USBD_EP_ATTR_ISOC_NOSYNC                       0x00
#define USBD_EP_ATTR_ISOC_ASYNC                        0x04
#define USBD_EP_ATTR_ISOC_ADAPT                        0x08
#define USBD_EP_ATTR_ISOC_SYNC                         0x0C
#define USBD_EP_ATTR_ISOC_FEEDBACK                     0x10

#define USBD_FULL_SPEED                                0x00U
#define USBD_HIGH_SPEED                                0x01U

#define USB_BCDUSB                                     0x0200U
#define LANGUAGE_ID_MAX_LENGTH                         2U

#define USBD_IDX_MFC_STR                               0x01U
#define USBD_IDX_PRODUCT_STR                           0x02U
#define USBD_IDX_SERIAL_STR                            0x03U

#define USBD_MAX_EP0_SIZE                              64U
#define USBD_DEVICE_QUALIFIER_DESC_SIZE                0x0AU

#define USBD_STRING_FRAMEWORK_MAX_LENGTH               256U

/* Device AUDIO Class */

#define USBD_AUDIO_AS_PLAY_INTERFACE                  0x01U

#define USB_AUDIO_CONFIG_PLAY_TERMINAL_INPUT_ID       0x12
#define USB_AUDIO_CONFIG_PLAY_UNIT_FEATURE_ID         0x16
#define USB_AUDIO_CONFIG_PLAY_TERMINAL_OUTPUT_ID      0x14
#define USB_AUDIO_CONFIG_PLAY_CLOCK_SOURCE_ID         0x18
#define USB_AUDIO_CONFIG_PLAY_SAMPLING_FREQUENCY      48000
#define USB_AUDIO_CONFIG_PLAY_CHANNEL_COUNT           0x02 /* stereo audio  */
#define USB_AUDIO_CONFIG_PLAY_RES_BIT                 16   /* 16 bit per sample */
#define USB_AUDIO_CONFIG_PLAY_RES_BYTE                2    /* 2 bytes */
#define USB_AUDIO_CONFIG_PLAY_CHANNEL_MAP             0x03

#define USBD_AUDIO_FU_CONTROL_MUTE                    0x0003
#define USBD_AUDIO_FU_CONTROL_VOLUME                  0x000C
#define USBD_PLAYBACK_AC_INTERFACE_SIZE               sizeof(USBD_AUDIOInputTerminalDescTypeDef)+\
                                                      sizeof(USBD_AUDIOFeatureUnitPlayDescTypeDef)+\
                                                      sizeof(USBD_AUDIOOutputTerminalDescTypeDef)+\
                                                      sizeof(USBD_AUDIOClockSourceDescTypeDef)\

#define USBD_CONFIG_DESCRIPTOR_AC_TOTAL_SIZE          sizeof(USBD_AUDIOCCSIfDescTypeDef)+\
                                                      USBD_PLAYBACK_AC_INTERFACE_SIZE\

#define USBD_AUDIO_EPOUT_ADDR                         0x01
#define USBD_AUDIO_EPOUT_Feedback_ADDR                0x81
#define USBD_AUDIO_EPIN_ADDR                          0x82

#define USBD_AUDIO_EPIN_FS_MPS                        (((USB_AUDIO_CONFIG_PLAY_SAMPLING_FREQUENCY + 999)/1000) *\
                                                      USB_AUDIO_CONFIG_PLAY_CHANNEL_COUNT *\
                                                      USB_AUDIO_CONFIG_PLAY_RES_BYTE)\

#define USBD_AUDIO_EPIN_HS_MPS                        (((USB_AUDIO_CONFIG_PLAY_SAMPLING_FREQUENCY + 7999)/8000) *\
                                                      USB_AUDIO_CONFIG_PLAY_CHANNEL_COUNT *\
                                                      USB_AUDIO_CONFIG_PLAY_RES_BYTE)\

/* This is the maximum supported configuration descriptor size
   User may redefine this value in order to optima */
#ifndef USBD_FRAMEWORK_MAX_DESC_SZ
#define USBD_FRAMEWORK_MAX_DESC_SZ                    200U
#endif /* USBD_FRAMEWORK_MAX_DESC_SZ */

#ifndef USBD_CONFIG_STR_DESC_IDX
#define USBD_CONFIG_STR_DESC_IDX                      0U
#endif /* USBD_CONFIG_STR_DESC_IDX */

#ifndef USBD_CONFIG_BMATTRIBUTES
#define USBD_CONFIG_BMATTRIBUTES                      0xC0U
#endif /* USBD_CONFIG_BMATTRIBUTES */

/* Private macro -----------------------------------------------------------*/
/* USER CODE BEGIN Private_macro */

/* USER CODE END Private_macro */
#define __USBD_FRAMEWORK_SET_EP(epadd, eptype, epsize, HSinterval, FSinterval) do { \
                                /* Append Endpoint descriptor to Configuration descriptor */ \
                                pEpDesc = ((USBD_EpDescTypedef*)((uint32_t)pConf + *Sze)); \
                                pEpDesc->bLength            = (uint8_t)sizeof(USBD_EpDescTypedef); \
                                pEpDesc->bDescriptorType    = USB_DESC_TYPE_ENDPOINT; \
                                pEpDesc->bEndpointAddress   = (epadd); \
                                pEpDesc->bmAttributes       = (eptype); \
                                pEpDesc->wMaxPacketSize     = (epsize); \
                                if(pdev->Speed == USBD_HIGH_SPEED) \
                                { \
                                  pEpDesc->bInterval        = (HSinterval); \
                                } \
                                else \
                                { \
                                  pEpDesc->bInterval        = (FSinterval); \
                                } \
                                *Sze += (uint32_t)sizeof(USBD_EpDescTypedef); \
                              } while(0)

#define __USBD_FRAMEWORK_SET_IF(ifnum, alt, eps, class, subclass, protocol, istring) do {\
                                /* Interface Descriptor */ \
                                pIfDesc = ((USBD_IfDescTypedef*)((uint32_t)pConf + *Sze)); \
                                pIfDesc->bLength = (uint8_t)sizeof(USBD_IfDescTypedef); \
                                pIfDesc->bDescriptorType = USB_DESC_TYPE_INTERFACE; \
                                pIfDesc->bInterfaceNumber = (ifnum); \
                                pIfDesc->bAlternateSetting = (alt); \
                                pIfDesc->bNumEndpoints = (eps); \
                                pIfDesc->bInterfaceClass = (class); \
                                pIfDesc->bInterfaceSubClass = (subclass); \
                                pIfDesc->bInterfaceProtocol = (protocol); \
                                pIfDesc->iInterface = (istring); \
                                *Sze += (uint32_t)sizeof(USBD_IfDescTypedef); \
                              } while(0)
#ifdef __cplusplus
}
#endif
#endif  /* __UX_DEVICE_DESCRIPTORS_H__ */
