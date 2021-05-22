
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
#include <stdlib.h>
#include <stdint.h>
#include <stdarg.h>

#include "stm32f746xx.h"
#include "system_stm32f746.h"
#include "sdram.h"
#include "led.h"

#include "lwip/init.h"
#include "lwip/netif.h"
#include "lwip/dhcp.h"
#include "lwip/timeouts.h"

#include "lwip/apps/httpd.h"
#include "ethernetif.h"




static volatile uint32_t tick_ms = 0;
static volatile uint32_t delay_ms = 0;
static int led_initialized = 0;

#define INTERVAL 500
void SysTick_Handler(void) {

    if( !led_initialized ) {
        LED_Init();
        led_initialized = 1;
    }
    if( tick_ms >= INTERVAL ) {
       LED_Toggle();
       tick_ms = 0;
    } else {
       tick_ms++;
    }

    if( delay_ms > 0 ) delay_ms--;

    sys_count();

}

/**
 *  @brief  Delay
 *
 * @note    Delays *delay* milliseconds
 */

void Delay(uint32_t delay) {

    delay_ms = delay;
    while( delay_ms ) {}

}

/**
 * @brief message
 *
 * @note  There are two versions. One using the standard library vprintf function and
 *        another, using a macro with variable number of arguments
 *
 * @note  Macros requires at least two arguments. Particularly, the message must have a specifier.
 *        Dismissed!!!!
 */
int verbose = 1;

void message(char *msg,...) {

    va_list args;

    va_start(args,msg);

    if( verbose ) {
        vprintf(msg,args);
    }
    va_end(args);
}


err_t mynetif_input() {}


err_t mynetif_init() {}

void mynetif_status_callback(struct netif *netif) {}


/**
 * @brief   main
 *
 * @note    Initializes GPIO and SDRAM, blinks LED and test SDRAM access
 */
int main(void) {
struct netif netif;

    message("Starting.at %ld KHz...\n",SystemCoreClock/1000);

    /* Set Clock to 200 MHz */
    SystemConfigMainPLL(&MainPLLConfiguration_200MHz);
    SystemSetCoreClock(CLOCKSRC_PLL,1);

    message("Now running at %ld KHz...\n",SystemCoreClock/1000);

    SysTick_Config(SystemCoreClock/1000);

    printf("Starting SDRAM\n");
    SDRAM_Init();

    message("Initializing LWIP\n");
    lwip_init();

    message("Initializing interface\n");
    netif_add(&netif, IP4_ADDR_ANY, IP4_ADDR_ANY, IP4_ADDR_ANY, NULL, mynetif_init, mynetif_input);
    netif.name[0] = 'l';
    netif.name[1] = 'n';
    netif_set_status_callback(&netif, mynetif_status_callback);
    netif_set_default(&netif);
    netif_set_up(&netif);

    message("Starting DHCP\n");
    dhcp_start(&netif);

    message("Starting HTTP server\n");
    httpd_init();

    // Entering Main loop
    while(1) {


        sys_check_timeouts();
    }
}
