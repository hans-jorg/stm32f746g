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
#include "tte.h"

/**
 * @brief   Systick routine
 *
 * @note    It is called every 1ms
 *
 */
static volatile int blinking = 1;

static volatile uint32_t tick_ms = 0;
void SysTick_Handler(void) {

    Task_Update();
    tick_ms++;
}

void Blink(void) {
    if( blinking )
        LED_Toggle();
    else
        LED_Clear();
}

/**
 * @brief   main
 *
 * @note    Initializes GPIO and blinks LED
 *
 */

int main(void) {
int taskno_blink;

    //SystemSetCoreClock(CLOCKSRC_HSE,100);

    /* configure clock to 200 MHz */
    SystemConfigMainPLL(&MainPLLConfiguration_200MHz);
    SystemSetCoreClock(CLOCKSRC_PLL,1);

    SysTick_Config(SystemCoreClock/1000);

    LED_Init();
    Button_Init();
    Task_Init();

    taskno_blink  = Task_Add(Blink,500,0);

    /* Main */
    for (;;) {
        Task_Dispatch();
    }
}
