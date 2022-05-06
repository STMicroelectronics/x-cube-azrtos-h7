/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    ux_device_descriptors.c
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

/* Includes ------------------------------------------------------------------*/
#include "ux_device_descriptors.h"

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
USBD_DevClassHandleTypeDef  USBD_Device_FS, USBD_Device_HS;

uint8_t UserClassInstance[USBD_MAX_CLASS_INTERFACES] = {
  CLASS_TYPE_AUDIO,
};

/* The generic device descriptor buffer that will be filled by builder
   Size of the buffer is the maximum possible device FS descriptor size. */
#if defined ( __ICCARM__ ) /* IAR Compiler */
#pragma data_alignment=4
#endif /* defined ( __ICCARM__ ) */
__ALIGN_BEGIN static uint8_t DevFrameWorkDesc_FS[USBD_FRAMEWORK_MAX_DESC_SZ] __ALIGN_END = {0};

/* The generic device descriptor buffer that will be filled by builder
   Size of the buffer is the maximum possible device HS descriptor size. */
#if defined ( __ICCARM__ ) /* IAR Compiler */
#pragma data_alignment=4
#endif /* defined ( __ICCARM__ ) */
__ALIGN_BEGIN static uint8_t DevFrameWorkDesc_HS[USBD_FRAMEWORK_MAX_DESC_SZ] __ALIGN_END = {0};

static uint8_t *pDevFrameWorkDesc_FS = DevFrameWorkDesc_FS;

static uint8_t *pDevFrameWorkDesc_HS = DevFrameWorkDesc_HS;
/* USER CODE BEGIN PV0 */

/* USER CODE END PV0 */

/* String Device Framework :
 Byte 0 and 1 : Word containing the language ID : 0x0904 for US
 Byte 2       : Byte containing the index of the descriptor
 Byte 3       : Byte containing the length of the descriptor string
*/
#if defined ( __ICCARM__ ) /* IAR Compiler */
#pragma data_alignment=4
#endif /* defined ( __ICCARM__ ) */
__ALIGN_BEGIN UCHAR USBD_string_framework[USBD_STRING_FRAMEWORK_MAX_LENGTH]
__ALIGN_END = {0};

/* Multiple languages are supported on the device, to add
   a language besides English, the Unicode language code must
   be appended to the language_id_framework array and the length
   adjusted accordingly. */

#if defined ( __ICCARM__ ) /* IAR Compiler */
#pragma data_alignment=4
#endif /* defined ( __ICCARM__ ) */
UCHAR USBD_language_id_framework[LANGUAGE_ID_MAX_LENGTH] = {0};

/* USER CODE BEGIN PV4 */

/* USER CODE END PV4 */

/* Private function prototypes -----------------------------------------------*/
static void USBD_Desc_GetString(uint8_t *desc, uint8_t *Buffer, uint16_t *len);
static uint8_t USBD_Desc_GetLen(uint8_t *buf);

static uint8_t *USBD_Device_Framework_Builder(USBD_DevClassHandleTypeDef *pdev,
                                              uint8_t *pDevFrameWorkDesc,
                                              uint8_t *UserClassInstance,
                                              uint8_t Speed);

static uint8_t USBD_FrameWork_AddToConfDesc(USBD_DevClassHandleTypeDef *pdev,
                                            uint8_t Speed,
                                            uint8_t *pCmpstConfDesc);

static uint8_t USBD_FrameWork_AddClass(USBD_DevClassHandleTypeDef *pdev,
                                       USBD_CompositeClassTypeDef class,
                                       uint8_t itfProtocol,
                                       uint8_t cfgidx, uint8_t Speed,
                                       uint8_t *pCmpstConfDesc);

static uint8_t USBD_FrameWork_FindFreeIFNbr(USBD_DevClassHandleTypeDef *pdev);

static void USBD_FrameWork_AddConfDesc(uint32_t Conf, uint32_t *pSze);

static void USBD_FrameWork_AssignEp(USBD_DevClassHandleTypeDef *pdev, uint8_t Add,
                                    uint8_t Type, uint32_t Sze);

#if USBD_AUDIO_CLASS_ACTIVATED == 1U
static void  USBD_FrameWork_AUDIO_Desc(USBD_DevClassHandleTypeDef *pdev,
                                       uint32_t pConf, uint32_t *Sze);
#endif /* USBD_AUDIO_CLASS_ACTIVATED == 1U */

/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  USBD_Get_Device_Framework_Speed
  *         Return the device speed descriptor
  * @param  Speed : HIGH or FULL SPEED flag
  * @param  length : length of HIGH or FULL SPEED array
  * @retval Pointer to descriptor buffer
  */
uint8_t *USBD_Get_Device_Framework_Speed(uint8_t Speed, ULONG *Length)
{
  uint8_t *pFrameWork = NULL;
  /* USER CODE BEGIN Device_Framework0 */

  /* USER TAG BEGIN Device_Framework0 */

  if (USBD_FULL_SPEED == Speed)
  {
    USBD_Device_Framework_Builder(&USBD_Device_FS, pDevFrameWorkDesc_FS,
                                  UserClassInstance, Speed);

    /* Get the length of USBD_device_framework_full_speed */
    *Length = (ULONG)(USBD_Device_FS.CurrDevDescSz + USBD_Device_FS.CurrConfDescSz);

    pFrameWork = pDevFrameWorkDesc_FS;
  }
  else
  {
    USBD_Device_Framework_Builder(&USBD_Device_HS, pDevFrameWorkDesc_HS,
                                  UserClassInstance, Speed);

    /* Get the length of USBD_device_framework_high_speed */
    *Length = (ULONG)(USBD_Device_HS.CurrDevDescSz + USBD_Device_HS.CurrConfDescSz);

    pFrameWork = pDevFrameWorkDesc_HS;
  }
  /* USER CODE Device_Framework1 */

  /* USER CODE Device_Framework1 */
  return pFrameWork;
}

/**
  * @brief  USBD_Get_String_Framework
  *         Return the language_id_framework
  * @param  Length : Length of String_Framework
  * @retval Pointer to language_id_framework buffer
  */
uint8_t *USBD_Get_String_Framework(ULONG *Length)
{
  uint16_t len = 0U;
  uint8_t count = 0U;

  /* USER CODE String_Framework0 */

  /* USER CODE String_Framework0 */

  /* Set the Manufacturer language Id and index in USBD_string_framework */
  USBD_string_framework[count++] = USBD_LANGID_STRING & 0xFF;
  USBD_string_framework[count++] = USBD_LANGID_STRING >> 8;
  USBD_string_framework[count++] = USBD_IDX_MFC_STR;

  /* Set the Manufacturer string in string_framework */
  USBD_Desc_GetString((uint8_t *)USBD_MANUFACTURER_STRING, USBD_string_framework + count, &len);

  /* Set the Product language Id and index in USBD_string_framework */
  count += len + 1;
  USBD_string_framework[count++] = USBD_LANGID_STRING & 0xFF;
  USBD_string_framework[count++] = USBD_LANGID_STRING >> 8;
  USBD_string_framework[count++] = USBD_IDX_PRODUCT_STR;

  /* Set the Product string in USBD_string_framework */
  USBD_Desc_GetString((uint8_t *)USBD_PRODUCT_STRING, USBD_string_framework + count, &len);

  /* Set Serial language Id and index in string_framework */
  count += len + 1;
  USBD_string_framework[count++] = USBD_LANGID_STRING & 0xFF;
  USBD_string_framework[count++] = USBD_LANGID_STRING >> 8;
  USBD_string_framework[count++] = USBD_IDX_SERIAL_STR;

  /* Set the Serial number in USBD_string_framework */
  USBD_Desc_GetString((uint8_t *)USBD_SERIAL_NUMBER, USBD_string_framework + count, &len);

  /* USER CODE String_Framework1 */

  /* USER CODE String_Framework1 */

  /* Get the length of USBD_string_framework */
  *Length = strlen((const char *)USBD_string_framework);

  return USBD_string_framework;
}

/**
  * @brief  USBD_Get_Language_Id_Framework
  *         Return the language_id_framework
  * @param  Length : Length of Language_Id_Framework
  * @retval Pointer to language_id_framework buffer
  */
uint8_t *USBD_Get_Language_Id_Framework(ULONG *Length)
{
  uint8_t count = 0U;

  /* Set the language Id in USBD_language_id_framework */
  USBD_language_id_framework[count++] = USBD_LANGID_STRING & 0xFF;
  USBD_language_id_framework[count++] = USBD_LANGID_STRING >> 8;

  /* Get the length of USBD_language_id_framework */
  *Length = strlen((const char *)USBD_language_id_framework);

  return USBD_language_id_framework;
}

/**
  * @brief  USBD_Desc_GetString
  *         Convert ASCII string into Unicode one
  * @param  desc : descriptor buffer
  * @param  Unicode : Formatted string buffer (Unicode)
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
  * @param  buf : pointer to the ASCII string buffer
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

/**
  * @brief  USBD_Device_Framework_Builder
  *         Device Framework builder
  * @param  pdev: device instance
  * @param  pDevFrameWorkDesc: Pointer to the device framework descriptor
  * @param  UserClassInstance: type of the class to be added
  * @param  Speed: Speed parameter HS or FS
  * @retval status
  */
static uint8_t *USBD_Device_Framework_Builder(USBD_DevClassHandleTypeDef *pdev,
                                              uint8_t *pDevFrameWorkDesc,
                                              uint8_t *UserClassInstance,
                                              uint8_t Speed)
{
  static USBD_DeviceDescTypedef *pDevDesc;
  static USBD_DevQualiDescTypedef *pDevQualDesc;
  uint8_t Idx_Instance = 0U;
  uint8_t NumberClass = 1U;
  uint8_t itfProtocol;

  /* Set Dev and conf descriptors size to 0 */
  pdev->CurrConfDescSz = 0U;
  pdev->CurrDevDescSz = 0U;

  /* Set the pointer to the device descriptor area*/
  pDevDesc = (USBD_DeviceDescTypedef *)pDevFrameWorkDesc;

  /* Start building the generic device descriptor common part */
  pDevDesc->bLength = (uint8_t)sizeof(USBD_DeviceDescTypedef);
  pDevDesc->bDescriptorType = UX_DEVICE_DESCRIPTOR_ITEM;
  pDevDesc->bcdUSB = USB_BCDUSB;
  pDevDesc->bDeviceClass = 0x00;
  pDevDesc->bDeviceSubClass = 0x00;
  pDevDesc->bDeviceProtocol = 0x00;
  pDevDesc->bMaxPacketSize = USBD_MAX_EP0_SIZE;
  pDevDesc->idVendor = USBD_VID;
  pDevDesc->idProduct = USBD_PID;
  pDevDesc->bcdDevice = 0x0200;
  pDevDesc->iManufacturer = USBD_IDX_MFC_STR;
  pDevDesc->iProduct = USBD_IDX_PRODUCT_STR;
  pDevDesc->iSerialNumber = USBD_IDX_SERIAL_STR;
  pDevDesc->bNumConfigurations = USBD_MAX_NUM_CONFIGURATION;
  pdev->CurrDevDescSz += (uint32_t)sizeof(USBD_DeviceDescTypedef);

  /* Check if USBx is in high speed mode to add qualifier descriptor */
  if (Speed == USBD_HIGH_SPEED)
  {
    pDevQualDesc = (USBD_DevQualiDescTypedef *)(pDevFrameWorkDesc + pdev->CurrDevDescSz);
    pDevQualDesc->bLength = (uint8_t)sizeof(USBD_DevQualiDescTypedef);
    pDevQualDesc->bDescriptorType = UX_DEVICE_QUALIFIER_DESCRIPTOR_ITEM;
    pDevQualDesc->bcdDevice = 0x0200;
    pDevQualDesc->Class = 0x00;
    pDevQualDesc->SubClass = 0x00;
    pDevQualDesc->Protocol = 0x00;
    pDevQualDesc->bMaxPacketSize = 0x40;
    pDevQualDesc->bNumConfigurations = 0x01;
    pDevQualDesc->bReserved = 0x00;
    pdev->CurrDevDescSz += (uint32_t)sizeof(USBD_DevQualiDescTypedef);
  }

  /* Build the device framework */
  while (Idx_Instance < USBD_MAX_SUPPORTED_CLASS)
  {
    if ((pdev->classId < USBD_MAX_SUPPORTED_CLASS) &&
        (pdev->NumClasses < USBD_MAX_SUPPORTED_CLASS))
    {
      itfProtocol = 0U;

      /* Call the composite class builder */
      (void)USBD_FrameWork_AddClass(pdev,
                                    (USBD_CompositeClassTypeDef)UserClassInstance[Idx_Instance],
                                    itfProtocol,
                                    0, Speed, (pDevFrameWorkDesc + pdev->CurrDevDescSz));

      /* Increment the ClassId for the next occurrence */
      pdev->classId ++;
      pdev->NumClasses ++;
    }
    Idx_Instance++;

    /* Count the number of Classes different of CLASS_TYPE_NONE */
    if (UserClassInstance[Idx_Instance] != CLASS_TYPE_NONE)
    {
      NumberClass++;
    }
  }
  /* Check if there is a composite class and update device class */
  if (NumberClass > 1)
  {
    pDevDesc->bDeviceClass = 0xEF;
    pDevDesc->bDeviceSubClass = 0x02;
    pDevDesc->bDeviceProtocol = 0x01;
  }
  else
  {
    /* Check if the CDC ACM class is set and update device class */
    if (UserClassInstance[0] == CLASS_TYPE_CDC_ACM)
    {
      pDevDesc->bDeviceClass = 0x02;
      pDevDesc->bDeviceSubClass = 0x02;
      pDevDesc->bDeviceProtocol = 0x00;
    }
  }

  return pDevFrameWorkDesc;
}

/**
  * @brief  USBD_FrameWork_AddClass
  *         Register a class in the class builder
  * @param  pdev: device instance
  * @param  pclass: pointer to the class structure to be added
  * @param  class: type of the class to be added (from USBD_CompositeClassTypeDef)
  * @param  cfgidx: configuration index
  * @retval status
  */
uint8_t  USBD_FrameWork_AddClass(USBD_DevClassHandleTypeDef *pdev,
                                 USBD_CompositeClassTypeDef class,
                                 uint8_t itfProtocol,
                                 uint8_t cfgidx, uint8_t Speed,
                                 uint8_t *pCmpstConfDesc)
{
  if ((pdev->classId < USBD_MAX_SUPPORTED_CLASS) &&
      (pdev->tclasslist[pdev->classId].Active == 0U))
  {
    /* Store the class parameters in the global tab */
    pdev->tclasslist[pdev->classId].ClassId = pdev->classId;
    pdev->tclasslist[pdev->classId].Active = 1U;
    pdev->tclasslist[pdev->classId].ClassType = class;

    /* Call configuration descriptor builder and endpoint configuration builder */
    if (USBD_FrameWork_AddToConfDesc(pdev, Speed, pCmpstConfDesc) != UX_SUCCESS)
    {
      return UX_ERROR;
    }
  }

  UNUSED(cfgidx);

  return UX_SUCCESS;
}

/**
  * @brief  USBD_FrameWork_AddToConfDesc
  *         Add a new class to the configuration descriptor
  * @param  pdev: device instance
  * @retval status
  */
uint8_t  USBD_FrameWork_AddToConfDesc(USBD_DevClassHandleTypeDef *pdev, uint8_t Speed,
                                      uint8_t *pCmpstConfDesc)
{
  uint8_t interface = 0U;

  /* USER CODE FrameWork_AddToConfDesc_0 */

  /* USER CODE FrameWork_AddToConfDesc_0 */

  /* The USB drivers do not set the speed value, so set it here before starting */
  pdev->Speed = Speed;

  /* start building the config descriptor common part */
  if (pdev->classId == 0U)
  {
    /* Add configuration and IAD descriptors */
    USBD_FrameWork_AddConfDesc((uint32_t)pCmpstConfDesc, &pdev->CurrConfDescSz);
  }

  switch (pdev->tclasslist[pdev->classId].ClassType)
  {

#if USBD_AUDIO_CLASS_ACTIVATED == 1
    case CLASS_TYPE_AUDIO:

      /* Find the first available interface slot and Assign number of interfaces */
      interface = USBD_FrameWork_FindFreeIFNbr(pdev);
      pdev->tclasslist[pdev->classId].NumIf = 2U;
      pdev->tclasslist[pdev->classId].Ifs[0] = interface;
      pdev->tclasslist[pdev->classId].Ifs[1] = (uint8_t)(interface + 1U);

      /* Assign endpoint numbers */
      pdev->tclasslist[pdev->classId].NumEps = 1U; /* EP1_OUT */

      /* Check the current speed to assign endpoint IN */
      if (pdev->Speed == USBD_HIGH_SPEED)
      {
        /* Assign OUT Endpoint */
        USBD_FrameWork_AssignEp(pdev, USBD_AUDIO_EPOUT_ADDR, USBD_EP_TYPE_ISOC|USBD_EP_ATTR_ISOC_ASYNC,
                                USBD_AUDIO_EPIN_HS_MPS);
      }
      else
      {
        /* Assign OUT Endpoint */
        USBD_FrameWork_AssignEp(pdev, USBD_AUDIO_EPOUT_ADDR, USBD_EP_TYPE_ISOC|USBD_EP_ATTR_ISOC_ASYNC,
                                USBD_AUDIO_EPIN_FS_MPS);
      }

      /* Configure and Append the Descriptor */
      USBD_FrameWork_AUDIO_Desc(pdev, (uint32_t)pCmpstConfDesc, &pdev->CurrConfDescSz);

      break;
#endif /* USBD_AUDIO_CLASS_ACTIVATED */

    /* USER CODE FrameWork_AddToConfDesc_1 */

    /* USER CODE FrameWork_AddToConfDesc_1 */

    default:
      /* USER CODE FrameWork_AddToConfDesc_2 */

      /* USER CODE FrameWork_AddToConfDesc_2 */
      break;
  }

  return UX_SUCCESS;
}

/**
  * @brief  USBD_FrameWork_FindFreeIFNbr
  *         Find the first interface available slot
  * @param  pdev: device instance
  * @retval The interface number to be used
  */
static uint8_t USBD_FrameWork_FindFreeIFNbr(USBD_DevClassHandleTypeDef *pdev)
{
  uint32_t idx = 0U;

  /* Unroll all already activated classes */
  for (uint32_t i = 0U; i < pdev->NumClasses; i++)
  {
    /* Unroll each class interfaces */
    for (uint32_t j = 0U; j < pdev->tclasslist[i].NumIf; j++)
    {
      /* Increment the interface counter index */
      idx++;
    }
  }

  /* Return the first available interface slot */
  return (uint8_t)idx;
}

/**
  * @brief  USBD_FrameWork_AddToConfDesc
  *         Add a new class to the configuration descriptor
  * @param  pdev: device instance
  * @retval none
  */
static void  USBD_FrameWork_AddConfDesc(uint32_t Conf, uint32_t *pSze)
{
  /* Intermediate variable to comply with MISRA-C Rule 11.3 */
  USBD_ConfigDescTypedef *ptr = (USBD_ConfigDescTypedef *)Conf;

  ptr->bLength                = (uint8_t)sizeof(USBD_ConfigDescTypedef);
  ptr->bDescriptorType        = USB_DESC_TYPE_CONFIGURATION;
  ptr->wDescriptorLength      = 0U;
  ptr->bNumInterfaces         = 0U;
  ptr->bConfigurationValue    = 1U;
  ptr->iConfiguration         = USBD_CONFIG_STR_DESC_IDX;
  ptr->bmAttributes           = USBD_CONFIG_BMATTRIBUTES;
  ptr->bMaxPower              = USBD_CONFIG_MAXPOWER;
  *pSze                       += sizeof(USBD_ConfigDescTypedef);
}

/**
  * @brief  USBD_FrameWork_AssignEp
  *         Assign and endpoint
  * @param  pdev: device instance
  * @param  Add: Endpoint address
  * @param  Type: Endpoint type
  * @param  Sze: Endpoint max packet size
  * @retval none
  */
static void  USBD_FrameWork_AssignEp(USBD_DevClassHandleTypeDef *pdev,
                                     uint8_t Add, uint8_t Type, uint32_t Sze)
{
  uint32_t idx = 0U;

  /* Find the first available endpoint slot */
  while (((idx < (pdev->tclasslist[pdev->classId]).NumEps) && \
          ((pdev->tclasslist[pdev->classId].Eps[idx].is_used) != 0U)))
  {
    /* Increment the index */
    idx++;
  }

  /* Configure the endpoint */
  pdev->tclasslist[pdev->classId].Eps[idx].add = Add;
  pdev->tclasslist[pdev->classId].Eps[idx].type = Type;
  pdev->tclasslist[pdev->classId].Eps[idx].size = (uint16_t) Sze;
  pdev->tclasslist[pdev->classId].Eps[idx].is_used = 1U;
}

#if USBD_AUDIO_CLASS_ACTIVATED == 1
/**
  * @brief  USBD_FrameWork_AUDIO_Desc
  *         Configure and Append the AUDIO Descriptor
  * @param  pdev: device instance
  * @param  pConf: Configuration descriptor pointer
  * @param  Sze: pointer to the current configuration descriptor size
  * @retval None
  */
static void USBD_FrameWork_AUDIO_Desc(USBD_DevClassHandleTypeDef *pdev,
                                      uint32_t pConf, uint32_t *Sze)
{

  static USBD_IfDescTypedef                    *pIfDesc;
  static USBD_EpDescTypedef                    *pEpDesc;

#if USBD_COMPOSITE_USE_IAD == 1
  static USBD_IadDescTypedef                   *pIadDesc;
#endif /* USBD_COMPOSITE_USE_IAD == 1 */

  static USBD_AUDIOCCSIfDescTypeDef            *pSpeakerACCSIfDesc;
  static USBD_AUDIOSCSIfDescTypeDef            *pSpeakerASCSIfDesc;
  static USBD_AUDIOSFormatIfDescTypeDef        *pSpeakerASFormatDesc;
  static USBD_AUDIOSCSEpDescTypeDef            *pSpeakerASCSEpDesc;
  static USBD_AUDIOClockSourceDescTypeDef      *pSpeakerCSDesc;
  static USBD_AUDIOInputTerminalDescTypeDef    *pSpeakerITDesc;
  static USBD_AUDIOFeatureUnitPlayDescTypeDef  *pSpeakerFUDesc;
  static USBD_AUDIOOutputTerminalDescTypeDef   *pSpeakerOTDesc;

#if USBD_COMPOSITE_USE_IAD == 1
  pIadDesc                          = ((USBD_IadDescTypedef *)(pConf + *Sze));
  pIadDesc->bLength                 = (uint8_t)sizeof(USBD_IadDescTypedef);
  pIadDesc->bDescriptorType         = USB_DESC_TYPE_IAD; /* IAD descriptor */
  pIadDesc->bFirstInterface         = pdev->tclasslist[pdev->classId].Ifs[0];
  pIadDesc->bInterfaceCount         = 0x02U;
  pIadDesc->bFunctionClass          = UX_DEVICE_CLASS_AUDIO_FUNCTION_CLASS;
  pIadDesc->bFunctionSubClass       = UX_DEVICE_CLASS_AUDIO_FUNCTION_SUBCLASS_UNDEFINED;
  pIadDesc->bFunctionProtocol       = UX_DEVICE_CLASS_AUDIO_FUNCTION_PROTOCOL_VERSION_02_00;
  pIadDesc->iFunction               = 0U; /* String Index */
  *Sze                             += (uint32_t)sizeof(USBD_IadDescTypedef);
#endif /* USBD_COMPOSITE_USE_IAD == 1 */

  /* Append AUDIO Interface descriptor to Configuration descriptor */
  __USBD_FRAMEWORK_SET_IF(pdev->tclasslist[pdev->classId].Ifs[0], 0x00U, 0x00U,
                          UX_DEVICE_CLASS_AUDIO_CLASS, \
                          UX_DEVICE_CLASS_AUDIO_SUBCLASS_CONTROL,
                          UX_DEVICE_CLASS_AUTIO_PROTOCOL_VERSION_02_00,
                          0x00U);

  /* Append AUDIO USB Speaker Class-specific AC Interface descriptor to Configuration descriptor */
  pSpeakerACCSIfDesc = ((USBD_AUDIOCCSIfDescTypeDef *)(pConf + *Sze));
  pSpeakerACCSIfDesc->bLength = (uint8_t)sizeof(USBD_AUDIOCCSIfDescTypeDef);
  pSpeakerACCSIfDesc->bDescriptorType = UX_DEVICE_CLASS_AUDIO_CS_INTERFACE;
  pSpeakerACCSIfDesc->bDescriptorSubtype = UX_DEVICE_CLASS_AUDIO_AC_HEADER;
  pSpeakerACCSIfDesc->bcdADC = 0x0200U;
  pSpeakerACCSIfDesc->bCategory = UX_DEVICE_CLASS_AUDIO20_CATEGORY_DESKTOP_SPEAKER;
  pSpeakerACCSIfDesc->wTotalLength = USBD_CONFIG_DESCRIPTOR_AC_TOTAL_SIZE;
  pSpeakerACCSIfDesc->bmControls = 0x00;
  *Sze += (uint32_t)sizeof(USBD_AUDIOCCSIfDescTypeDef);

  /* Append USB Speaker Clock Source Descriptor to Configuration descriptor*/
  pSpeakerCSDesc = ((USBD_AUDIOClockSourceDescTypeDef *)(pConf + *Sze));
  pSpeakerCSDesc->bLength = (uint8_t)sizeof(USBD_AUDIOClockSourceDescTypeDef);
  pSpeakerCSDesc->bDescriptorType = UX_DEVICE_CLASS_AUDIO_CS_INTERFACE;
  pSpeakerCSDesc->bDescriptorSubtype = UX_DEVICE_CLASS_AUDIO20_AC_CLOCK_SOURCE;
  pSpeakerCSDesc->bClockID = USB_AUDIO_CONFIG_PLAY_CLOCK_SOURCE_ID;
  pSpeakerCSDesc->bmAttributes = 0x01U;
  pSpeakerCSDesc->bmControls = 0x01U;
  pSpeakerCSDesc->bAssocTerminal = 0x00U;
  pSpeakerCSDesc->iClockSource = 0x00U;
  *Sze += (uint32_t)sizeof(USBD_AUDIOClockSourceDescTypeDef);

  /* Append USB Speaker Input Terminal Descriptor to Configuration descriptor*/
  pSpeakerITDesc = ((USBD_AUDIOInputTerminalDescTypeDef *)(pConf + *Sze));
  pSpeakerITDesc->bLength = (uint8_t)sizeof(USBD_AUDIOInputTerminalDescTypeDef);
  pSpeakerITDesc->bDescriptorType = UX_DEVICE_CLASS_AUDIO_CS_INTERFACE;
  pSpeakerITDesc->bDescriptorSubtype = UX_DEVICE_CLASS_AUDIO_AC_INPUT_TERMINAL;
  pSpeakerITDesc->bTerminalID = USB_AUDIO_CONFIG_PLAY_TERMINAL_INPUT_ID;
  pSpeakerITDesc->wTerminalType = UX_DEVICE_CLASS_AUDIO_USB_STREAMING;
  pSpeakerITDesc->bAssocTerminal = 0x00U;
  pSpeakerITDesc->bCSourceID = USB_AUDIO_CONFIG_PLAY_CLOCK_SOURCE_ID;
  pSpeakerITDesc->bNrChannels = USB_AUDIO_CONFIG_PLAY_CHANNEL_COUNT;
  pSpeakerITDesc->bmChannelConfig = 0x00000003U;
  pSpeakerITDesc->iChannelNames = 0x00U;
  pSpeakerITDesc->bmControls = 0x0000U;
  pSpeakerITDesc->iTerminal = 0x00U;
  *Sze += (uint32_t)sizeof(USBD_AUDIOInputTerminalDescTypeDef);

  /*Append USB Speaker Audio Feature Unit Descriptor to Configuration descriptor */
  pSpeakerFUDesc = ((USBD_AUDIOFeatureUnitPlayDescTypeDef *)(pConf + *Sze));
  pSpeakerFUDesc->bLength = (uint8_t)sizeof(USBD_AUDIOFeatureUnitPlayDescTypeDef);
  pSpeakerFUDesc->bDescriptorType = UX_DEVICE_CLASS_AUDIO_CS_INTERFACE;
  pSpeakerFUDesc->bDescriptorSubtype = UX_DEVICE_CLASS_AUDIO_AC_FEATURE_UNIT;
  pSpeakerFUDesc->bUnitID = USB_AUDIO_CONFIG_PLAY_UNIT_FEATURE_ID;
  pSpeakerFUDesc->bSourceID = USB_AUDIO_CONFIG_PLAY_TERMINAL_INPUT_ID;
  pSpeakerFUDesc->bmaControls[0] = USBD_AUDIO_FU_CONTROL_MUTE|USBD_AUDIO_FU_CONTROL_VOLUME;
  pSpeakerFUDesc->bmaControls[1] = 0x00000000U;
  pSpeakerFUDesc->bmaControls[2] = 0x00000000U;
  pSpeakerFUDesc->iFeature = 0x00;
  *Sze += (uint32_t)sizeof(USBD_AUDIOFeatureUnitPlayDescTypeDef);

  /*Append USB Speaker Output Terminal Descriptor to Configuration descriptor*/
  pSpeakerOTDesc = ((USBD_AUDIOOutputTerminalDescTypeDef *)(pConf + *Sze));
  pSpeakerOTDesc->bLength = (uint8_t)sizeof(USBD_AUDIOOutputTerminalDescTypeDef);
  pSpeakerOTDesc->bDescriptorType = UX_DEVICE_CLASS_AUDIO_CS_INTERFACE;
  pSpeakerOTDesc->bDescriptorSubtype = UX_DEVICE_CLASS_AUDIO_AC_OUTPUT_TERMINAL;
  pSpeakerOTDesc->bTerminalID = USB_AUDIO_CONFIG_PLAY_TERMINAL_OUTPUT_ID;
  pSpeakerOTDesc->wTerminalType = UX_DEVICE_CLASS_AUDIO_SPEAKER;
  pSpeakerOTDesc->bAssocTerminal = 0x00U;
  pSpeakerOTDesc->bSourceID = USB_AUDIO_CONFIG_PLAY_UNIT_FEATURE_ID;
  pSpeakerOTDesc->bCSourceID = USB_AUDIO_CONFIG_PLAY_CLOCK_SOURCE_ID;
  pSpeakerOTDesc->bmaControls = 0x0000U;
  pSpeakerOTDesc->iTerminal = 0x00U;
  *Sze += (uint32_t)sizeof(USBD_AUDIOOutputTerminalDescTypeDef);

  /* USB Speaker Standard AS Interface Descriptor - Audio Streaming Zero Bandwidth */
  /* Interface 1, Alternate Setting 0*/
  __USBD_FRAMEWORK_SET_IF(pdev->tclasslist[pdev->classId].Ifs[USBD_AUDIO_AS_PLAY_INTERFACE],
                          0x00U,
                          0x00U,
                          UX_DEVICE_CLASS_AUDIO_CLASS, \
                          UX_DEVICE_CLASS_AUDIO_SUBCLASS_AUDIOSTREAMING,
                          UX_DEVICE_CLASS_AUTIO_PROTOCOL_VERSION_02_00,
                          0x00U);

  /* USB Speaker Standard AS Interface Descriptor -Audio Streaming Operational */
  /* Interface 1, Alternate Setting 1*/
  __USBD_FRAMEWORK_SET_IF(pdev->tclasslist[pdev->classId].Ifs[USBD_AUDIO_AS_PLAY_INTERFACE],
                          0x01U,
                          0x01U,
                          UX_DEVICE_CLASS_AUDIO_CLASS, \
                          UX_DEVICE_CLASS_AUDIO_SUBCLASS_AUDIOSTREAMING,
                          UX_DEVICE_CLASS_AUTIO_PROTOCOL_VERSION_02_00,
                          0x00U);

  /* USB Speaker Audio Streaming Class-Specific Interface Descriptor */
  pSpeakerASCSIfDesc = ((USBD_AUDIOSCSIfDescTypeDef *)(pConf + *Sze));
  pSpeakerASCSIfDesc->bLength = (uint8_t)sizeof(USBD_AUDIOSCSIfDescTypeDef);
  pSpeakerASCSIfDesc->bDescriptorType = UX_DEVICE_CLASS_AUDIO_CS_INTERFACE;
  pSpeakerASCSIfDesc->bDescriptorSubtype = UX_DEVICE_CLASS_AUDIO_AS_GENERAL;
  pSpeakerASCSIfDesc->bTerminalLink = USB_AUDIO_CONFIG_PLAY_TERMINAL_INPUT_ID;
  pSpeakerASCSIfDesc->bmControls = 0x00U;
  pSpeakerASCSIfDesc->bFormatType = UX_DEVICE_CLASS_AUDIO_FORMAT_TYPE_I;
  pSpeakerASCSIfDesc->bmFormats = 0x00000001U;
  pSpeakerASCSIfDesc->bNrChannels = USB_AUDIO_CONFIG_PLAY_CHANNEL_COUNT;
  pSpeakerASCSIfDesc->bmChannelConfig = USB_AUDIO_CONFIG_PLAY_CHANNEL_MAP;
  pSpeakerASCSIfDesc->iChannelNames = 0x0000U;
  *Sze += (uint32_t)sizeof(USBD_AUDIOSCSIfDescTypeDef);

  /* USB Speaker Audio Format Interface Descriptor */
  pSpeakerASFormatDesc = ((USBD_AUDIOSFormatIfDescTypeDef *)(pConf + *Sze));
  pSpeakerASFormatDesc->bLength = (uint8_t)sizeof(USBD_AUDIOSFormatIfDescTypeDef);
  pSpeakerASFormatDesc->bDescriptorType = UX_DEVICE_CLASS_AUDIO_CS_INTERFACE;
  pSpeakerASFormatDesc->bDescriptorSubtype = UX_DEVICE_CLASS_AUDIO_AS_FORMAT_TYPE;
  pSpeakerASFormatDesc->bFormatType = UX_DEVICE_CLASS_AUDIO_FORMAT_TYPE_I;
  pSpeakerASFormatDesc->bSubslotSize = USB_AUDIO_CONFIG_PLAY_RES_BYTE;
  pSpeakerASFormatDesc->bBitResolution = USB_AUDIO_CONFIG_PLAY_RES_BIT;
  *Sze += (uint32_t)sizeof(USBD_AUDIOSFormatIfDescTypeDef);

  /* Append Endpoint descriptor to Configuration descriptor */
  __USBD_FRAMEWORK_SET_EP((pdev->tclasslist[pdev->classId].Eps[0].add),
                          (USBD_EP_TYPE_ISOC|USBD_EP_ATTR_ISOC_SYNC),
                          (uint16_t)(pdev->tclasslist[pdev->classId].Eps[0].size),
                          (0x01U), (0x01U));

  /* Class-Specific AS Isochronous Audio Data Endpoint Descriptor*/
  pSpeakerASCSEpDesc = ((USBD_AUDIOSCSEpDescTypeDef *)(pConf + *Sze));
  pSpeakerASCSEpDesc->bLength = (uint8_t)sizeof(USBD_AUDIOSCSEpDescTypeDef);
  pSpeakerASCSEpDesc->bDescriptorType = UX_DEVICE_CLASS_AUDIO_CS_ENDPOINT;
  pSpeakerASCSEpDesc->bDescriptorSubtype = UX_DEVICE_CLASS_AUDIO_EP_GENERAL;
  pSpeakerASCSEpDesc->bmAttributes = 0x00U;
  pSpeakerASCSEpDesc->bmControls = 0x00U;
  pSpeakerASCSEpDesc->bLockDelayUnits = 0x00U;
  pSpeakerASCSEpDesc->wLockDelay = 0x0000U;
  *Sze += (uint32_t)sizeof(USBD_AUDIOSCSEpDescTypeDef);

  /* Update Config Descriptor and IAD descriptor */
  ((USBD_ConfigDescTypedef *)pConf)->bNumInterfaces += 2U;
  ((USBD_ConfigDescTypedef *)pConf)->wDescriptorLength = *Sze;
}
#endif /* USBD_AUDIO_CLASS_ACTIVATED == 1 */

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */
