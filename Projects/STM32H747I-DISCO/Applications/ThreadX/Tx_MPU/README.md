
## <b>Tx_MPU application description</b>

This application provides an example of Azure RTOS ThreadX stack usage, it shows how to develop an application using the ThreadX Module feature.
It demonstrates how to load, start and unload modules. In addition, it shows how ThreadX memory protection on modules using the Memory Protection Unit (MPU). 

This project is composed of two sub-projects:

 - Tx_Module_Manager : ThreadX Module Manager code that load and start the module dynamically at runtime.
 - Tx_Module : ThreadX Module code that is to be loaded and started by the module manager dynamically at runtime.


At the module manager stage, the main entry function tx_application_define() is called by ThreadX during kernel start, the application creates 1 thread and 1 message queue:
  - ModuleManager (Prio : 4; Preemption Threshold : 4)
  - ResidentQueue (Size : 16 * ULONG)

ModuleManager thread uses the ThreadX Module Manager APIs to configure, load and start the expected module. ResidentQueue is used to synchronize operations between Module Manager and the loaded Module.

At the module stage, the main entry function default_module_start() is called by ThreadX during module start, the application creates 1 thread:
  - MainThread (Prio : 2; Preemption Threshold : 2)

MainThread is expected to execute data read and write operations to/from user-defined Shared Memory regions. Memory protection is then demonstrated by trapping the Module's attempt at writing to the shared Read Only region. A Memory Fault is then expected before the unload of the module and the module manager continues to run correctly.

####  <b>Expected success behavior</b>

  - LED_GREEN toggles every 500ms.
  - Information regarding the module processing progress printed to the serial port.

#### <b>Error behaviors</b>

LED_RED toggles every 1 second if any error occurs.

#### <b>Assumptions if any</b>
None

#### <b>Known limitations</b>
None

### <b>Notes</b>

1. Memory regions attributes :

  - Kernel Code region:
    * MPU region index: 0
    * Start Address: Dynamically set at the kernel entry function address
    * Size: 32 Bytes
    * Attributes: (XN=0 S=0 C=1 B=1, Preveliged access only, Read Only)

  - Module Code region:
    * MPU region index: 1 through 4
    * Start Address: 0x08020000
    * Size: Code size of module (defined in the preamble)
    * Attributes: (XN=0 S=0 C=1 B=1, All access permitted, Read Only)

  - Module Data region:
    * MPU region index: 5 through 8
    * Start Address: Defined by the module_data_area buffer
    * Size: Defined by the buffer size MODULE_DATA_SIZE
    * Attributes: (XN=1 S=0 C=1 B=1, All access permitted, Read/Write)

  - User defined Shared Memory region 1:
    * MPU region index: 9
    * Start Address: Defined by READONLY_REGION
    * Size: 256 Bytes
    * Attributes: (XN=1 S=0 C=1 B=1, All access permitted, Read Only)

  - User defined Shared Memory region 2:
    * MPU region index: 10
    * Start Address: Defined by READWRITE_REGION
    * Size: 256 Bytes
    * Attributes: (XN=1 S=0 C=1 B=1, All access permitted, Read/Write)

  The MPU regions attributes are specific per MCU, they are set by overriding the access control defines found in the file txm_module_port.h using new ones in the tx_user.h file:
   - TXM_MODULE_MPU_CODE_ACCESS_CONTROL
   - TXM_MODULE_MPU_DATA_ACCESS_CONTROL
   - TXM_MODULE_MPU_SHARED_ACCESS_CONTROL

2. A preamble is required with each Module to expose the module configuration to the Module Manager. Particularly the preamble contains information such as the module unique ID and attributes.
Module Properties (attributes) is a 32bit word, laid out as:
  - Bits 31-24: Compiler ID 0 -> IAR 1 -> ARM 2 -> GNU
  - Bits 23-3: Reserved
  - Bit 2: 0 -> Disable shared/external memory access 1 -> Enable shared/external memory access
  - Bit 1: 0 -> No MPU protection 1 -> MPU protection (must have user mode selected - bit 0 set)
  - Bit 0: 0 -> Privileged mode execution 1 -> User mode execution

For this application demonstrating MPU memory protection on modules, the attributes should be set as follows:
  - Shared memory access is allowed.
  - MPU protection is enabled.
  - User mode is set for the module

The above configuration results in an attributes word equals 0x00000007

3. All C files in a module must #define TXM_MODULE prior to including txm_module.h. Doing so remaps the ThreadX API calls to the module-specific version of the API that invokes the dispatch function in the resident Module Manager to perform the call to the actual API function.

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

### <b>Keywords</b>

RTOS, ThreadX, Thread, Message Queue, Module Manager, Module, MPU


### <b>Hardware and Software environment</b>

  - This example runs on STM32H747xx devices
  - This example has been tested with STMicroelectronics STM32H747I-DISCO boards Revision: MB1248D-01.
    and can be easily tailored to any other supported device and development board.
  - A virtual COM port appears in the HyperTerminal:
      - Hyperterminal configuration:
        + Data Length = 8 Bits
        + One Stop Bit
        + No parity
        + BaudRate = 115200 baud
        + Flow control: None

###  <b>How to use it ?</b>

In order to make the program work, you must do the following :

 - Open Multi-projects workspace using your pereferred IDE
 - Rebuild Tx_Module project
 - Rebuild Tx_Module_Manager project
 - Flash the Tx_Module binary at address defined by MODULE_FLASH_ADDRESS
 - Set the "Tx_Module_Manager" as active application (Set as Active)
 - Run the example