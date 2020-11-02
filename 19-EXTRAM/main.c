/**
 * @file     main.c
 * @brief    Blink LEDs using counting delays and CMSIS (Heavy use of macros)
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

/*
 * @brief   Configuration for PLLSAI
 *
 * @note    Assumes PLL Main will use HSE (crystal) and have a 1 MHz input for PLL
 *
 * @note    LCD_CLK should be in range 5-12, with typical value 9 MHz.
 *
 * @note    There is an extra divisor in PLLSAIDIVR[1:0] of RCC_DCKCFGR, that can
 *          have value 2, 4, 8 or 16.
 *
 * @note    So the R output must be 18, 36, 72 or 144 MHz.
 *          But USB, RNG and SDMMC needs 48 MHz. The LCM of 48 and 9 is 144.
 *
 *          f_LCDCLK  = 9 MHz        PLLSAIRDIV=8
 *
 */

PLL_Configuration  pllsaiconfig  = {
    .source         = RCC_PLLCFGR_PLLSRC_HSI,
    .M              = HSE_FREQ/1000,                        // f_IN = 1 MHz
    .N              = 144,                                  // f_VCO = 144 MHz
    .P              = 3,                                    // f_P = 48 MHz
    .Q              = 3,                                    // f_Q = 48 MHz
    .R              = 2                                     // f_R = 72 MHz
};


/**
 * @brief   Quick and dirty delay routine
 *
 * @note    It gives approximately 1ms delay at 16 MHz
 *
 * @note    The COUNTERFOR1MS must be adjusted by trial and error
 *
 * @note    Do not use this or similar in production code
 */

#define COUNTERFOR1MS 300000


void ms_delay(volatile int ms) {
   while (ms-- > 0) {
      volatile int x=COUNTERFOR1MS;
      while (x-- > 0)
         __NOP();
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

    RCC->DCKCFGR1 = (RCC->DCKCFGR1&~RCC_DCKCFGR1_PLLSAIDIVR)|(8<<RCC_DCKCFGR1_PLLSAIDIVR_Pos);
    SystemPLLSAIConfig(&pllsaiconfig);

    LED_Init();

    /*
     * Blink LED
     */
    for (;;) {
#if 1
       ms_delay(500);
       LED_Toggle();
#else
        ms_delay(500);
        LED_Set();
        ms_delay(500);
        LED_Clear();
        ms_delay(500);
#endif
    }
}
