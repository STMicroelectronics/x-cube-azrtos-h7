#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include "whd_thread.h"
#include "bus_protocols/whd_bus_sdio_protocol.h"
#include "cyabs_rtos.h"
#include "cyhal_gpio.h"
#include "cyhal_sdio.h"
#include "whd_config.h"


#define COMMAND_FINISHED_CMD52_TIMEOUT_LOOPS (1000)
#define COMMAND_FINISHED_CMD53_TIMEOUT_LOOPS (1000)
#define SDMMC_TX_RX_COMPLETE_TIMEOUT_LOOPS   (1000)

#define BUS_LEVEL_MAX_RETRIES  10000
#define SDMMC_CMD_5            5
#define SDMMC_CMD_53           53
#define SDMMC_STA_STBITERR    ((uint32_t)0x00000200) /*!<Start bit not detected on all data signals in wide bus mode */
#define SDMMC_ERROR_MASK      (SDMMC_STA_CCRCFAIL\
                               | SDMMC_STA_DCRCFAIL | SDMMC_STA_CTIMEOUT | SDMMC_STA_DTIMEOUT \
                               | SDMMC_STA_TXUNDERR | SDMMC_STA_RXOVERR)
#define SDMMC_MASK_SDIOTIE

/* size must be multiple of 32 bytes to avoid cache mngt issues */
#define TEMP_BUFFER_SIZE        1536
#define MAX(A, B)               ((A) > (B)) ? (A) : (B)

void SDMMC1_IRQHandler(void);
cy_rslt_t sdio_enable_high_speed(void);


extern pinconfig_t PinConfig[];

static void *whd_handler_context;
static cyhal_sdio_irq_handler_t whd_func_handler;

static uint8_t          *dma_data_source;
static uint32_t          dma_transfer_size;
static cy_semaphore_t    sdio_transfer_finished_semaphore;
__IO static uint32_t     sdio_transfer_failed = 0;
__IO static uint32_t     irqstatus = 0;


#if defined ( __ICCARM__ ) /*!< IAR Compiler */
#pragma data_alignment = 32
static uint32_t temp_dma_buffer[TEMP_BUFFER_SIZE];

#elif (defined ( __CC_ARM ) || defined(__ARMCC_VERSION)) /* MDK ARM Compiler */
static uint32_t temp_dma_buffer[TEMP_BUFFER_SIZE] __attribute__((aligned(32)));

#elif defined ( __GNUC__ ) /* GNU Compiler */
static uint32_t temp_dma_buffer[TEMP_BUFFER_SIZE] __attribute__((aligned(32)));
#endif /* defined ( __ICCARM__ ) */


static uint8_t  *user_data;
static uint32_t user_data_size;
static bool     use_copy_buffer;

static SD_HandleTypeDef *phsd;

void cypress_wifi_select_sd(SD_HandleTypeDef *pSelectedSDHandle, DMA_HandleTypeDef *notused)
{
  (void)notused;

  phsd = pSelectedSDHandle;
}


cy_rslt_t cyhal_sdio_init(cyhal_sdio_t *obj, cyhal_gpio_t cmd, cyhal_gpio_t clk,
                          cyhal_gpio_t data0, cyhal_gpio_t data1, cyhal_gpio_t data2, cyhal_gpio_t data3)
{
  cy_rslt_t ret = CY_RSLT_SUCCESS;

#ifdef STM32H747xx
 /*__HAL_RCC_GPIOA_CLK_ENABLE();*/
 __HAL_RCC_GPIOB_CLK_ENABLE();
 __HAL_RCC_GPIOC_CLK_ENABLE();
 __HAL_RCC_GPIOD_CLK_ENABLE();
#endif /* STM32H747xx */

  HAL_GPIO_Init(PinConfig[cmd].port, &PinConfig[cmd].config);
  HAL_GPIO_Init(PinConfig[clk].port, &PinConfig[clk].config);
  HAL_GPIO_Init(PinConfig[data0].port, &PinConfig[data0].config);
  HAL_GPIO_Init(PinConfig[data1].port, &PinConfig[data1].config);
  HAL_GPIO_Init(PinConfig[data2].port, &PinConfig[data2].config);
  HAL_GPIO_Init(PinConfig[data3].port, &PinConfig[data3].config);

  /* Reset SDIO Block */
  SDMMC_PowerState_OFF(SDMMC1);
  __HAL_RCC_SDMMC1_FORCE_RESET();
  __HAL_RCC_SDMMC1_RELEASE_RESET();

  /* Enable the SDIO Clock */
  __HAL_RCC_SDMMC1_CLK_ENABLE();


  SDMMC_InitTypeDef sdio_init_structure = {0};

  sdio_init_structure.ClockDiv            = SDMMC_INIT_CLK_DIV;
  sdio_init_structure.ClockEdge           = SDMMC_CLOCK_EDGE_RISING;
  sdio_init_structure.ClockPowerSave      = SDMMC_CLOCK_POWER_SAVE_DISABLE;
  sdio_init_structure.BusWide             = SDMMC_BUS_WIDE_1B;
  sdio_init_structure.HardwareFlowControl = SDMMC_HARDWARE_FLOW_CONTROL_DISABLE;
#ifndef STM32H735xx
#if (USE_SD_TRANSCEIVER != 0U)
  sdio_init_structure.TranceiverPresent   = SDMMC_TRANSCEIVER_NOT_PRESENT;
#endif /* USE_SD_TRANSCEIVER */
#endif /* STM32H735xx */

  ret  = SDMMC_Init(SDMMC1, sdio_init_structure);
  ret |= SDMMC_PowerState_ON(SDMMC1);
  ret |= SDMMC_SetSDMMCReadWaitMode(SDMMC1, SDMMC_READ_WAIT_MODE_CLK);
  if (ret)
  {
    return ret;
  }

  /* Clear all SDIO interrupts */
  SDMMC1->ICR = (uint32_t) 0xffffffff;

  /* Turn on SDIO IRQ */
  /* Must be lower priority than the value of SYSCALL_INTERRUPT_PRIORITY */
  /* otherwise RTOS will not be able to mask the interrupt */
  /* keep in mind that ARMCM7 interrupt priority logic is inverted, the highest value */
  /* is the lowest priority */
  HAL_NVIC_EnableIRQ((IRQn_Type) SDMMC1_IRQn);
  HAL_NVIC_SetPriority(SDMMC1_IRQn, 5, 0);

  if (cy_rtos_init_semaphore(&sdio_transfer_finished_semaphore, 1, 0) != WHD_SUCCESS)
  {
    cy_rtos_deinit_semaphore(&sdio_transfer_finished_semaphore);
    return -1;
  }
  return ret;
}

void cyhal_sdio_free(cyhal_sdio_t *obj)
{
  cy_rtos_deinit_semaphore(&sdio_transfer_finished_semaphore);
}

cy_rslt_t sdio_enable_high_speed(void)
{
  SDMMC_InitTypeDef sdio_init_structure = {0};
#ifdef SLOW_SDMMC_CLOCK
  sdio_init_structure.ClockDiv       = (uint8_t) 10; /* 10 = 10 MHz if SDIO clock = 200MHz */
#else
  /* SDMMC High Speed Frequency (50Mhz max) for Peripheral CLK 200MHz. */
  sdio_init_structure.ClockDiv       = SDMMC_HSpeed_CLK_DIV;
#endif /* SLOW_SDMMC_CLOCK */
  sdio_init_structure.ClockEdge      = SDMMC_CLOCK_EDGE_RISING;
  sdio_init_structure.ClockPowerSave = SDMMC_CLOCK_POWER_SAVE_DISABLE;
#ifndef SDMMC_1_BIT
  sdio_init_structure.BusWide        = SDMMC_BUS_WIDE_4B;
#else
  sdio_init_structure.BusWide        = SDMMC_BUS_WIDE_1B;
#endif /* SDMMC_1_BIT */
  sdio_init_structure.HardwareFlowControl = SDMMC_HARDWARE_FLOW_CONTROL_DISABLE;

  SDMMC_Init(SDMMC1, sdio_init_structure);
  return CY_RSLT_SUCCESS;
}

void cyhal_sdio_register_irq(cyhal_sdio_t *obj, cyhal_sdio_irq_handler_t handler, void *handler_arg)
{
  whd_handler_context = handler_arg;
  whd_func_handler = handler;
}

void cyhal_sdio_irq_enable(cyhal_sdio_t *obj, cyhal_sdio_irq_event_t event, bool enable)
{
}


cy_rslt_t cyhal_sdio_send_cmd(const cyhal_sdio_t *obj, cyhal_transfer_t direction, cyhal_sdio_command_t command,
                              uint32_t argument, uint32_t *response)

{
  uint32_t loop_count = 0;
  cy_rslt_t result;
  uint16_t attempts = 0;
  uint32_t temp_sta;

  if (response != NULL)
  {
    *response = 0;
  }
  /* platform_mcu_powersave_disable(); */

  /* Ensure the bus isn't stuck half way through transfer */
  DMA2_Stream3->CR   = 0;

restart:
  SDMMC1->ICR = (uint32_t) 0xFFFFFFFF;
  ++attempts;

  /* Check if we've tried too many times */
  if (attempts >= (uint16_t) BUS_LEVEL_MAX_RETRIES)
  {
    /* WWD_SDMMC_RETRIES_EXCEEDED */
    result = -1;
    goto exit;
  }

  /* Send the command */
  SDMMC1->ARG = argument;
  SDMMC1->CMD = (uint32_t)(command | SDMMC_RESPONSE_SHORT | SDMMC_WAIT_NO | SDMMC_CPSM_ENABLE);
  loop_count = (uint32_t) COMMAND_FINISHED_CMD52_TIMEOUT_LOOPS;
  do
  {
    temp_sta = SDMMC1->STA;
    loop_count--;
    if (loop_count == 0 || ((response != NULL) && ((temp_sta & SDMMC_ERROR_MASK) != 0)))
    {
      goto restart;
    }
  } while ((temp_sta & SDMMC_FLAG_CMDACT) != 0);

  if (response != NULL)
  {
    *response = SDMMC1->RESP1;
  }
  result = CY_RSLT_SUCCESS;

exit:

  if (result)
  {
    while (1);
  }
  SDMMC1->CMD = 0;
  return result;
}


static  uint32_t find_optimal_block_size(uint32_t data_size)
{
  if (data_size > (uint32_t) 256)
  {
    return 512;
  }
  if (data_size > (uint32_t) 128)
  {
    return 256;
  }
  if (data_size > (uint32_t) 64)
  {
    return 128;
  }
  if (data_size > (uint32_t) 32)
  {
    return 64;
  }
  if (data_size > (uint32_t) 16)
  {
    return 32;
  }
  if (data_size > (uint32_t) 8)
  {
    return 16;
  }
  if (data_size > (uint32_t) 4)
  {
    return 8;
  }
  if (data_size > (uint32_t) 2)
  {
    return 4;
  }

  return 4;
}

static uint32_t sdio_get_blocksize(uint32_t blocksize)
{
  uint32_t result = 0;
  blocksize >>= 1;
  while (blocksize)
  {
    result++;
    blocksize >>= 1;
  }
  result <<= 4;
  return result;
}

static void sdio_prepare_data_transfer_local(cyhal_transfer_t direction, uint32_t block_size, uint8_t *data,
                                             uint16_t data_size)
{
  /* Setup a single transfer using the temp buffer */
  user_data         = data;
  user_data_size    = data_size;

  dma_transfer_size = (uint32_t)(((data_size + (uint16_t) block_size - 1) \
                                  / (uint16_t) block_size) * (uint16_t) block_size);
  /* we should use copy buffer if data is not aligned on data cache line or */
  /* if size is not multiple of cache line size */
  /* otherwise access to variable located in front or */
  /* after data variable may unexpectedly reload the data cache */
  /* prior to DMA end (RTOS thread), so incorrect value would be read */

  if ((direction == CYHAL_READ) && ((dma_transfer_size != data_size) || ((uint32_t) data & 31)
                                    || (dma_transfer_size & 31)))
  {
    /* printf("can t use block DMA %d %d\n",dma_transfer_size,data_size); */
    use_copy_buffer = true;
    if (dma_transfer_size > TEMP_BUFFER_SIZE)
    {
      PRINTF("please increase TEMP_BUFFER_SIZE to %"PRIu32" size\n", dma_transfer_size);
      while (1);
    }
  }
  else
  {
    use_copy_buffer = false;
  }
  use_copy_buffer = true;
  if (direction == CYHAL_WRITE)
  {
    SCB_CleanDCache_by_Addr((uint32_t *)data, data_size + 32);
    dma_data_source = data;
    if ((uint32_t) data & 3)
    {
      PRINTF("Unaligned 4 byte buffers , need to add copy code to handle that\n");
      while (3);
    }
  }
  else
  {
    if (use_copy_buffer)
    {
      dma_data_source = (uint8_t *)temp_dma_buffer;
    }
    else
    {
      dma_data_source = (uint8_t *)data;
    }
    SCB_InvalidateDCache_by_Addr((uint32_t *) dma_data_source, dma_transfer_size);
  }

  SDMMC1->DTIMER = (uint32_t) 0xFFFFFFFF;
  SDMMC1->DLEN = dma_transfer_size;
  SDMMC1->DCTRL = sdio_get_blocksize(block_size) | ((direction == CYHAL_READ) ? \
                                                    SDMMC_TRANSFER_DIR_TO_SDMMC : \
                                                    SDMMC_TRANSFER_DIR_TO_CARD) | \
                  SDMMC_TRANSFER_MODE_BLOCK | \
                  SDMMC_DPSM_DISABLE | \
                  SDMMC_DCTRL_SDIOEN;

  SDMMC1->IDMACTRL  = SDMMC_ENABLE_IDMA_SINGLE_BUFF;
  if ((uint32_t) dma_data_source & 0x3)
  {
    while (1);
  }
  SDMMC1->IDMABASE0 = (uint32_t) dma_data_source;
}


static void sdio_enable_bus_irq(void)
{
  SDMMC1->MASK |= SDMMC_IT_DCRCFAIL | SDMMC_IT_DTIMEOUT | \
                  SDMMC_IT_RXOVERR | SDMMC_IT_TXUNDERR | SDMMC_IT_DATAEND | SDMMC_ICR_SDIOITC;
}


cy_rslt_t cyhal_sdio_bulk_transfer(cyhal_sdio_t *obj, cyhal_transfer_t direction, uint32_t argument,
                                   const uint32_t *data, uint16_t length, uint32_t *response)
{
  cy_rslt_t result;

  uint32_t loop_count = 0;
  uint16_t attempts = 0;
  uint32_t block_size = 64;
  sdio_cmd_argument_t arg;
  uint32_t cmd;

  arg.value = argument;
  sdio_enable_bus_irq();
  if (response != NULL)
  {
    *response = 0;
  }


restart:

  sdio_transfer_failed = 0;

  SDMMC1->ICR = (uint32_t) 0xFFFFFFFF;
  ++attempts;

  /* Check if we've tried too many times */
  if (attempts >= (uint16_t) BUS_LEVEL_MAX_RETRIES)
  {
    /* WWD_SDMMC_RETRIES_EXCEEDED */
    PRINTF("Too much attempt\n");
    result = -1;
    goto exit;
  }

  /* Dodgy STM32 hack to set the CMD53 byte mode size to be the same as the block size */
  if (arg.cmd53.block_mode == 0)
  {
    block_size = find_optimal_block_size(arg.cmd53.count);
    if (block_size < SDIO_512B_BLOCK)
    {
      arg.cmd53.count = block_size;
    }
    else
    {
      arg.cmd53.count = 0;
    }
    argument = arg.value;
  }


  /* Prepare the SDIO for a data transfer */
  sdio_prepare_data_transfer_local(direction, block_size, (uint8_t *) data, (uint32_t) length);

  /* Send the command */
  SDMMC1->ARG = argument;
  cmd = (uint32_t)(SDMMC_CMD_53 | SDMMC_RESPONSE_SHORT | SDMMC_WAIT_NO | SDMMC_CPSM_ENABLE | SDMMC_CMD_CMDTRANS);

  SDMMC1->CMD = cmd;

  /* Wait for the whole transfer to complete */
  result = cy_rtos_get_semaphore(&sdio_transfer_finished_semaphore, 4 * 500, WHD_FALSE);
  if (result != CY_RSLT_SUCCESS)
  {
    goto exit;
  }
  if (sdio_transfer_failed)
  {
    PRINTF("try again sdio_transfer_failed %"PRIx32"\n", sdio_transfer_failed);
    PRINTF("try again irq %"PRIx32"\n", irqstatus);
    goto restart;
  }
  /* Check if there were any SDIO errors */
  if ((SDMMC1->STA & (SDMMC_STA_DTIMEOUT | SDMMC_STA_CTIMEOUT)) != 0)
  {
    PRINTF("sdio errors SDMMC_STA_DTIMEOUT | SDMMC_STA_CTIMEOUT\n");
    goto restart;
  }
  else if (((SDMMC1->STA & (SDMMC_STA_CCRCFAIL | SDMMC_STA_DCRCFAIL | SDMMC_STA_TXUNDERR | SDMMC_STA_RXOVERR)) != 0))
  {
    PRINTF("sdio errors SDMMC_STA_CCRCFAIL | SDMMC_STA_DCRCFAIL | SDMMC_STA_TXUNDERR | SDMMC_STA_RXOVER \n");
    goto restart;
  }

  /* Wait till complete */
  loop_count = (uint32_t) SDMMC_TX_RX_COMPLETE_TIMEOUT_LOOPS * 1000;
  do
  {
    loop_count--;
    if (loop_count == 0 || ((SDMMC1->STA & SDMMC_ERROR_MASK) != 0))
    {
      PRINTF("sdio errors SDMMC1->STA & SDMMC_ERROR_MASK or TO, loopct=%"PRIu32" \n", loop_count);
      goto restart;
    }
  } while ((SDMMC1->STA & (SDMMC_STA_CPSMACT | SDMMC_STA_DPSMACT)) != 0);

  if ((direction == CYHAL_READ))
  {
    SCB_CleanInvalidateDCache_by_Addr((uint32_t *)((uint32_t) dma_data_source & (~31)), dma_transfer_size + 32);
    if (use_copy_buffer)
    {
      memcpy(user_data, dma_data_source, (size_t) user_data_size);
    }
  }

  if (response != NULL)
  {
    *response = SDMMC1->RESP1;
  }
  result = CY_RSLT_SUCCESS;

exit:

  SDMMC1->CMD = 0;
  if (result != 0)
  {
    PRINTF("Timeout Irqstatus %"PRIx32"  cmdis %"PRIx32" arg %"PRIx32" \n", irqstatus, cmd, argument);
  }
  while (result != 0) ;
  return result;
}

void SDMMC1_IRQHandler(void)
{
  uint32_t intstatus = SDMMC1->STA;

  irqstatus = intstatus;

  if ((intstatus & (SDMMC_STA_CCRCFAIL | SDMMC_STA_DCRCFAIL | SDMMC_STA_TXUNDERR | SDMMC_STA_RXOVERR |
                    SDMMC_STA_STBITERR)) != 0)
  {
    sdio_transfer_failed = intstatus;
    SDMMC1->ICR = (uint32_t) 0xffffffff;
    cy_rtos_set_semaphore(&sdio_transfer_finished_semaphore, WHD_TRUE);
  }
  else
  {
    if ((intstatus & (SDMMC_STA_CMDREND | SDMMC_STA_CMDSENT)) != 0)
    {
      if ((SDMMC1->RESP1 & 0x800) != 0)
      {
        sdio_transfer_failed = irqstatus;
        cy_rtos_set_semaphore(&sdio_transfer_finished_semaphore, WHD_TRUE);
      }

      /* Clear all command/response interrupts */
      SDMMC1->ICR = (SDMMC_STA_CMDREND | SDMMC_STA_CMDSENT);
    }

    if (intstatus & SDMMC_STA_DATAEND)
    {
      SDMMC1->ICR      = SDMMC_STA_DATAEND;
      SDMMC1->DLEN     = 0;
      SDMMC1->DCTRL    = SDMMC_DCTRL_SDIOEN;
      SDMMC1->IDMACTRL = SDMMC_DISABLE_IDMA;
      SDMMC1->CMD      = 0;
      cy_rtos_set_semaphore(&sdio_transfer_finished_semaphore, WHD_TRUE);
    }

    /* Check whether the external interrupt was triggered */
    if (intstatus & SDMMC_STA_SDIOIT)
    {
      /* Clear the interrupt */
      SDMMC1->ICR   = SDMMC_STA_SDIOIT;
      /* Mask interrupt, to be unmasked later for each bulk*/
      SDMMC1->MASK &= ~(SDMMC_ICR_SDIOITC);
      /* Inform WICED WWD thread whd_thread_notify_irq */
      whd_func_handler(whd_handler_context, CYHAL_SDIO_CARD_INTERRUPT);
    }
  }
}
