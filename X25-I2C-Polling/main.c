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

#include <stdio.h>
#include "stm32f746xx.h"
#include "system_stm32f746.h"
#include "led.h"
#include "i2c-master.h"


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
 * I2C slaves addresses
 *
 * From schematics:
 * Touch address: 01110000 (0x70), which is a 8-bit value but I2C address is 0x038 (7-bit value)
 * Audio address: 00110100 (0x34), which is a 8-bit value but I2C address is 0x1A (7-bit value)
 */
#define TOUCH_ADDR              0x38
#define AUDIO_ADDR              0x1A

/**
 * @brief   main
 *
 * @note    Initializes GPIO and blinks LED
 *
 * @note    Really a bad idea to blink LED
 */

int main(void) {
int rc;

    SystemSetCoreClockFrequency(OPERATING_FREQUENCY);

    LED_Init();

    SystemConfigPLLSAI(&PLLSAIConfiguration_48MHz);

    /*
     * Test if the slaves are detected
     */
     printf("Initializing I2C3....");
     rc = I2CMaster_Init(I2C3,I2C_CONF_MODE_NORMAL|I2C_CONF_FILTER_NONE,0);
     if( rc <= 0 ) {
        printf("Error (%d)\n");
     } else {
        printf("OK\n");
     }
     printf("Detecting Touch Controller ....");
     rc = I2CMaster_Detect(I2C3,TOUCH_ADDR);
     if( rc <= 0 ) {
        printf("Error (%d)\n");
     } else {
        printf("OK\n");
     }
     printf("Detecting Audio Controller ....");
     rc = I2CMaster_Detect(I2C3,AUDIO_ADDR);
     if( rc <= 0 ) {
        printf("Error (%d)\n");
     } else {
        printf("OK\n");
     }
     /*
      * Blink
      */
    for (;;) {
       ms_delay(500);
       LED_Toggle();
    }
}
