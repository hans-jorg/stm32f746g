Blink
=====

Introduction
------------

This is a very simple LED blink. It shows how to compile a program, upload and 
debug it.

It uses a quick and dirty delay routine, and it should not to be used in 
production code.

The STM32F746G Discovery board has a LED connected to pin 1 of GPIO port I. 
This is not documented but can be found in the schematics.

Furthermore, pin 3 of GPIO port K controls the LCD. By setting it low, it is 
possible to turn off the LCD.

GPIO
----

Each GPIO port controls up to 16 pins.

To use a GPIO port pin:

1. The clock for the specific port must be enable by setting the corresponding 
bit in the RCC->AHB1ENR register.
2. The pin must be configured as input or output. There are additional options 
for pullup, pulldown, open-drain, speed, etc.


The GPIO registers are:

MODER:      pin is used as input, output, alternate or analog  
OTYPER:     pin uses push-pull or open-drain output  
OSPEEDR:    pins is low, medium, high and very high speed  
PUPDR       normal, pull-up or pull-down  
ODR:        set pins low or high  
IDR:        get pin status  
BSRR:       set pins high and low with one operation  


MODER, OSPEEDR and PUPDR use a 2-bit field for each pin. To set a field 
to a specified value, it must be cleared before, by AND-ing with a mask with 
all field bits cleared. The other bits of this mask must be set to 1.

ODR and OTYPER use a 1-bit field. It can be set by OR-ing and AND-ing directly.

BSRR is a write only register and has two fields. Writing a 1 on one of the 
higher order 16 bits set the corresponding pin to low. Writing a 1 on one of 
the lower order 16 bits, set the corresponding pin to high. Writing a 0 does 
not change the pin.

References
----------

1. [STM32F75xxx and STM32F74xxx advanced Arm ® -based
32-bit MCUs Reference Manual - RM0385](https://www.st.com/resource/en/reference_manual/dm00124865-stm32f75xxx-and-stm32f74xxx-advanced-arm-based-32-bit-mcus-stmicroelectronics.pdf)
