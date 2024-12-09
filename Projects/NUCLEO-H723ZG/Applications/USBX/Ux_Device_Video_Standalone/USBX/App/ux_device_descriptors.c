/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    ux_device_descriptors.c
  * @author  MCD Application Team
  * @brief   USBX Device descriptor header file
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
  CLASS_TYPE_VIDEO,
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
__ALIGN_BEGIN UCHAR USBD_language_id_framework[LANGUAGE_ID_MAX_LENGTH]
__ALIGN_END = {0};

/* USER CODE BEGIN PV1 */

/* USER CODE END PV1 */

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
                                       uint8_t cfgidx, uint8_t Speed,
                                       uint8_t *pCmpstConfDesc);

static uint8_t USBD_FrameWork_FindFreeIFNbr(USBD_DevClassHandleTypeDef *pdev);

static void USBD_FrameWork_AddConfDesc(uint32_t Conf, uint32_t *pSze);

static void USBD_FrameWork_AssignEp(USBD_DevClassHandleTypeDef *pdev, uint8_t Add,
                                    uint8_t Type, uint32_t Sze);

#if USBD_VIDEO_CLASS_ACTIVATED == 1U
static void USBD_FrameWork_VIDEO_Desc(USBD_DevClassHandleTypeDef *pdev,
                                      uint32_t pConf, uint32_t *Sze);
#endif /* USBD_VIDEO_CLASS_ACTIVATED == 1U */

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

  /* USER CODE END Device_Framework0 */

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
  /* USER CODE BEGIN Device_Framework1 */

  /* USER CODE END Device_Framework1 */
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

  /* USER CODE BEGIN String_Framework0 */

  /* USER CODE END String_Framework0 */

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

  /* USER CODE BEGIN String_Framework1 */

  /* USER CODE END String_Framework1 */

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
  * @brief  USBD_Get_Interface_Number
  *         Return interface number
  * @param  class_type : Device class type
  * @param  interface_type : Device interface type
  * @retval interface number
  */
uint16_t USBD_Get_Interface_Number(uint8_t class_type, uint8_t interface_type)
{
  uint8_t itf_num = 0U;
  uint8_t idx = 0U;

  /* USER CODE BEGIN USBD_Get_Interface_Number0 */

  /* USER CODE END USBD_Get_Interface_Number0 */

  for(idx = 0; idx < USBD_MAX_SUPPORTED_CLASS; idx++)
  {
    if ((USBD_Device_FS.tclasslist[idx].ClassType == class_type) &&
        (USBD_Device_FS.tclasslist[idx].InterfaceType == interface_type))
    {
      itf_num = USBD_Device_FS.tclasslist[idx].Ifs[0];
    }
  }

  /* USER CODE BEGIN USBD_Get_Interface_Number1 */

  /* USER CODE END USBD_Get_Interface_Number1 */

  return itf_num;
}

/**
  * @brief  USBD_Get_Configuration_Number
  *         Return configuration number
  * @param  class_type : Device class type
  * @param  interface_type : Device interface type
  * @retval configuration number
  */
uint16_t USBD_Get_Configuration_Number(uint8_t class_type, uint8_t interface_type)
{
  uint8_t cfg_num = 1U;

  /* USER CODE BEGIN USBD_Get_CONFIGURATION_Number0 */

  /* USER CODE END USBD_Get_CONFIGURATION_Number0 */

  /* USER CODE BEGIN USBD_Get_CONFIGURATION_Number1 */

  /* USER CODE END USBD_Get_CONFIGURATION_Number1 */

  return cfg_num;
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
  static USBD_DeviceDescTypedef   *pDevDesc;
  static USBD_DevQualiDescTypedef *pDevQualDesc;
  uint8_t Idx_Instance = 0U;

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
        (pdev->NumClasses < USBD_MAX_SUPPORTED_CLASS) &&
        (UserClassInstance[Idx_Instance] != CLASS_TYPE_NONE))
    {
      /* Call the composite class builder */
      (void)USBD_FrameWork_AddClass(pdev,
                                    (USBD_CompositeClassTypeDef)UserClassInstance[Idx_Instance],
                                    0, Speed,
                                    (pDevFrameWorkDesc + pdev->CurrDevDescSz));

      /* Increment the ClassId for the next occurrence */
      pdev->classId ++;
      pdev->NumClasses ++;
    }

    Idx_Instance++;
  }

  /* Check if there is a composite class and update device class */
  if (pdev->NumClasses > 1)
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
  * @param  class: type of the class to be added (from USBD_CompositeClassTypeDef)
  * @param  cfgidx: configuration index
  * @param  speed: device speed
  * @param  pCmpstConfDesc: to composite device configuration descriptor
  * @retval status
  */
uint8_t  USBD_FrameWork_AddClass(USBD_DevClassHandleTypeDef *pdev,
                                 USBD_CompositeClassTypeDef class,
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
  * @param  Speed: device speed
  * @param  pCmpstConfDesc: to composite device configuration descriptor
  * @retval status
  */
uint8_t  USBD_FrameWork_AddToConfDesc(USBD_DevClassHandleTypeDef *pdev, uint8_t Speed,
                                      uint8_t *pCmpstConfDesc)
{
  uint8_t interface = 0U;

  /* USER CODE BEGIN FrameWork_AddToConfDesc_0 */

  /* USER CODE END FrameWork_AddToConfDesc_0 */

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

#if USBD_VIDEO_CLASS_ACTIVATED == 1

    case CLASS_TYPE_VIDEO:

      /* Find the first available interface slot and Assign number of interfaces */
      interface = USBD_FrameWork_FindFreeIFNbr(pdev);
      pdev->tclasslist[pdev->classId].NumIf = 2U;
      pdev->tclasslist[pdev->classId].Ifs[0] = interface;
      pdev->tclasslist[pdev->classId].Ifs[1] = (uint8_t)(interface + 1U);

      /* Assign endpoint numbers */
      pdev->tclasslist[pdev->classId].NumEps = 1U; /* EP_IN */

      /* Check the current speed to assign endpoint IN */
      if (pdev->Speed == USBD_HIGH_SPEED)
      {
        /* Assign IN Endpoint */
        USBD_FrameWork_AssignEp(pdev, USBD_VIDEO_EPIN_ADDR,
                                USBD_EP_TYPE_ISOC, USBD_VIDEO_EPIN_HS_MPS);
      }
      else
      {
        /* Assign IN Endpoint */
        USBD_FrameWork_AssignEp(pdev, USBD_VIDEO_EPIN_ADDR,
                                USBD_EP_TYPE_ISOC, USBD_VIDEO_EPIN_FS_MPS);
      }

      /* Configure and Append the Descriptor */
      USBD_FrameWork_VIDEO_Desc(pdev, (uint32_t)pCmpstConfDesc, &pdev->CurrConfDescSz);

      break;

#endif /* USBD_VIDEO_CLASS_ACTIVATED */

    /* USER CODE BEGIN FrameWork_AddToConfDesc_1 */

    /* USER CODE END FrameWork_AddToConfDesc_1 */

    default:
      /* USER CODE BEGIN FrameWork_AddToConfDesc_2 */

      /* USER CODE END FrameWork_AddToConfDesc_2 */
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
  * @brief  USBD_FrameWork_AddConfDesc
  *         Add a new class to the configuration descriptor
  * @param  Conf: configuration descriptor
  * @param  pSze: pointer to the configuration descriptor size
  * @retval none
  */
static void  USBD_FrameWork_AddConfDesc(uint32_t Conf, uint32_t *pSze)
{
  /* Intermediate variable to comply with MISRA-C Rule 11.3 */
  USBD_ConfigDescTypedef *ptr = (USBD_ConfigDescTypedef *)Conf;

  ptr->bLength = (uint8_t)sizeof(USBD_ConfigDescTypedef);
  ptr->bDescriptorType = USB_DESC_TYPE_CONFIGURATION;
  ptr->wDescriptorLength = 0U;
  ptr->bNumInterfaces = 0U;
  ptr->bConfigurationValue = 1U;
  ptr->iConfiguration = USBD_CONFIG_STR_DESC_IDX;
  ptr->bmAttributes = USBD_CONFIG_BMATTRIBUTES;
  ptr->bMaxPower = USBD_CONFIG_MAXPOWER;
  *pSze += sizeof(USBD_ConfigDescTypedef);
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

#if USBD_VIDEO_CLASS_ACTIVATED == 1
/**
  * @brief  USBD_FrameWork_VIDEO_Desc
  *         Configure and Append the VIDEO Descriptor
  * @param  pdev: device instance
  * @param  pConf: Configuration descriptor pointer
  * @param  Sze: pointer to the current configuration descriptor size
  * @retval None
  */
static void USBD_FrameWork_VIDEO_Desc(USBD_DevClassHandleTypeDef *pdev,
                                      uint32_t pConf, uint32_t *Sze)
{

  USBD_IfDescTypedef *pIfDesc;
  USBD_EpDescTypedef *pEpDesc;

#if USBD_COMPOSITE_USE_IAD == 1
  static USBD_IadDescTypedef *pIadDesc;
#endif /* USBD_COMPOSITE_USE_IAD == 1 */

  USBD_VIDEOCSVCIfDescTypeDef *pVideoVCInDesc;
  USBD_VIDEOInputTerminalDescTypeDef *pVideoITDesc;
  USBD_VIDEOOutputTerminalDescTypeDef *pVideoOTDesc;
  USBD_VIDEOVSHeaderDescTypeDef *pVideoSHeaderDesc;
  USBD_VIDEOPayloadFormatDescTypeDef *pVideoPayForDesc;
  USBD_VIDEOFrameDescTypeDef *pVideoFrameDesc;

#if USBD_COMPOSITE_USE_IAD == 1
  pIadDesc = ((USBD_IadDescTypedef *)(pConf + *Sze));
  pIadDesc->bLength = (uint8_t)sizeof(USBD_IadDescTypedef);
  pIadDesc->bDescriptorType = USB_DESC_TYPE_IAD; /* IAD descriptor */
  pIadDesc->bFirstInterface = pdev->tclasslist[pdev->classId].Ifs[0];
  pIadDesc->bInterfaceCount = 2U;    /* 2 interfaces */
  pIadDesc->bFunctionClass = UX_DEVICE_CLASS_VIDEO_CC_VIDEO;
  pIadDesc->bFunctionSubClass = UX_DEVICE_CLASS_VIDEO_SC_INTERFACE_COLLECTION;
  pIadDesc->bFunctionProtocol = UX_DEVICE_CLASS_VIDEO_PC_PROTOCOL_UNDEFINED;
  pIadDesc->iFunction = 0U; /* String Index */
  *Sze += (uint32_t)sizeof(USBD_IadDescTypedef);
#endif /* USBD_COMPOSITE_USE_IAD == 1 */

  /* Append VIDEO Interface descriptor to Configuration descriptor */
  __USBD_FRAMEWORK_SET_IF(pdev->tclasslist[pdev->classId].Ifs[0], 0U, 0U,
                          UX_DEVICE_CLASS_VIDEO_CC_VIDEO,
                          UX_DEVICE_CLASS_VIDEO_SC_CONTROL,
                          UX_DEVICE_CLASS_VIDEO_PC_PROTOCOL_UNDEFINED,
                          0U);

  /* Append Class-specific VC Interface Descriptor to Configuration descriptor*/
  pVideoVCInDesc = ((USBD_VIDEOCSVCIfDescTypeDef *)(pConf + *Sze));
  pVideoVCInDesc->bLength = (uint8_t)sizeof(USBD_VIDEOCSVCIfDescTypeDef);
  pVideoVCInDesc->bDescriptorType = UX_DEVICE_CLASS_VIDEO_CS_INTERFACE;
  pVideoVCInDesc->bDescriptorSubtype = 0x01U;
  pVideoVCInDesc->bcdUVC = 0x0110U;
  pVideoVCInDesc->wTotalLength = 0x001EU;
  pVideoVCInDesc->dwClockFrequency = 0x02DC6C00U;
  pVideoVCInDesc->bInCollection = 0x01U;
  pVideoVCInDesc->aInterfaceNr = 0x01U;
  *Sze += (uint32_t)sizeof(USBD_VIDEOCSVCIfDescTypeDef);

  /*Append Input Terminal Descriptor to Configuration descriptor */
  pVideoITDesc = ((USBD_VIDEOInputTerminalDescTypeDef *)(pConf + *Sze));
  pVideoITDesc->bLength = (uint8_t)sizeof(USBD_VIDEOInputTerminalDescTypeDef);
  pVideoITDesc->bDescriptorType = UX_DEVICE_CLASS_VIDEO_CS_INTERFACE;
  pVideoITDesc->bDescriptorSubtype = 0x02U;
  pVideoITDesc->bTerminalID = 0x01U;
  pVideoITDesc->wTerminalType = 0x0200U;
  pVideoITDesc->bAssocTerminal = 0x00U;
  pVideoITDesc->iTerminal =  0x00U;
  *Sze += (uint32_t)sizeof(USBD_VIDEOInputTerminalDescTypeDef);

  /* Append Output Terminal Descriptor to Configuration descriptor */
  pVideoOTDesc = ((USBD_VIDEOOutputTerminalDescTypeDef *)(pConf + *Sze));
  pVideoOTDesc->bLength = (uint8_t)sizeof(USBD_VIDEOOutputTerminalDescTypeDef);
  pVideoOTDesc->bDescriptorType = UX_DEVICE_CLASS_VIDEO_CS_INTERFACE;
  pVideoOTDesc->bDescriptorSubtype = 0x03U;
  pVideoOTDesc->bTerminalID = 0x02U;
  pVideoOTDesc->wTerminalType = 0x0101U;
  pVideoOTDesc->bAssocTerminal = 0x00U;
  pVideoOTDesc->bSourceID = 0x01U;
  pVideoOTDesc->iTerminal = 0x00U;
  *Sze += (uint32_t)sizeof(USBD_VIDEOOutputTerminalDescTypeDef);

  /* Standard VS (Video Streaming) Interface Descriptor */
  /* Interface 1, Alternate Setting 0 = Zero Bandwidth*/
  __USBD_FRAMEWORK_SET_IF(pdev->tclasslist[pdev->classId].Ifs[1], 0U, 0U,
                          UX_DEVICE_CLASS_VIDEO_CC_VIDEO,
                          UX_DEVICE_CLASS_VIDEO_SC_STREAMING,
                          UX_DEVICE_CLASS_VIDEO_PC_PROTOCOL_UNDEFINED,
                          0U);

  /* Append Class-specific VS Header Descriptor (Input) to Configuration descriptor */
  pVideoSHeaderDesc = ((USBD_VIDEOVSHeaderDescTypeDef *)(pConf + *Sze));
  pVideoSHeaderDesc->bLength = (uint8_t)sizeof(USBD_VIDEOVSHeaderDescTypeDef);
  pVideoSHeaderDesc->bDescriptorType = UX_DEVICE_CLASS_VIDEO_CS_INTERFACE;
  pVideoSHeaderDesc->bDescriptorSubtype = UX_DEVICE_CLASS_VIDEO_VC_HEADER;
  pVideoSHeaderDesc->bNumFormats = 0x01U;
  pVideoSHeaderDesc->wTotalLength = VC_HEADER_SIZE;
  pVideoSHeaderDesc->bEndpointAddress = USBD_VIDEO_EPIN_ADDR;
  pVideoSHeaderDesc->bmInfo = 0x00U;
  pVideoSHeaderDesc->bTerminalLink = 0x02U;
  pVideoSHeaderDesc->bStillCaptureMethod = 0x00U;
  pVideoSHeaderDesc->bTriggerSupport = 0x00U;
  pVideoSHeaderDesc->bTriggerUsage = 0x00U;
  pVideoSHeaderDesc->bControlSize = 0x01U;
  pVideoSHeaderDesc->bmaControls = 0x00U;
  *Sze += (uint32_t)sizeof(USBD_VIDEOVSHeaderDescTypeDef);

  /* Append Payload Format Descriptor to Configuration descriptor */
  pVideoPayForDesc = ((USBD_VIDEOPayloadFormatDescTypeDef *)(pConf + *Sze));
  pVideoPayForDesc->bLength = (uint8_t)sizeof(USBD_VIDEOPayloadFormatDescTypeDef);
  pVideoPayForDesc->bDescriptorType = UX_DEVICE_CLASS_VIDEO_CS_INTERFACE;
  pVideoPayForDesc->bDescriptorSubType = UX_DEVICE_CLASS_VIDEO_VS_FORMAT_MJPEG;
  pVideoPayForDesc->bFormatIndex = 0x01U;
  pVideoPayForDesc->bNumFrameDescriptors = 0x01U;
  pVideoPayForDesc->bmFlags = 0x01U;
  pVideoPayForDesc->bDefaultFrameIndex = 0x01U;
  pVideoPayForDesc->bAspectRatioX = 0x00U;
  pVideoPayForDesc->bAspectRatioY = 0x00U;
  pVideoPayForDesc->bmInterfaceFlag = 0x00U;
  pVideoPayForDesc->bCopyProtect = 0x00U;
  *Sze += (uint32_t)sizeof(USBD_VIDEOPayloadFormatDescTypeDef);

  /* Append Class-specific VS (Video Streaming) Frame Descriptor to
     Configuration descriptor */
  pVideoFrameDesc = ((USBD_VIDEOFrameDescTypeDef *)(pConf + *Sze));
  pVideoFrameDesc->bLength = (uint8_t)sizeof(USBD_VIDEOFrameDescTypeDef);
  pVideoFrameDesc->bDescriptorType = UX_DEVICE_CLASS_VIDEO_CS_INTERFACE;
  pVideoFrameDesc->bDescriptorSubType = UX_DEVICE_CLASS_VIDEO_VS_FRAME_MJPEG;

  pVideoFrameDesc->bFrameIndex = 0x01U;
  pVideoFrameDesc->bmCapabilities = 0x02U;
  pVideoFrameDesc->wWidth = UVC_FRAME_WIDTH;
  pVideoFrameDesc->wHeight = UVC_FRAME_HEIGHT;

  if (pdev->Speed == (uint8_t)USBD_HIGH_SPEED)
  {
    pVideoFrameDesc->dwMinBitRate = UVC_MIN_BIT_RATE(UVC_CAM_FPS_HS);
    pVideoFrameDesc->dwMaxBitRate = UVC_MAX_BIT_RATE(UVC_CAM_FPS_HS);
    pVideoFrameDesc->dwDefaultFrameInterval = UVC_INTERVAL(UVC_CAM_FPS_HS);
    pVideoFrameDesc->dwFrameInterval = UVC_INTERVAL(UVC_CAM_FPS_HS);
  }
  else
  {
    pVideoFrameDesc->dwMinBitRate = UVC_MIN_BIT_RATE(UVC_CAM_FPS_FS);
    pVideoFrameDesc->dwMaxBitRate = UVC_MAX_BIT_RATE(UVC_CAM_FPS_FS);
    pVideoFrameDesc->dwDefaultFrameInterval = UVC_INTERVAL(UVC_CAM_FPS_FS);
    pVideoFrameDesc->dwFrameInterval = UVC_INTERVAL(UVC_CAM_FPS_FS);
  }

  pVideoFrameDesc->dwMaxVideoFrameBufferSize = UVC_MAX_FRAME_SIZE;
  pVideoFrameDesc->bFrameIntervalType = 0x01U;

  *Sze += (uint32_t)sizeof(USBD_VIDEOFrameDescTypeDef);

  /* USB Standard VS Interface  Descriptor - data transfer mode */
  /* Interface 1, Alternate Setting 1*/
  __USBD_FRAMEWORK_SET_IF(pdev->tclasslist[pdev->classId].Ifs[1], 1U, 1U,
                          UX_DEVICE_CLASS_VIDEO_CC_VIDEO,
                          UX_DEVICE_CLASS_VIDEO_SC_STREAMING,
                          UX_DEVICE_CLASS_VIDEO_PC_PROTOCOL_UNDEFINED,
                          0U);

    /* Append Endpoint descriptor to Configuration descriptor */
  __USBD_FRAMEWORK_SET_EP((pdev->tclasslist[pdev->classId].Eps[0].add),
                          (USBD_EP_TYPE_ISOC|USBD_EP_ATTR_ISOC_ASYNC),
                          (uint16_t)(pdev->tclasslist[pdev->classId].Eps[0].size),
                          USBD_VIDEO_EPIN_HS_BINTERVAL,
                          USBD_VIDEO_EPIN_FS_BINTERVAL);

  /* Update Config Descriptor and IAD descriptor */
  ((USBD_ConfigDescTypedef *)pConf)->bNumInterfaces += 2U;
  ((USBD_ConfigDescTypedef *)pConf)->wDescriptorLength = *Sze;
}
#endif /* USBD_VIDEO_CLASS_ACTIVATED == 1 */

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */
