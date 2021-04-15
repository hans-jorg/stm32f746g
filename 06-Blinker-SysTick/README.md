Blink6
======

Introduction
------------

This project uses the same structure of the last one, a layered architecture with two HALs, one for GPIOs and other for LEDs.$

The main difference is in how to control timing. The anterior projects used a quick and dirty way to generate the delays, by cycle counting. This represents a waste of resource like processor time and energy.

A better approach is to use timers. Practically, all Cortex-M have a SysTick timer. This is a 24 bit count down register, that can be driven by the core clock or an alternate clock source.

The CMSIS *SystemCoreClock* global variable contains the frequency of the core clock and must be updated every time, the clock changes.

Considering a clock frequency of 20 MHz (20.000.000 Hz), loading the SysTick timer with 20.000 and configuring it to generated a interrupt every time it reaches 0, there will be 1.000 interrupts every second. This means an interrupt every one millisecond.

There is a CMSIS SysTick_Config routine to configure it. With the code below

    SysTick_Config(SystemCoreClock/1000);

SysTick will generate an interrupt every 1 ms.

When an interrupt occurs, the SysTick_Handler function will be called. Every 500 times, the routine is called, the LED is toggled. And so it blinks every second.

    static volatile uint32_t tick_ms = 0;
    void SysTick_Handler(void) {

        if( tick_ms >= 500 ) {
           LED_Toggle();
           tick_ms = 0;
        } else {
           tick_ms++;
        }
    }



References
----------

1. [STM32F75xxx and STM32F74xxx advanced Arm ® -based
32-bit MCUs Reference Manual - RM0385](https://www.st.com/resource/en/reference_manual/dm00124865-stm32f75xxx-and-stm32f74xxx-advanced-arm-based-32-bit-mcus-stmicroelectronics.pdf)

