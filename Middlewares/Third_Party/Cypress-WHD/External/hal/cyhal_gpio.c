
#include "cyhal_gpio.h"

#include <stdio.h>


extern pinconfig_t PinConfig[];

void cyhal_gpio_register_irq(cyhal_gpio_t pin, uint8_t intrPriority, cyhal_gpio_irq_handler_t handler,
                             void *handler_arg)
{
  (void)pin;
  (void)intrPriority;
  (void)handler;
  (void)handler_arg;
}


void cyhal_gpio_irq_enable(cyhal_gpio_t pin, cyhal_gpio_irq_event_t event, bool enable)
{
  (void)pin;
  (void)event;
  (void)enable;
}




cy_rslt_t cyhal_gpio_init(cyhal_gpio_t pin, cyhal_gpio_direction_t direction, cyhal_gpio_drive_mode_t drvMode,
                          bool initVal)
{
  cy_rslt_t ret = CY_RSLT_SUCCESS;

  (void)drvMode;

#ifdef STM32H747xx
  if (PinConfig[pin].port == GPIOA)
  {
    __HAL_RCC_GPIOA_CLK_ENABLE();
  }
  else if (PinConfig[pin].port == GPIOB)
  {
    __HAL_RCC_GPIOB_CLK_ENABLE();
  }
  else if (PinConfig[pin].port == GPIOC)
  {
    __HAL_RCC_GPIOC_CLK_ENABLE();
  }
  else if (PinConfig[pin].port == GPIOD)
  {
    __HAL_RCC_GPIOD_CLK_ENABLE();
  }
#endif /* STM32H747xx */

  /* printf("Port Init %s %d\n",PinConfig[pin].portname, PinConfig[pin].pinnumber); */

  /* Ignore the parameter and take the pin configuration directly from a static array definitions */
  HAL_GPIO_Init(PinConfig[pin].port, &PinConfig[pin].config);
  if (direction == CYHAL_GPIO_DIR_OUTPUT)
  {
    HAL_GPIO_WritePin(PinConfig[pin].port, PinConfig[pin].config.Pin, (initVal) ? GPIO_PIN_SET : GPIO_PIN_RESET);
  }
  /* if (direction == CYHAL_GPIO_DIR_OUTPUT)                                                                */
  /*   printf("Port %s %s %d\n", (initVal)?"High":"low", PinConfig[pin].portname, PinConfig[pin].pinnumber);*/

  return ret;
}



void cyhal_gpio_write(cyhal_gpio_t pin, bool value)
{
  /* printf("Port %s %s %d\n",(value)?"High":"low", PinConfig[pin].portname, PinConfig[pin].pinnumber);*/
  HAL_GPIO_WritePin(PinConfig[pin].port, PinConfig[pin].config.Pin, (value) ? GPIO_PIN_SET : GPIO_PIN_RESET);
}
