#if defined(STM32H753xx) || defined(STM32H747xx) || defined(STM32H735xx)
#include "cyhal_sdio_h7.c"
#else
#include "cyhal_sdio_legacy.c"
#endif
