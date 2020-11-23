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
#include "button.h"


/**
 * @brief   Systick routine
 *
 * @note    It is called every 1ms
 *
 */
static volatile int blinkon = 1;

static volatile uint32_t tick_ms = 0;
void SysTick_Handler(void) {

    if( tick_ms >= 500 ) {
       if( blinkon )
           LED_Toggle();
       tick_ms = 0;
    } else {
       tick_ms++;
    }
}

/**
 * @brief   PLL Configuration for 200 MHz
 */

static PLL_Configuration Clock216MHz = {
    .source = CLOCKSRC_HSE,
    .M = HSE_OSCILLATOR_FREQ/1000000,       // f_INT = 1 MHz
    .N = 432,                               // f_VCO = 432 MHz
    .P = 2,                                 // f_OUT = 216 MHz
    .Q = 2,                                 // not used
    .R = 2                                  // not used
};

static PLL_Configuration Clock200MHz = {
    .source = CLOCKSRC_HSE,
    .M = HSE_OSCILLATOR_FREQ/1000000,       // f_INT = 1 MHz
    .N = 400,                               // f_VCO = 400 MHz
    .P = 2,                                 // f_OUT = 200 MHz
    .Q = 2,                                 // not used
    .R = 2                                  // not used
};

/**
 * @brief   main
 *
 * @note    Initializes GPIO and blinks LED
 *
 */

int main(void) {

    //SystemSetCoreClock(CLOCKSRC_HSE,100);

    /* configure clock to 200 MHz */
    SystemConfigMainPLL(&Clock200MHz);
    SystemSetCoreClock(CLOCKSRC_PLL,1);

    SysTick_Config(SystemCoreClock/1000);

    LED_Init();
    Button_Init();

    /* Main */
    for (;;) {
        if( Button_Read() )          // No debounce (yet) !!!!!
            blinkon = !blinkon;
    }
}
