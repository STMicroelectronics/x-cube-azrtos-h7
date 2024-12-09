# <b>IAP using Cypress WiFi module</b>

<center>
Copyright 2016 STMicroelectronics

![](../../../../../_htmresc/st_logo_2020.png)
</center>

This application shows how to use the Cypress WiFi module to perform an IAP ("In Application Programming") using STM32 Cube HAL.

This application can be run with the following way:

Using a remote PC

<center>

![](./_htmresc/scenario_1.png)
</center>

In this scenario, the STM32H747ZI-DISCO board equipped, with the WiFi Module Cypress,
 joins, alongside a PC, a WiFi access point.


The PC is running a HTTP Web Server, [the Wamp Server](http://www.wampserver.com/en/) for example,
 that hosts the "hello.bin" FW file.
 
Once WiFi access point is joined, the STM32H747ZI-DISCO board establishes a TCP connection to the PC and sends a request to download the "hello.bin.

As soon the downloading is finished the "hello.bin" file is written to the flash.
 

Once done, the board will run the "hello.bin" application when rebooted.

## <b>Note</b>

After flashing the binary file, the IAP original application can be restarted by rebooting the board while pressing down the "USER" button.
