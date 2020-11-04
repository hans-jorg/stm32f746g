
Developing for the STM32F746G using GCC on Linux
================================================

*This is under development. They compile but they are not tested!!!!*

Introduction
------------

A step-by-step programming guide to a Cortex M7 Microcontroller.

The board is a STM32F746GDISCOVERY board (1) with a STM32F746NGH6 with 1 MByte Flash memory and 340 KByte RAM memory.

The Gnu ARM Embedded Toolchain (2) is used.

The host computer is a Linux machine (Linux Mint 20).

The build process is based on Makefiles.


Projects
--------


  * 01-Blinker <br />
    Very simple blink. No symbols.

  * 02-Blinker <br />
    The same but with symbols and macros.

  * 03-Blinker <br />
    Simple blinker with a crude GPIO HAL.

  * 04-Blinker <br />
    Simple blinker with a LED HAL.

  * 05-Blinker <br />
    Simple blinker with a LED HAL in separated files.

  * 06-Blinker <br />
    Simple blinker with a LED HAL over the GPIO HAL.


References
----------

1 - [Discovery Kit with STM32F746NG MCU](https://www.st.com/en/evaluation-tools/32f746gdiscovery.html)

2 - [GNU Arm Embedded Toolchain](https://developer.arm.com/tools-and-software/open-source-software/developer-tools/gnu-toolchain/gnu-rm)

