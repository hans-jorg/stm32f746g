Developing for the STM32F746G using GCC on Linux
================================================

>NOTE 1: This shows software under development. They may compile but they are not tested!!!!

> NOTE 2: Only open source st-link (st-flash and st-utils) is fully tested.


Introduction
------------

A step-by-step programming guide to a Cortex M7 Microcontroller.

The board is a STM32F746GDISCOVERY board [1]     with a STM32F746NGH6 with 1 MByte Flash memory and 340 KByte RAM memory.

The Gnu ARM Embedded Toolchain [2] is used.

The host computer is a Linux machine (Linux Mint 20).

The build process is based on Makefiles.


Requisites
----------


> NOTE: It is recommend to upgrade the firmware in the STM32F746G Discovery board using the software in [8]


These software are needed to develop embedded applications on a STM32F7 microcontroller.

* GNU Arm Toolchain [2]
* STM32Cube software [3]
* GDB support
    - stlink [4]
    - STLink GDB Server (part of [5]. Do not confuse with [9])
* Flash memory programming
    - st-flash (part of stlink) [4]
    - STLink Utility [6]
    - STCubeProgrammer [7]
* Make

Recommended additional software are:

* Git: version control system
* Doxygen: Documentation tool

Projects
--------

|  #  |  Project            |  Description                         | Status
|-----|---------------------|--------------------------------------|------------
| 01  | Blinker-Simple      | A very simple LED blinker            | OK
| 02  | Blinker-GPIO-HAL    | Simple blinker with a GPIO HAL       | OK 
| 03  | Blinker-LED-HAL     | Simple blinker with a LED HAL        | OK 
| 04  | Blinker-LED-HAL-V2  | Simple blinker with a better LED HAL | OK 
| 05  | Blinker-LED-GPIO-HAL| Simple blinker with a LED/GPIO HAL   | OK 
| 06  | Blinker-SysTick     | Simple blinker using SysTick         | OK 
| 07  | Blinker-200MHz      | Changing the CPU clock frequency     | OK
| 08  | Button              | Reading button status                | OK
| 09  | UART-Polling        | UART interface with polling          | OK
| 10  | UART-Interrupts     | UART interface with interrupts       | OK
| 11  | Infos               | Show MCU Info                        | OK
| 12  | Ministdio           | A minimal stdio (printf)             | OK
| 13  | ----                | -----                                | -
| 14  | Newlib              | Using newlib (already in ARM GNU C)  | OK
| 15  | TimeTriggered-v1    | Pont's time triggered system (old)   | OK 
| 16  | TimeTriggered-v2    | Pont's time triggered system (new)   | OK
| 17  | ucos2               | Using uC/OS-II                       | OK 
| 18  | ----                | ----                                 | -
| 19  | ----                | ----                                 | -
| 20  | ----                | ----                                 | -
| 21  | ----                | ----                                 | -
| 22  | ExternalRAM         | External RAM                         | OK
| 23  | Buddy               | Using buddy allocator on external RAM| OK
| 24  | LCD                 | Using the on board LCD               | OK
| X25 | LCD-ChromeArt       | Using DMA2D to manipulate display    | ?
| X27 | Touch               | Getting touch information (I2C)      | ?
| X28 | MicroSD             | Using the on board MicroSD           | ?
| X29 | Flash               | Using the on board flash memory      | ?
| X30 | Ethernet            | Using the on board Ethernet and lwIP | ?



Notes
-----

1. OpenOCD under development

Makefile configuration   
Flash programming (telnet 4444)  


2. UART with interrupt

Single byte buffer not tested.

2. TODO: Teste parameters for compilation

Effects and use of -specs=nosys.specs -specs=nano.specs -specs=rdimon.specs (-lrdimon)

Use minimal newlib: --specs=nano.specs

3. Test semihosting

Test ITM-SendChar  

monitor arm semihosting enable  


    #include <stdio.h>
    #include <stdlib.h>
    # ….

    extern void initialise_monitor_handles(void);

    int main(void) {
      initialise_monitor_handles();

      printf("Hello !\n");

      HAL_Init();
      SystemClock_Config();

      puts("Check your openocd console.\n");
      printf("This works too\n");
      //……………
    }


References
----------

1 - [Discovery Kit with STM32F746NG MCU](https://www.st.com/en/evaluation-tools/32f746gdiscovery.html)

2 - [GNU Arm Embedded Toolchain](https://developer.arm.com/tools-and-software/open-source-software/developer-tools/gnu-toolchain/gnu-rm)

3 - [STM32Cube MCU Package for STM32F7 series](https://www.st.com/en/embedded-software/stm32cubef7.html)

4 - [Open source version of the STMicroelectronics STlink Tools](https://github.com/stlink-org/stlink)

5 - [Integrated Development Environment for STM32](https://www.st.com/en/development-tools/stm32cubeide.html)

6 - [STM32 ST-LINK utility (replaced by STM32CubeProgrammer)](https://www.st.com/content/st_com/en/products/development-tools/software-development-tools/stm32-software-development-tools/stm32-programmers/stsw-link004.html)

7 - [STM32CubeProgrammer software for all STM32](https://www.st.com/en/development-tools/stm32cubeprog.html)

8 - [ST-LINK, ST-LINK/V2, ST-LINK/V2-1, STLINK-V3 boards firmware upgrade](https://www.st.com/content/st_com/en/products/development-tools/software-development-tools/stm32-software-development-tools/stm32-programmers/stsw-link007.html)

9 - [ST-LINK server software module ](https://www.st.com/content/st_com/en/products/development-tools/software-development-tools/stm32-software-development-tools/stm32-performance-and-debuggers/st-link-server.html)

