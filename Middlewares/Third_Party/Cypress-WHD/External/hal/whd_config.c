#include "whd.h"
#include "whd_debug.h"
#include "whd_resource_api.h"
#include "whd_wifi_api.h"
#include "whd_network_types.h"
#include "whd_types_int.h"
#include "cy_network_buffer.h"
#include "cyhal_gpio.h"
#include "cyhal_sdio.h"
#include "cyabs_rtos.h"
#include "whd_config.h"
#include <stdlib.h>

static cy_rslt_t sdio_enumerate(cyhal_sdio_t *sdhc_obj);
static cy_rslt_t whd_init_hardware_sdio(cyhal_sdio_t *sdhc_obj);



extern whd_resource_source_t resource_ops;
extern whd_buffer_funcs_t buffer_funcs_ops;
extern whd_netif_funcs_t netif_funcs_ops;
extern whd_sdio_config_t sdio_config;
static whd_init_config_t whd_init_config;

#ifdef CY_WHD_CONFIG_USE_SDIO

typedef enum
{
  CYBSP_WIFI_WL_REG_ON,
  CYBSP_WIFI_32K_CLK,
  CYBSP_WIFI_SDIO_CMD,
  CYBSP_WIFI_SDIO_CLK,
  CYBSP_WIFI_SDIO_D0,
  CYBSP_WIFI_SDIO_D1,
  CYBSP_WIFI_SDIO_D2,
  CYBSP_WIFI_SDIO_D3,
  CYBSP_SDIO_OOB_IRQ,
  CYBSP_WIFI_MAX
} wwd_sdio_pin_t;

/* Edit  Pin configuration. */
const pinconfig_t PinConfig[] =
{
  [CYBSP_WIFI_WL_REG_ON] = WIFI_WL_REG_ON,
#ifdef WIFI_32K_CLK
  [CYBSP_WIFI_32K_CLK]  =  WIFI_32K_CLK,
#endif /* WIFI_32K_CLK */
  [CYBSP_WIFI_SDIO_CMD] =  WIFI_SDIO_CMD,
  [CYBSP_WIFI_SDIO_CLK] =  WIFI_SDIO_CLK,
  [CYBSP_WIFI_SDIO_D0 ] =  WIFI_SDIO_D0,
  [CYBSP_WIFI_SDIO_D1 ] =  WIFI_SDIO_D1,
  [CYBSP_WIFI_SDIO_D2 ] =  WIFI_SDIO_D2,
  [CYBSP_WIFI_SDIO_D3 ] =  WIFI_SDIO_D3,
#ifdef WIFI_SDIO_OOB_IRQ
  [CYBSP_SDIO_OOB_IRQ ] =  WIFI_SDIO_OOB_IRQ
#endif /* WIFI_SDIO_OOB_IRQ */
};


/* Edit SDIO configuration */
whd_sdio_config_t sdio_config =
{
  /* Bus config */
  .sdio_1bit_mode = WHD_FALSE,            /**< Default is false, means SDIO operates under 4 bit mode */
  .high_speed_sdio_clock = WHD_FALSE,     /**< Default is false, means SDIO operates in normal clock rate */
  .oob_config = {                         /**< Out-of-band interrupt configuration (required when bus can sleep) */
    .host_oob_pin = CYHAL_NC_PIN_VALUE,   /**< Host-side GPIO pin selection CYHAL_NC_PIN_VALUE or CYBSP_SDIO_OOB_IRQ */
    .dev_gpio_sel = 0,                    /**< WiFi device-side GPIO pin selection (must be zero) */
    .is_falling_edge = WHD_TRUE,          /**< Interrupt trigger (polarity) */
    .intr_priority = 0,                   /**< OOB interrupt priority */
  }
};

void sdio_enable_high_speed(void);


static cy_rslt_t sdio_enumerate(cyhal_sdio_t *sdhc_obj)
{
  cy_rslt_t result;
  uint32_t loop_count = 0;
  uint32_t data = 0;

  do
  {
    /* Send CMD0 to set it to idle state in SDIO_BYTE_MODE, SDIO_1B_BLOCK */
    cyhal_sdio_send_cmd(sdhc_obj, CYHAL_WRITE, CYHAL_SDIO_CMD_GO_IDLE_STATE, 0, NULL);

    /* CMD5. */
    cyhal_sdio_send_cmd(sdhc_obj, CYHAL_READ, CYHAL_SDIO_CMD_IO_SEND_OP_COND, 0, NULL);

    /* Send CMD3 to get RCA. */
    result = cyhal_sdio_send_cmd(sdhc_obj, CYHAL_READ, CYHAL_SDIO_CMD_SEND_RELATIVE_ADDR, 0, &data);
    loop_count++;
    if (loop_count >= (uint32_t) SDIO_ENUMERATION_TIMEOUT_MS)
    {
      return -1;
    }

  } while ((result != CY_RSLT_SUCCESS) && (cy_rtos_delay_milliseconds((uint32_t) 1), (1 == 1)));
  /* If you're stuck here, check the platform matches your hardware */

  /* Send CMD7 with the returned RCA to select the card */
  cyhal_sdio_send_cmd(sdhc_obj, CYHAL_WRITE, CYHAL_SDIO_CMD_SELECT_CARD, data, &data);
  return result;
}


static cy_rslt_t whd_init_hardware_sdio(cyhal_sdio_t *sdhc_obj)
{
  cy_rslt_t result;
#ifndef CYPRESS_USE_CUBEMX_oups
  result = cyhal_gpio_init(CYBSP_WIFI_WL_REG_ON, CYHAL_GPIO_DIR_OUTPUT, CYHAL_GPIO_DRIVE_PULLUP, 0);
#ifdef WIFI_32K_CLK
  result = cyhal_gpio_init(CYBSP_WIFI_32K_CLK, CYHAL_GPIO_DIR_INPUT, CYHAL_GPIO_DRIVE_PULLUP, 0);
#endif /* WIFI_32K_CLK */
#endif /* CYPRESS_USE_CUBEMX_oups */


#ifdef WIFI_32K_CLK
  cyhal_gpio_write(CYBSP_WIFI_32K_CLK, false);
#endif /* WIFI_32K_CLK */

  cy_rtos_delay_milliseconds(WLAN_POWER_UP_DELAY_MS);

  /* Init SDIO Host */
  cyhal_sdio_init(sdhc_obj, CYBSP_WIFI_SDIO_CMD, CYBSP_WIFI_SDIO_CLK,
                  CYBSP_WIFI_SDIO_D0, CYBSP_WIFI_SDIO_D1,
                  CYBSP_WIFI_SDIO_D2, CYBSP_WIFI_SDIO_D3);

  /* WiFi put power */
  cyhal_gpio_write(CYBSP_WIFI_WL_REG_ON, true);

  cy_rtos_delay_milliseconds(WLAN_POWER_UP_DELAY_MS);

  result = sdio_enumerate(sdhc_obj);
  if (result == CY_RSLT_SUCCESS)
  {
    sdio_enable_high_speed();
  }

  return result;
}


cy_rslt_t whd_boot(whd_driver_t *pwhd_driver)
{
  cy_rslt_t res = 0;
  cyhal_sdio_t sdhc_obj;

  /* Allocated stack  for thread */
  whd_init_config.thread_stack_size = (uint32_t) WHD_THREAD_STACK_SIZE;
  whd_init_config.thread_stack_start = (uint8_t *)WHD_CALLOC(WHD_THREAD_STACK_SIZE, 1);
  whd_init_config.thread_priority = (uint32_t) WHD_THREAD_PRIORITY;

  /* Country for WIFI  */
  whd_init_config.country = WHD_COUNTRY;

  /* Each WiFi chip, will have it's own instance of whd_driver. */
  /* Each whd_driver may use multiple instance of whd_interface_t structs to define behavior and functionality. */
  /* Most of the WHD function calls take this structure as input. */
  /* The default primary interface is created automatically at the time of power up of WiFi chip, whd_wifi_on(..). */
  /* Primary interface is STA/AP role neutral. */
  /* Call whd_init per WiFi chip (in other words per bus slot, two SDIO WiFi chip requires two calls.) */
  res = whd_init(pwhd_driver, &whd_init_config, &resource_ops, &buffer_funcs_ops, &netif_funcs_ops);
  if (res != WHD_SUCCESS)
  {
    WPRINT_WHD_ERROR(("Failed when performing whd driver initialization\n"));
    return res;
  }

  /* Attach a bus SDIO or SPI */
  whd_init_hardware_sdio(&sdhc_obj);
  res = whd_bus_sdio_attach(*pwhd_driver, &sdio_config, &sdhc_obj);
  if (res != WHD_SUCCESS)
  {
    WPRINT_WHD_ERROR(("Failed when performing whd driver initialization\n"));
    return res;
  }
  return res;
}

cy_rslt_t whd_powerdown(whd_driver_t *pwhd_driver)
{
  cy_rslt_t res = 0;

  if (whd_init_config.thread_stack_start)
  {
    WHD_FREE(whd_init_config.thread_stack_start);
  }
  whd_init_config.thread_stack_start = NULL;
  cyhal_sdio_free(NULL);
  return res;
}


#endif /* CY_CONFIG_USE_SDIO */

#if  CY_CONFIG_USE_SPI
TBD
#endif /* CY_CONFIG_USE_SPI  */
