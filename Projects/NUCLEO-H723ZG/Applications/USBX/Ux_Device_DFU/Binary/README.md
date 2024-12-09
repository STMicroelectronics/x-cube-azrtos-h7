
## <b>Example Description</b>

This directory contains a binary template (in DFU format) to be loaded into Flash memory using Device
Firmware Upgrade application.

This file was converted to the DFU format using the "DFU File Manager Tool" included in the "DfuSe" PC software install.
For more details on how to convert a .bin file to DFU format please refer to the UM0412 user manual
"Getting started with DfuSe USB device firmware upgrade STMicroelectronics extension" available from the
STMicroelectronics microcontroller website www.st.com.

This binary is a simple LED toggling.
The system Timer (Systick) is used to generate the delay.
The offset address of this binary is 0x0802000 which matches the definition in DFU application
"USBD_DFU_APP_DEFAULT_ADDR".

## <b>Hardware and Software environment</b>

  - This example runs on STM32H723xx devices.

  - This example has been tested with STM32H723ZG-NUCLEOboard and can be
    easily tailored to any other supported device and development board.
