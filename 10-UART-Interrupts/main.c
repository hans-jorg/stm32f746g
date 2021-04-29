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

    /* configure clock to 200 MHz */
    SystemConfigMainPLL(&MainPLLConfiguration_200MHz);
    SystemSetCoreClock(CLOCKSRC_PLL,1);

    SysTick_Config(SystemCoreClock/1000);

    LED_Init();

    UART_Init(UART_1,uartconfig);

    UART_WriteString(UART_1,"UART Test\n\r");
    /* Main loop */
    c = 'a';
    for(;;) {
        // Echo
        while( UART_GetStatus(UART_1)&UART_RXNOTEMPTY ) {
            c = UART_ReadChar(UART_1);
            if( c == '\r' ) {
                UART_WriteString(UART_1,"\n\r");
            } else if ( c == '\x1B') {
                UART_WriteString(UART_1,"0123456789");
            } else {
                UART_WriteChar(UART_1,c);
            }
        }
        Delay(100);   // Simulate load
    }
}
