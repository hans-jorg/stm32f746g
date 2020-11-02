/**
 * @file     main.c
 * @brief    Blink LEDs using Systick and CMSIS (Heavy use of macros)
 * @version  V1.0
 * @date     06/10/2020
 *
 * @note     The blinking frequency depends on core frequency
 * @note     Direct access to registers
 * @note     No library used
 *
 *
 ******************************************************************************/

#include "stm32f746xx.h"
#include "system_stm32f746.h"
#include "led.h"


/**
 * @brief   Systick routine
 *
 * @note    It is called every 1ms
 *
 */

static volatile uint32_t tick_ms = 0;
void SysTick_Handler(void) {

    if( tick_ms >= 500 ) {
       LED_Toggle();
       tick_ms = 0;
    } else {
       tick_ms++;
    }
}

/**
 * @brief   main
 *
 * @note    Initializes GPIO and blinks LED
 *
 * @note    Really a bad idea to blink LED
 */

int main(void) {

    /* configure clock to 200 MHz */
    SystemSetCoreClock(CLOCKSRC_PLL,1);

    SysTick_Config(SystemCoreClock/1000);

    LED_Init();

    /* Main */
    for (;;) {}
}
