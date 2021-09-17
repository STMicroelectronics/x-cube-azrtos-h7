
## <b>Ux_Device_DFU application description</b>

This application provides an example of Azure RTOS USBX stack usage on STM32H723ZG board,
it shows how to develop USB Device Firmware Upgrade "DFU" based application.

The application is designed to emulate an USB DFU device, the code provides all required device descriptors framework
and associated Class descriptor report to build a compliant USB DFU device.

At the beginning ThreadX calls the entry function tx_application_define(), at this stage, all USBx resources
are initialized, the DFU Class driver is registered and the application creates 2 threads with the same priorities :

  - usbx_app_thread_entry    (Prio : 10; PreemptionPrio : 10) used to initialize USB OTG HAL PCD driver and start the device.
  - usbx_dfu_download_thread (Prio : 20; PreemptionPrio : 20) used to execute download command.

The DFU transactions are based on Endpoint 0 (control endpoint) transfer. All requests and status
control are sent/received through this endpoint.

The Internal flash memory is split as follows:
 - DFU area located in [0x08000000 : USBD_DFU_APP_DEFAULT_ADD-1]: Only read access
 - Application area located in [USBD_DFU_APP_DEFAULT_ADD : Device's end address]: Read, Write, and Erase
   access

#### <b>Note 1</b>
In this application, two operating modes are available:

 1. DFU operating mode:
    This mode is entered after an MCU reset in case:
     - The DFU mode is forced by the user: the user presses the TAMP button.
     - No valid code found in the application area: a code is considered valid if the MSB of the initial
       Main Stack Pointer (MSP) value located in the first address of the application area is equal to
       0x20000.

 2. Run-time application mode:
    This is the normal run-time activities. A binary which toggles LED1 and LED2 on the NUCLEO-H723ZG board "NUCLEO-H723ZG_LED_Toggle_@0x08020000" is provided in Binary directory.

#### <b>Note 2</b>
Traditionally, firmware is stored in Hex, S19 or Binary files, but these formats do not contain the
necessary information to perform the upgrade operation, they contain only the actual data of the program
to be downloaded. However, the DFU operation requires more information, such as the product identifier,
vendor identifier, Firmware version and the Alternate setting number (Target ID) of the target to be
used, this information makes the upgrade targeted and more secure. To add this information, DFU file
format is used. For more details refer to the "DfuSe File Format Specification" document (UM0391).

To generate a DFU image, download "DFUse Demonstration" tool and use DFU File Manager to convert a
binary image into a DFU image. This tool is for download from www.st.com
To download a *.dfu image, use "DfuSe Demo" available within "DFUse Demonstration" install directory.

Please refer to UM0412, DFuSe USB device firmware upgrade STMicroelectronics extension for more details
on the driver installation and PC host user interface.

After each device reset (unplug the NUCLEO-H723ZG board from PC), Plug the NUCLEO-H723ZG board with Key User push-button button pressed to enter the DFU mode.

#### <b>Note 3</b>
In case of composite device, we shall :

  1.  Uninstall device composite driver.
  2.  Activate the install of unsigned drives in windows.
  3. Updated STtube.inf driver by replacing this line in all sections from
    %USB\VID_0483&PID_DF11_DeviceDesc%=STTub30.Device, USB\VID_0483 to 
    %USB\VID_0483&PID_DF11_DeviceDesc%=STTub30.Device, USB\VID_0483&PID_DF11&MI_00
  4. Reinstall driver.

#### <b>Expected success behavior</b>

When plugged to PC host, the STM32H723ZG must be properly enumerated as an USB DFU device.
During the enumeration phase, device provides host with the requested descriptors (Device, configuration, string).
Those descriptors are used by host driver to identify the device capabilities.
Once the STM32H723ZG USB device successfully completed the enumeration phase, Open DfuSe you can now download and upload to/from media .


#### <b>Error behaviors</b>

Host PC shows that USB device does not appear as DFU device.

#### <b>Assumptions if any</b>

User is familiar with USB 2.0 "Universal Serial BUS" Specification and DFU class Specification.

#### <b>Known limitations</b>

Only Dfuse is supported. CubeProgrammer is not yet supported.

When creating an USBX based application with MDK-ARM AC6 compiler make sure to disable the optimization for stm32h7xx_ll_usb.c file, otherwise application might not work correctly.
This limitation will be fixed in future release.

### <b>Notes</b>

 1. Some code parts can be executed in the ITCM-RAM (64 KB up to 256kB) which decreases critical task execution time, compared to code execution from Flash memory. This feature can be activated using '#pragma location = ".itcmram"' to be placed above function declaration, or using the toolchain GUI (file options) to execute a whole source file in the ITCM-RAM.
 2.  If the application is using the DTCM/ITCM memories (@0x20000000/ 0x0000000: not cacheable and only accessible by the Cortex M7 and the MDMA), no need for cache maintenance when the Cortex M7 and the MDMA access these RAMs. If the application needs to use DMA (or other masters) based access or requires more RAM, then the user has to:
      - Use a non TCM SRAM. (example : D1 AXI-SRAM @ 0x24000000).
      - Add a cache maintenance mechanism to ensure the cache coherence between CPU and other masters (DMAs,DMA2D,LTDC,MDMA).
      - The addresses and the size of cacheable buffers (shared between CPU and other masters) must be properly defined to be aligned to L1-CACHE line size (32 bytes).
 3.  It is recommended to enable the cache and maintain its coherence:
      - Depending on the use case it is also possible to configure the cache attributes using the MPU.
      - Please refer to the **AN4838** "Managing memory protection unit (MPU) in STM32 MCUs".
      - Please refer to the **AN4839** "Level 1 cache on STM32F7 Series"

#### <b>ThreadX usage hints</b>

 - ThreadX uses the Systick as time base, thus it is mandatory that the HAL uses a separate time base through the TIM IPs.
 - ThreadX is configured with 100 ticks/sec by default, this should be taken into account when using delays or timeouts at application. It is always possible to reconfigure it in the "tx_user.h", the "TX_TIMER_TICKS_PER_SECOND" define,but this should be reflected in "tx_initialize_low_level.s" file too.
 - ThreadX is disabling all interrupts during kernel start-up to avoid any unexpected behavior, therefore all system related calls (HAL, BSP) should be done either at the beginning of the application or inside the thread entry functions.
 - ThreadX offers the "tx_application_define()" function, that is automatically called by the tx_kernel_enter() API.
   It is highly recommended to use it to create all applications ThreadX related resources (threads, semaphores, memory pools...)  but it should not in any way contain a system API call (HAL or BSP).
 - Using dynamic memory allocation requires to apply some changes to the linker file.
   ThreadX needs to pass a pointer to the first free memory location in RAM to the tx_application_define() function,
   using the "first_unused_memory" argument.
   This require changes in the linker files to expose this memory location.
    + For EWARM add the following section into the .icf file:
     ```
	 place in RAM_region    { last section FREE_MEM };
	 ```
    + For MDK-ARM:
	```
    either define the RW_IRAM1 region in the ".sct" file
    or modify the line below in "tx_low_level_initilize.s to match the memory region being used
        LDR r1, =|Image$$RW_IRAM1$$ZI$$Limit|
	```
    + For STM32CubeIDE add the following section into the .ld file:
	```
    ._threadx_heap :
      {
         . = ALIGN(8);
         __RAM_segment_used_end__ = .;
         . = . + 64K;
         . = ALIGN(8);
       } >RAM_D1 AT> RAM_D1
	```

       The simplest way to provide memory for ThreadX is to define a new section, see ._threadx_heap above.
       In the example above the ThreadX heap size is set to 64KBytes.
       The ._threadx_heap must be located between the .bss and the ._user_heap_stack sections in the linker script.
       Caution: Make sure that ThreadX does not need more than the provided heap memory (64KBytes in this example).
       Read more in STM32CubeIDE User Guide, chapter: "Linker script".

    + The "tx_initialize_low_level.s" should be also modified to enable the "USE_DYNAMIC_MEMORY_ALLOCATION" flag.

#### <b>USBX usage hints</b>

- The DTCM (0x20000000) memory region should not be used by application in case USB DMA is enabled
- Should make sure to configure the USB pool memory region with attribute "Non-Cacheable" to ensure coherency between CPU and USB DMA

### <b>Keywords</b>

RTOS, ThreadX, USBX, USB_OTG, Full Speed, DFU, Firmware upgrade

### <b>Hardware and Software environment</b>

  - This example runs on STM32H723xx devices.
  - This example has been tested with STMicroelectronics STM32H723ZG boards Revision MB1364-H723ZG-E01 and can be easily tailored to any other supported device and development board.

  - NUCLEO-H723ZG board Set-up
    - Install the DFU driver available in "DfuSe Demonstrator" installation directory
      For Windows 10 and later : Update STM32 DFU device driver manually from Windows Device Manager.
      The install of required device driver is available under:
      "Program Files\STMicroelectronics\Software\DfuSe v3.0.6\Bin\Driver\Win10" directory.
    - Connect the NUCLEO-H723ZG board to the PC.
    - Connect the NUCLEO-H723ZG board to the PC through micro A-Male to standard A Male cable connected to the connector.
     - CN13 : to use USB High Speed OTG IP in full speed (HS_IN_FS)

### <b>How to use it ?</b>

In order to make the program work, you must do the following :

 - Open your preferred toolchain
 - Rebuild all files and load your image into target memory
 - To run the application, proceed as follows:
   - Install "DfuSe Demonstrator"
   - Install the DFU driver available in "DfuSe Demonstrator" installation directory
   - For Windows 10 and later : Update STM32 DFU device driver manually from Windows Device Manager.
     The install of required device driver is available under:
     "Program Files\STMicroelectronics\Software\DfuSe v3.0.6\Bin\Driver\Win10" directory.
   - Open "DfuSe Demo", choose the "STM32H7xx_Nucleo_LED_Toggle@0x08020000.dfu" provided in Binary
     directory, upgrade and verify to check that it is successfully downloaded.
   - This application allows also to upload a dfu file (either the provided DFU file or by creating a new dfu file).
     To check that the upload was successfully performed, choose the dfu uploaded file, upgrade and verify.
   - To run the downloaded application, execute the command "leave the DFU mode" or simply reset the
     board.
