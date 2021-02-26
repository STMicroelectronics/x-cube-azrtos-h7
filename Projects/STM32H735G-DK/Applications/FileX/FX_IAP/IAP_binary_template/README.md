## <b>IAP_binary_template application description</b>

This is an example Application to be used with the IAP_main. This example is meant to demonstrate the base configuration 
that need to be used in order to successfully deploy an application with the IAP_main bootloader.

Flash start address for this application is moved away from the IAP_main bootloader at the address 0x08020000.
In order to configure the new start address, in IAR:

This application should be configured to start from an offset into the flash that does not overlap with the IAP_main application memory sections.
Particularly, linker options should be changed to set the **Vector Table** and the **ROM START** both pointing to **APP_ADDRESS**.

Upon startup, this application will set the VTOR register with its Interrupt Vector Table starting address, so offset should be taken into account. 
This can be achieved by setting the offset to the defined name **VECT_TABLE_OFFSET** located in file **system_stm32h7xx.c**.

This application must be generated as raw binary, this can be achieved by setting the output format of the IDE to **Raw binary**.
The name for the binary should also be specified there as defined by **FW_NAME_STRING** located in "FileX/FX_IAP/IAP_main/FileX/App/app_filex.h".

### <b>Expacted behaviour:</b>
IAP_binary_template should toggle both LEDs.

### <b>Error behaviour:</b>
On failure, red LED should toggle.

### <b>Note</b>
This application can be debugged using IAR by going into **Project** menu and click **Attach to Running Target**.

A pre-built binary can be found under BIN directory.
