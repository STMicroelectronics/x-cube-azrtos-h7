#ifndef _WHD_CONFIG_TEMPLATE_H_
#define _WHD_CONFIG_TEMPLATE_H_

#include "stm32f7xx.h"
#include "stm32f769xx.h"
#include "net_connect.h"

#define WHD_FREE           NET_FREE
#define WHD_MALLOC         NET_MALLOC
#define WHD_CALLOC(A, B)   NET_CALLOC((A), (B))
#define WLAN_BUS_TYPE_SDIO



#define WHD_COUNTRY     WHD_COUNTRY_AUSTRALIA

/* Internal thread config    */
#define WHD_THREAD_STACK_SIZE   5120
#define WHD_THREAD_PRIORITY     osPriorityNormal

/* please define your configuration , either SDIO or SPI */
#define CY_WHD_CONFIG_USE_SDIO
/* #define CY_WHD_CONFIG_USE_SPI */

/* Set Timeout for your platform */
#define WLAN_POWER_UP_DELAY_MS                   250
#define SDIO_ENUMERATION_TIMEOUT_MS              500



#ifdef STM32F412xG
#include "stm32f4xx.h"


#define WIFI_WL_REG_ON {GPIOC,{.Pin= GPIO_PIN_13, .Mode=GPIO_MODE_OUTPUT_PP, .Pull=GPIO_NOPULL, .Speed= GPIO_SPEED_FREQ_LOW}}
#define WIFI_32K_CLK   {GPIOA,{.Pin= GPIO_PIN_8 , .Mode=GPIO_MODE_AF_PP , .Pull=GPIO_NOPULL , .Speed= GPIO_SPEED_FREQ_LOW      , .Alternate=GPIO_AF0_MCO}}
#define WIFI_SDIO_CMD  {GPIOD,{.Pin= GPIO_PIN_2 , .Mode=GPIO_MODE_AF_PP , .Pull=GPIO_PULLUP , .Speed= GPIO_SPEED_FREQ_VERY_HIGH, .Alternate=GPIO_AF12_SDIO}}
#define WIFI_SDIO_CLK  {GPIOC,{.Pin= GPIO_PIN_12, .Mode=GPIO_MODE_AF_PP , .Pull=GPIO_PULLUP , .Speed= GPIO_SPEED_FREQ_VERY_HIGH, .Alternate=GPIO_AF12_SDIO}}
#define WIFI_SDIO_D0   {GPIOC,{.Pin= GPIO_PIN_8 , .Mode=GPIO_MODE_AF_PP , .Pull=GPIO_PULLUP , .Speed= GPIO_SPEED_FREQ_VERY_HIGH, .Alternate=GPIO_AF12_SDIO}}
#define WIFI_SDIO_D1   {GPIOC,{.Pin= GPIO_PIN_9 , .Mode=GPIO_MODE_AF_PP , .Pull=GPIO_PULLUP , .Speed= GPIO_SPEED_FREQ_VERY_HIGH, .Alternate=GPIO_AF12_SDIO}}
#define WIFI_SDIO_D2   {GPIOC,{.Pin= GPIO_PIN_10, .Mode=GPIO_MODE_AF_PP , .Pull=GPIO_PULLUP , .Speed= GPIO_SPEED_FREQ_VERY_HIGH, .Alternate=GPIO_AF12_SDIO}}
#define WIFI_SDIO_D3   {GPIOC,{.Pin= GPIO_PIN_11, .Mode=GPIO_MODE_AF_PP , .Pull=GPIO_PULLUP , .Speed= GPIO_SPEED_FREQ_VERY_HIGH, .Alternate=GPIO_AF12_SDIO}}

/* Out of band pin , not defined for murata 1LD */
/* #define SDIO_OOB_IRQ  {GPIOC,{.Pin= GPIO_PIN_1 , .Mode=GPIO_MODE_INPUT     , .Pull=GPIO_NOPULL , .Speed= GPIO_SPEED_FREQ_VERY_HIGH}} */

#endif /* STM32F412xG */


#ifdef STM32H753xx
#include "stm32h7xx.h"

#define WIFI_WL_REG_ON  {GPIOD,{.Pin= GPIO_PIN_3, .Mode=GPIO_MODE_OUTPUT_PP , .Pull=GPIO_NOPULL , .Speed= GPIO_SPEED_FREQ_HIGH}
/*#define WIFI_32K_CLK  {GPIOA,{.Pin= GPIO_PIN_8, .Mode=GPIO_MODE_AF_PP , .Pull=GPIO_NOPULL , .Speed= GPIO_SPEED_FREQ_LOW , .Alternate=GPIO_AF0_MCO}} */

#define WIFI_SDIO_CMD   {GPIOD,{.Pin= GPIO_PIN_2 , .Mode=GPIO_MODE_AF_PP , .Pull=GPIO_PULLUP , .Speed= GPIO_SPEED_FREQ_VERY_HIGH, .Alternate=GPIO_AF12_SDMMC1}}
#define WIFI_SDIO_CLK   {GPIOC,{.Pin= GPIO_PIN_12, .Mode=GPIO_MODE_AF_PP , .Pull=GPIO_PULLUP , .Speed= GPIO_SPEED_FREQ_VERY_HIGH, .Alternate=GPIO_AF12_SDMMC1}}
#define WIFI_SDIO_D0    {GPIOC,{.Pin= GPIO_PIN_8 , .Mode=GPIO_MODE_AF_PP , .Pull=GPIO_PULLUP , .Speed= GPIO_SPEED_FREQ_VERY_HIGH, .Alternate=GPIO_AF12_SDMMC1}}
#define WIFI_SDIO_D1    {GPIOC,{.Pin= GPIO_PIN_9 , .Mode=GPIO_MODE_AF_PP , .Pull=GPIO_PULLUP , .Speed= GPIO_SPEED_FREQ_VERY_HIGH, .Alternate=GPIO_AF12_SDMMC1}}
#define WIFI_SDIO_D2    {GPIOC,{.Pin= GPIO_PIN_10, .Mode=GPIO_MODE_AF_PP , .Pull=GPIO_PULLUP , .Speed= GPIO_SPEED_FREQ_VERY_HIGH, .Alternate=GPIO_AF12_SDMMC1}}
#define WIFI_SDIO_D3    {GPIOC,{.Pin= GPIO_PIN_11, .Mode=GPIO_MODE_AF_PP , .Pull=GPIO_PULLUP , .Speed= GPIO_SPEED_FREQ_VERY_HIGH, .Alternate=GPIO_AF12_SDMMC1}}
#define WIFI_SDIO_OOB_IRQ {GPIOD,{.Pin= GPIO_PIN_9,  .Mode=GPIO_MODE_IT_RISING , .Pull=GPIO_NOPULL , .Speed= GPIO_SPEED_FREQ_VERY_HIGH, .Alternate=GPIO_AF12_SDMMC1}}

#endif /* STM32H753xx */


#ifdef STM32H747xx
#include "stm32h7xx.h"

/* power pin */
#define WIFI_WL_REG_ON    {GPIOB,{.Pin= GPIO_PIN_8, .Mode=GPIO_MODE_OUTPUT_PP , .Pull=GPIO_NOPULL , .Speed= GPIO_SPEED_FREQ_LOW}}
/* #define WIFI_32K_CLK   {GPIOA,{.Pin= GPIO_PIN_8, .Mode=GPIO_MODE_AF_PP , .Pull=GPIO_NOPULL , .Speed= GPIO_SPEED_FREQ_LOW , .Alternate=GPIO_AF0_MCO}} */

#define WIFI_SDIO_CMD   {GPIOD,{.Pin= GPIO_PIN_2 , .Mode=GPIO_MODE_AF_PP , .Pull=GPIO_PULLUP , .Speed= GPIO_SPEED_FREQ_VERY_HIGH, .Alternate=GPIO_AF12_SDMMC1}}
#define WIFI_SDIO_CLK   {GPIOC,{.Pin= GPIO_PIN_12, .Mode=GPIO_MODE_AF_PP , .Pull=GPIO_PULLUP , .Speed= GPIO_SPEED_FREQ_VERY_HIGH, .Alternate=GPIO_AF12_SDMMC1}}
#define WIFI_SDIO_D0    {GPIOC,{.Pin= GPIO_PIN_8 , .Mode=GPIO_MODE_AF_PP , .Pull=GPIO_PULLUP , .Speed= GPIO_SPEED_FREQ_VERY_HIGH, .Alternate=GPIO_AF12_SDMMC1}}
#define WIFI_SDIO_D1    {GPIOC,{.Pin= GPIO_PIN_9 , .Mode=GPIO_MODE_AF_PP , .Pull=GPIO_PULLUP , .Speed= GPIO_SPEED_FREQ_VERY_HIGH, .Alternate=GPIO_AF12_SDMMC1}}
#define WIFI_SDIO_D2    {GPIOC,{.Pin= GPIO_PIN_10, .Mode=GPIO_MODE_AF_PP , .Pull=GPIO_PULLUP , .Speed= GPIO_SPEED_FREQ_VERY_HIGH, .Alternate=GPIO_AF12_SDMMC1}}
#define WIFI_SDIO_D3    {GPIOC,{.Pin= GPIO_PIN_11, .Mode=GPIO_MODE_AF_PP , .Pull=GPIO_PULLUP , .Speed= GPIO_SPEED_FREQ_VERY_HIGH, .Alternate=GPIO_AF12_SDMMC1}}
#define WIFI_SDIO_OOB_IRQ {GPIOD,{.Pin= GPIO_PIN_9,  .Mode=GPIO_MODE_IT_RISING , .Pull=GPIO_NOPULL , .Speed= GPIO_SPEED_FREQ_VERY_HIGH, .Alternate=GPIO_AF12_SDMMC1}}

#endif /* STM32H747xx */

#ifdef STM32H735xx
#include "stm32h7xx.h"

/* power pin */
/* STMOD + PF13=PIN 20 when display is UP top right corner of connector */
#define WIFI_WL_REG_ON    {GPIOF,{.Pin= GPIO_PIN_13, .Mode=GPIO_MODE_OUTPUT_PP , .Pull=GPIO_NOPULL , .Speed= GPIO_SPEED_FREQ_LOW}}
/*#define WIFI_32K_CLK      {GPIOA,{.Pin= GPIO_PIN_8, .Mode=GPIO_MODE_AF_PP , .Pull=GPIO_NOPULL , .Speed= GPIO_SPEED_FREQ_LOW , .Alternate=GPIO_AF0_MCO}} */

#define WIFI_SDIO_CMD   {GPIOD,{.Pin= GPIO_PIN_2 , .Mode=GPIO_MODE_AF_PP , .Pull=GPIO_PULLUP , .Speed= GPIO_SPEED_FREQ_VERY_HIGH, .Alternate=GPIO_AF12_SDMMC1}}
#define WIFI_SDIO_CLK   {GPIOC,{.Pin= GPIO_PIN_12, .Mode=GPIO_MODE_AF_PP , .Pull=GPIO_PULLUP , .Speed= GPIO_SPEED_FREQ_VERY_HIGH, .Alternate=GPIO_AF12_SDMMC1}}
#define WIFI_SDIO_D0    {GPIOC,{.Pin= GPIO_PIN_8 , .Mode=GPIO_MODE_AF_PP , .Pull=GPIO_PULLUP , .Speed= GPIO_SPEED_FREQ_VERY_HIGH, .Alternate=GPIO_AF12_SDMMC1}}
#define WIFI_SDIO_D1    {GPIOC,{.Pin= GPIO_PIN_9 , .Mode=GPIO_MODE_AF_PP , .Pull=GPIO_PULLUP , .Speed= GPIO_SPEED_FREQ_VERY_HIGH, .Alternate=GPIO_AF12_SDMMC1}}
#define WIFI_SDIO_D2    {GPIOC,{.Pin= GPIO_PIN_10, .Mode=GPIO_MODE_AF_PP , .Pull=GPIO_PULLUP , .Speed= GPIO_SPEED_FREQ_VERY_HIGH, .Alternate=GPIO_AF12_SDMMC1}}
#define WIFI_SDIO_D3    {GPIOC,{.Pin= GPIO_PIN_11, .Mode=GPIO_MODE_AF_PP , .Pull=GPIO_PULLUP , .Speed= GPIO_SPEED_FREQ_VERY_HIGH, .Alternate=GPIO_AF12_SDMMC1}}
#define WIFI_SDIO_OOB_IRQ {GPIOD,{.Pin= GPIO_PIN_9,  .Mode=GPIO_MODE_IT_RISING , .Pull=GPIO_NOPULL , .Speed= GPIO_SPEED_FREQ_VERY_HIGH, .Alternate=GPIO_AF12_SDMMC1}}

#endif /* STM32H735xx */

#ifdef STM32F769xx
#include "stm32f7xx.h"

/* power pin */
/* STMOD + PF13=PIN 20 when display is UP top right corner of connector */
#define WIFI_WL_REG_ON    {GPIOB,{.Pin= GPIO_PIN_8, .Mode=GPIO_MODE_OUTPUT_PP , .Pull=GPIO_NOPULL , .Speed= GPIO_SPEED_FREQ_LOW}}
/* #define WIFI_32K_CLK      {GPIOA,{.Pin= GPIO_PIN_8, .Mode=GPIO_MODE_AF_PP , .Pull=GPIO_NOPULL , .Speed= GPIO_SPEED_FREQ_LOW , .Alternate=GPIO_AF0_MCO}} */

#define WIFI_SDIO_CMD   {GPIOD,{.Pin= GPIO_PIN_2 , .Mode=GPIO_MODE_AF_PP , .Pull=GPIO_PULLUP , .Speed= GPIO_SPEED_FREQ_VERY_HIGH, .Alternate=GPIO_AF12_SDMMC1}}
#define WIFI_SDIO_CLK   {GPIOC,{.Pin= GPIO_PIN_12, .Mode=GPIO_MODE_AF_PP , .Pull=GPIO_PULLUP , .Speed= GPIO_SPEED_FREQ_VERY_HIGH, .Alternate=GPIO_AF12_SDMMC1}}
#define WIFI_SDIO_D0    {GPIOC,{.Pin= GPIO_PIN_8 , .Mode=GPIO_MODE_AF_PP , .Pull=GPIO_PULLUP , .Speed= GPIO_SPEED_FREQ_VERY_HIGH, .Alternate=GPIO_AF12_SDMMC1}}
#define WIFI_SDIO_D1    {GPIOC,{.Pin= GPIO_PIN_9 , .Mode=GPIO_MODE_AF_PP , .Pull=GPIO_PULLUP , .Speed= GPIO_SPEED_FREQ_VERY_HIGH, .Alternate=GPIO_AF12_SDMMC1}}
#define WIFI_SDIO_D2    {GPIOC,{.Pin= GPIO_PIN_10, .Mode=GPIO_MODE_AF_PP , .Pull=GPIO_PULLUP , .Speed= GPIO_SPEED_FREQ_VERY_HIGH, .Alternate=GPIO_AF12_SDMMC1}}
#define WIFI_SDIO_D3    {GPIOC,{.Pin= GPIO_PIN_11, .Mode=GPIO_MODE_AF_PP , .Pull=GPIO_PULLUP , .Speed= GPIO_SPEED_FREQ_VERY_HIGH, .Alternate=GPIO_AF12_SDMMC1}}
#define WIFI_SDIO_OOB_IRQ {GPIOD,{.Pin= GPIO_PIN_9,  .Mode=GPIO_MODE_IT_RISING , .Pull=GPIO_NOPULL , .Speed= GPIO_SPEED_FREQ_VERY_HIGH, .Alternate=GPIO_AF12_SDMMC1}}

#endif /* STM32F769xx */

#endif /* _WHD_CONFIG_TEMPLATE_H_ */
