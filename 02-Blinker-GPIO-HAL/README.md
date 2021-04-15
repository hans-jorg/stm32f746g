Blink2
======

Introduction
------------

This is still a very simple LED blink, but now it uses a Hardware Abstraction Layer (HAL)
for accessing the GPIO.

The structure of the application is a little bit more elaborated.

    +----------------------------------------------------+
    |                Application                         |
    +----------------------------------------------------+
    |                GPIO HAL                            |
    +----------------------------------------------------+
    |                 Hardware                           |
    +----------------------------------------------------+


It uses the same quick and dirty delay routine, and it should not to be used in 
production code.

GPIO HAL
--------

Each GPIO port controls up to 16 pins.

The HAL is composed of the following routines:

* GPIO_Init
* GPIO_ConfigureOutputPin(int pin)
* void GPIO_TogglePin(int pin)
* void GPIO_SetPin(int pin)
* void GPIO_ClearPin(int pin)

They are all defined in the main.c file.
Using these routines one can access the LEDGPIO pins, specially the LEDPIN.

It does not permit the access other GPIO units. For examplo, it does not permit to access the pin 3 of GPIO port K that controls the LCD. By setting it low, it would be possible to turn off the LCD.


References
----------

1. [STM32F75xxx and STM32F74xxx advanced Arm ® -based 32-bit MCUs Reference Manual - RM0385](https://www.st.com/resource/en/reference_manual/dm00124865-stm32f75xxx-and-stm32f74xxx-advanced-arm-based-32-bit-mcus-stmicroelectronics.pdf)
