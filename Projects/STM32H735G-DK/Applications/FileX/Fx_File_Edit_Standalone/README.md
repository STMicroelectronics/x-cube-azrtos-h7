## <b>Fx_File_Edit_Standalone Application Description</b>

This application provides an example of FileX stack usage on STM32H735G-DK board, running in standalone mode (without ThreadX). It demonstrates how to create a Fat File system on the SD card memory using FileX API.

The application is designed to execute file operations on the SD card, it provides all required software code for properly managing it.

The application's main calls the MX_FileX_Init() function in order to Initialize FileX, format the SD memory as FAT and open the sdio_disk driver. The resulting file system is a FAT32 compatible, with 512 bytes per sector.

Upon successful opening of the created SDIO-Disk media, FileX continues with creating a file called "STM32.TXT" into the root directory by calling MX_FileX_Process(void) function, then writes into it some predefined data. The file is re-opened in read only mode and content is checked.


As stated earlier, the present application runs in standalone mode without ThreadX, for this reason, the standalone variant of fileX is used, plus the following flags need to be set in fx_user.h:

-  #define FX_SINGLE_THREAD
-  #define FX_STANDALONE_ENABLE

#### <b>Expected success behavior</b>

- A file named STM32.TXT should be visible in the root directory of the SD card.
- A blinking green LED light marks the success of the file operations.

#### <b>Error behaviors</b>

- On failure, the LED_RED should start blinking.
- Error handler is called at the spot where the error occurred.

#### <b>Assumptions if any</b>
None.

#### <b>Known limitations</b>

No SD card insertion/removal mechanisms are implemented.

### <b>Notes</b>

 1. Before starting the application, the SD card should be present in the SD card connector and formatted initially by user.

#### <b>FileX/LevelX usage hints</b>

- FileX sd driver is using the DMA, thus the DTCM (0x20000000) memory should not be used by the application, as it is not accessible by the SD DMA.
- When calling the fx_media_format() API, it is highly recommended to understand all the parameters used by the API to correctly generate a valid filesystem.

### <b>Keywords</b>

FileX, File system, SDMMC, FAT32


### <b>Hardware and Software environment</b>

  - This application runs on STM32H735xx devices.
  - This application has been tested with STMicroelectronics STM32H735G-DK boards Revision: MB1520-H735I-B02.
    and can be easily tailored to any other supported device and development board.

### <b>How to use it ?</b>

In order to make the program work, you must do the following :

  - Open your preferred toolchain
  - Rebuild all files and load your image into target memory
  - Run the application