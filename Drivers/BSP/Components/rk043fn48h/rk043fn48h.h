/**
  ******************************************************************************
  * @file    rk043fn48h.h
  * @author  MCD Application Team
  * @brief   This file contains all the constants parameters for the RK043FN48H-CT672B
  *          LCD component.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2015-2024 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __RK043FN48H_H
#define __RK043FN48H_H

#ifdef __cplusplus
 extern "C" {
#endif 

/* Includes ------------------------------------------------------------------*/  

/** @addtogroup BSP
  * @{
  */ 

/** @addtogroup Components
  * @{
  */ 
  
/** @addtogroup rk043fn48h
  * @{
  */

/** @defgroup RK043FN48H_Exported_Types
  * @{
  */
   
/**
  * @}
  */ 

/** @defgroup RK043FN48H_Exported_Constants
  * @{
  */

/** 
  * @brief  RK043FN48H Size  
  */     
#define  RK043FN48H_WIDTH    ((uint16_t)480)          /* LCD PIXEL WIDTH            */
#define  RK043FN48H_HEIGHT   ((uint16_t)272)          /* LCD PIXEL HEIGHT           */

/** 
  * @brief  RK043FN48H Timing  
  */     
#define  RK043FN48H_HSYNC            ((uint16_t)41)   /* Horizontal synchronization */
#define  RK043FN48H_HBP              ((uint16_t)13)   /* Horizontal back porch      */
#define  RK043FN48H_HFP              ((uint16_t)32)   /* Horizontal front porch     */
#define  RK043FN48H_VSYNC            ((uint16_t)10)   /* Vertical synchronization   */
#define  RK043FN48H_VBP              ((uint16_t)2)    /* Vertical back porch        */
#define  RK043FN48H_VFP              ((uint16_t)2)    /* Vertical front porch       */

/** 
  * @brief  RK043FN48H frequency divider  
  */    
#define  RK043FN48H_FREQUENCY_DIVIDER    5            /* LCD Frequency divider      */
/**
  * @}
  */
  
/** @defgroup RK043FN48H_Exported_Functions
  * @{
  */

/**
  * @}
  */    
#ifdef __cplusplus
}
#endif

#endif /* __RK043FN48H_H */
/**
  * @}
  */ 

/**
  * @}
  */ 

/**
  * @}
  */ 

