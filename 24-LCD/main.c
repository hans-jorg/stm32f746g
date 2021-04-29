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
#include "lcd.h"



#define OPERATING_FREQUENCY (200000000)

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


    SystemSetCoreClockFrequency(OPERATING_FREQUENCY);

    LED_Init();

    LCD_Init();

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
