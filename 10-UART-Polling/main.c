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
#include "uart.h"


/**
 * @brief   Systick routine
 *
 * @note    It is called every 1ms
 *
 */

static volatile uint32_t tick_ms = 0;
static volatile uint32_t delay_ms = 0;

#define INTERVAL 500
void SysTick_Handler(void) {

    if( tick_ms >= INTERVAL ) {
       LED_Toggle();
       tick_ms = 0;
    } else {
       tick_ms++;
    }

    if( delay_ms > 0 ) delay_ms--;

}

void Delay(uint32_t delay) {

    delay_ms = delay;
    while( delay_ms ) {}

}

static PLL_Configuration Clock200MHz = {
    .source = CLOCKSRC_HSE,
    .M = HSE_OSCILLATOR_FREQ/1000000,       // f_INT = 1 MHz
    .N = 400,                               // f_VCO = 400 MHz
    .P = 2,                                 // f_OUT = 200 MHz
    .Q = 2,                                 // not used
    .R = 2                                  // not used
};


static const uint32_t uartconfig =  UART_NOPARITY | UART_8BITS | UART_STOP_2 |
                                    UART_BAUD_9600;
/**
 * @brief   main
 *
 * @note    Initializes GPIO and blinks LED
 *
 * @note    Really a bad idea to blink LED
 */

int main(void) {
int c;

#if 1
    /* configure clock to 200 MHz */
    SystemSetCoreClock(CLOCKSRC_PLL,1);
#else
    SystemConfigMainPLL(&Clock200MHz);
    SystemSetCoreClock(CLOCKSRC_PLL,1);
#endif
    SysTick_Config(SystemCoreClock/1000);

    LED_Init();

    UART_Init(UART_1,uartconfig);
//    ITM_Init();

//   __enable_irq();
    /* Main */
    c = 'A';
    for(;;) {
#if 0
        // Output test
        UART_WriteChar(UART_1,c);
//        ITM_SendChar('a'+(c-'A'));
        if( c == 'Z' ) {
            UART_WriteChar(UART_1,'\n');
            UART_WriteChar(UART_1,'\r');
            c = 'A';
        } else {
            c++;
        }
        Delay(1);
#endif
#if 1
        // Echo
          if( UART_GetStatus(UART_1)&UART_RXNOTEMPTY ) {
              c = UART_ReadChar(UART_1);
              if( c == '\r' ) {
                  UART_WriteChar(UART_1,'\n');
              }
              UART_WriteChar(UART_1,c);
          }
          Delay(100);   // Simulate load
#endif
    }
}
