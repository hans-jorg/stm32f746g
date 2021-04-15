Blink4
======

Introduction
------------

This is still a very simple LED blink, but now it uses a Hardware Abstraction Layer (HAL)
for accessing the LED, not the GPIO.


The structure of the application is similar to one used in the anterior project.

    +----------------------------------------------------+
    |                Application                         |
    +----------------------------------------------------+
    |                LED HAL                             |
    +----------------------------------------------------+
    |                Hardware                            |
    +----------------------------------------------------+


It uses the same quick and dirty delay routine, and it should not to be used in 
production code.


LED HAL
--------

There are only one LED that can be controlled by the user.

The LED HAL is composed of the following routines:

* void LED_Init(void)
* void LED_Set(void)
* void LED_Clear(void)
* void LED_Toggle(void)


They are defined as static inline. This means it can only be accessed in the main.c file. And when possible, the 
compiler will insert the code of the function direct in the point, where it is called. So, the cost (time and memory) of calling a routine and then returning is spared.

Inline functions should always be defined as static. If not, a function, that can be called from other file, will be implemented, defeating one of the objectives of inlining a function.

They are all defined in the main.c file.

> NOTE: In C, *function()* means a function that can have any parameter. *function(void)* means a function that has no parameter.


References
----------

1. [STM32F75xxx and STM32F74xxx advanced Arm ® -based 32-bit MCUs Reference Manual - RM0385](https://www.st.com/resource/en/reference_manual/dm00124865-stm32f75xxx-and-stm32f74xxx-advanced-arm-based-32-bit-mcus-stmicroelectronics.pdf)
