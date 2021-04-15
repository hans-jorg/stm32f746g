Blink3
======

Introduction
------------

This is basically the same implementation used in the last project. The difference is that the 
implementation is moved to two files. led.c contains the implementation of LED_Init, because it 
is big and generally, the compiler does not insert big chucks of code. The inline declaration is 
a suggestion to the compiler. It is not mandatory.

But the routines LED_Set, LED_Clear and LED_Toggle are small and good candidates for inlining.

The main function does not need to be changed, because the HAL remains unchanged.


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


The LED_Set, LED_Clear and LED_Toggle are defined as static inline in led.h. This means it can only
 be accessed in the C file, where it is included. The LED_Init is in the led.c file.

Inline functions should always be defined as static. If not, a function, that can be called 
from other file, will be implemented, defeating one of the objectives of inlining a function.




References
----------

1. [STM32F75xxx and STM32F74xxx advanced Arm ® -based 32-bit MCUs Reference Manual - RM0385](https://www.st.com/resource/en/reference_manual/dm00124865-stm32f75xxx-and-stm32f74xxx-advanced-arm-based-32-bit-mcus-stmicroelectronics.pdf)
