Button
======

Introduction
------------

The STM32F746G Discovery board has two buttons. The black one is used to reset the board and the blue one
can have its status read by the MCU.

It is connected to pin 11 of the GPIO port I.

This program blinks the LED and pressing the buttons can stop and restart the blinking.



Button HAL
----------

The following routines are used to configure the GPIO in order to read the button status.

* void     ButtonInit(void)
* uint32_t Button_Read(void)

NOTE: There is no debouncing!!!!!

References
----------

1. [STM32F75xxx and STM32F74xxx advanced Arm ® -based
32-bit MCUs Reference Manual - RM0385](https://www.st.com/resource/en/reference_manual/dm00124865-stm32f75xxx-and-stm32f74xxx-advanced-arm-based-32-bit-mcus-stmicroelectronics.pdf)