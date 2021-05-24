
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
#include <string.h>

#include "stm32f746xx.h"
#include "system_stm32f746.h"
#include "sdram.h"
#include "led.h"
#include "eth.h"


#include "lwip/init.h"
#include "lwip/netif.h"
#include "lwip/dhcp.h"
#include "lwip/etharp.h"
#include "lwip/timeouts.h"
#include "lwip/prot/ethernet.h"
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


/**
 * @brief convertbyte to string
 */
char *
convertbyte(uint8_t b, char *p) {

    if( b >= 100 ) {
        *p++ = '0'+(b/100);
        b %= 100;
        *p++ = '0'+(b/10);
        b %= 10;
        *p++ = '0'+b;
    } else if ( b >= 10 ) {
        *p++ = '0'+(b/10);
        b %= 10;
        *p++ = '0'+b;
    } else {
        *p++ = '0'+b;
    }
    *p = 0;
    return p;

}
/**
 * @brief   convert IPv4 Address to (point separated) string
 *
 * @note
 */
int
ip2str(uint32_t ip, char *s) {
char *p = s;

    p = convertbyte((ip>>24)&0xFF,p);
    *p++ = '.';
    p = convertbyte((ip>>16)&0xFF,p);
    *p++ = '.';
    p = convertbyte((ip>>8)&0xFF,p);
    *p++ = '.';
    p = convertbyte((ip)&0xFF,p);
    *p = 0;

    return 0;
}




/**
 * @brief mynetif_input
 *
 * @note  Called by lwip when data is received
 */

static err_t
netif_output(struct netif *netif, struct pbuf *p) {

//    LINK_STATS_INC(link.xmit);

    lock_interrupts();
//    pbuf_copy_partial(p, mac_send_buffer, p->tot_len, 0);
    /* Start MAC transmit here */
    unlock_interrupts();
    return ERR_OK;
}

/**
 * @brief mynetif_input
 *
 * @note  Called by lwip when data is received
 */
err_t mynetif_input() {


    return ERR_OK;
}

/**
 * @brief mynetif_input
 *
 * @note  Called by lwip when data is received
 */
err_t mynetif_output() {


    return ERR_OK;
}


/**
 * @brief mynetif_init
 *
 * @note  Called by lwip to initialize device
 */
err_t mynetif_init(struct netif *netif) {
uint8_t macaddr[6];

    netif->linkoutput = netif_output;
    netif->output     = etharp_output;

    netif->mtu        = ETH_MAX_ETH_PAYLOAD;
    netif->flags      =   NETIF_FLAG_BROADCAST
                        | NETIF_FLAG_ETHARP
                        | NETIF_FLAG_ETHERNET
                        | NETIF_FLAG_IGMP;

    ETH_GetMACAddress(macaddr);
    SMEMCPY(netif->hwaddr, macaddr, ETH_HWADDR_LEN);
    netif->hwaddr_len = ETH_HWADDR_LEN;

    // Initialize device
    ETH_Init();

    // Start device
    ETH_Start();

    return ERR_OK;
}


/**
 * @brief mynetif_status_callback
 *
 * @note  Called every time the status (up,down) of network connection changes
 */
void mynetif_status_callback(struct netif *netif) {

    printf("netif status changed %s\n", ip4addr_ntoa(netif_ip4_addr(netif)));

}



/**
 * @brief mynetif_link_callback
 *
 * @note  Called every time the status (up,down) of network connection changes
 */
void mynetif_link_callback(struct netif *netif) {

    printf("netif status changed %s\n", ip4addr_ntoa(netif_ip4_addr(netif)));

}


/**
 * brief network interface configuration
 */
static struct netif     netif;
static ip4_addr_t       ipaddr;
static ip4_addr_t       netmask;
static ip4_addr_t       gateway;

/**
 * @brief   main
 *
 * @note    Initializes GPIO and SDRAM, blinks LED and test SDRAM access
 */
int main(void) {


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
    ipaddr.addr  = 0;
    netmask.addr = 0;
    gateway.addr = 0;
    netif_add(&netif, &ipaddr, &netmask, &gateway, NULL, mynetif_init, mynetif_input);
    netif.name[0] = 'l';
    netif.name[1] = 'n';
    netif_set_status_callback(&netif, mynetif_status_callback);
    netif_set_default(&netif);

    if( netif_is_up(&netif) ) {
        netif_set_up(&netif);
    } else {
        netif_set_down(&netif);
    }

    netif_set_link_callback(&netif,mynetif_link_callback);


    message("Starting DHCP\n");
    dhcp_start(&netif);
    Delay(100);

    if( verbose ) {
        if( ip4_addr_isany_val(ipaddr) ) {
            char s[20];
            ip2str(ipaddr.addr,s);
            printf("IP Address = %s\n",s);
            ip2str(netmask.addr,s);
            printf("IP Network Mask = %s\n",s);
            ip2str(gateway.addr,s);
            printf("IP Gateway = %s\n",s);
        }
    }

//    message("Starting HTTP server\n");
//    httpd_init();


    /* Check for received frames, feed them to lwIP */
#if 0
    lock_interrupts();
    struct pbuf* p = 0; //queue_try_get(&queue);
    unlock_interrupts();
    if(p != NULL) {
//            LINK_STATS_INC(link.recv);
        if(netif.input(p, &netif) != ERR_OK) {
            pbuf_free(p);
        }
    }
#endif

    // Entering Main loop
    while(1) {
        /* Check link state, e.g. via MDIO communication with PHY */
        //if(link_state_changed()) {
            if(netif_is_up(&netif)) {
                netif_set_link_up(&netif);
            } else {
                netif_set_link_down(&netif);
            }
        //}
        sys_check_timeouts();
    }
}
