Blink
=====

Introduction
------------

This is a new implementation of the blink program. It uses a layered approach with two HAL. One
for the LED and other for the GPIO.

The GPIO HAL was changed. The functions have now a *GPIO \** parameter, that specifies which
GPIO unit is referenced. So it is now possible to use it to operate on all pins controlled by 
a GPIO unit.

This is the case of turning the LCD off, by setting low the pin 3 of GPIO port K.

    +----------------------------------------------------+
    |                Application                         |
    +---------------------------------------+            |
    |                   LED                 |            |
    +---------------------------------------+------------+
    |                   GPIO                             |
    +----------------------------------------------------+
    |                 Hardware                           |
    +----------------------------------------------------+

It uses the same quick and dirty delay routine, and it should not to be used in 
production code.

GPIO HAL
--------

This HAL is composed of the following routines:

* void GPIO\_Init( GPIO\_TypeDef *gpio, uint32\_t imask, uint32_t omask )
* void GPIO\_Set( GPIO\_TypeDef *gpio, uint32\_t mask )
* void GPIO\_Clear( GPIO\_TypeDef *gpio, uint32\_t mask )
* void GPIO\_Toggle( GPIO\_TypeDef *gpio, uint32\_t mask )
* uint32_t GPIO\_Read( GPIO\_TypeDef *gpio )

The pins now are specified as bitmasks. It means a 32-bit integer, where a bit in a certain 
position, specified by a number, refers to a pin with the same number. Although there is only 
16 pins in each GPIO and so, a *uint16_t* would be enough, but internally, the registers have 32 
bit, but only the lower 16 bits are of interest. Since the processor is optimized to work with
32 bit data, the parameters are of type *uint32_t*.




LED HAL
-------

The interface was not changed, but the implementation is based on the GPIO HAL.

* void LED_Init(void)
* void LED_Set(void)
* void LED_Clear(void)
* void LED_Toggle(void)


References
----------

1. [STM32F75xxx and STM32F74xxx advanced Arm ® -based
32-bit MCUs Reference Manual - RM0385](https://www.st.com/resource/en/reference_manual/dm00124865-stm32f75xxx-and-stm32f74xxx-advanced-arm-based-32-bit-mcus-stmicroelectronics.pdf)


