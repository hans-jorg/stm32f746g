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

/**
 * @brief   Quick and dirty delay routine
 *
 * @note    It gives approximately 1ms delay at 16 MHz
 *
 * @note    Do not use this or similar in production code
 */

void ms_delay(volatile int ms) {
   while (ms-- > 0) {
      volatile int x=300000;
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

    /*
     * Enable clock for GPIOI
     */
    RCC->AHB1ENR |= 0x000001000U;
    // Alternative (ughh!!))
    //*((uint32_t *) (0x40023C00+0x30) |= 0x000001000U;

    /*
     * Configure GPIO to drive LED
     */
    // Set LED pin to output
    GPIOI->MODER    = (GPIOI->MODER&~0x0000000CU)|0x00000004U;
    // Set pin type
    GPIOI->OTYPER   = GPIOI->OTYPER&~0x0000000CU;
    // Set pin SPEED)
    GPIOI->OSPEEDR  = GPIOI->OSPEEDR|0x0000000CU;
    // Set pullup/pushdown resistors configuration
    GPIOI->PUPDR    = GPIOI->PUPDR&~0x0000000CU;
    // Turn off LED
    GPIOI->ODR     &=  ~0x00000002U; // =0xFFFFFFFDU

    /*
     * Blink LED
     */
    for (;;) {
#if 1
       ms_delay(500);
       GPIOI->ODR ^= 0x00000002U;             // Use XOR to toggle output
#else
       /* Alternative
        * Writing a 1 to lower 16 bits of BSRR set the corresponding bit
        * Writing a 1 to upper 16 bits of BSRR clear the correspoding bit
       */
        ms_delay(500);
        GPIOI->BSRR = 0x00000002U;            // Turn on LED
        ms_delay(500);
        GPIOI->BSRR = 0x00020000U;      // Turn off LED
#endif
    }
}
