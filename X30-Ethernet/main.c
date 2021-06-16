
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
#include <ctype.h>

#include "stm32f746xx.h"
#include "system_stm32f746.h"
#include "sdram.h"
#include "led.h"
#include "eth.h"


#include "lwip/init.h"
#include "lwip/netif.h"
#include "netif/ethernet.h"
#include "lwip/dhcp.h"
#include "lwip/etharp.h"
#include "lwip/timeouts.h"
#include "arch/cc.h"
#include "arch/sys_arch.h"
#include "lwip/tcp.h"
#include "lwip/prot/ethernet.h"
#include "lwip/apps/tftp_server.h"
#include "ethernetif.h"


/** 
 * @brief Configuration
 */
///@{
//#define USE_HTTPD               1
#define USE_TFTP                  1
///@}


/**
 * @brief IP_PORT
 *
 * @note  Port number where the server listens
 */
#define IP_PORT              8080



////////////////// Timing Functions  /////////////////////////////////////////////////////////////

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
 * @brief   STOP
 *
 * @note    called when an error occurs
 */
void STOP(int code) {
static int static_code;

    static_code = code;
    while (1) {}
}


////////////////// Auxiliary Functions  ///////////////////////////////////////////////////////////

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
 * @brief   int2str
 *
  * @brief  Convert an integer to a string, avoiding string overflow
 *
 * @note    When an overflow is detected, the field is filled with astarisks
 *
 * @note    len must account for the null terminator
 */
int
int2str(int n, char *s, int len) {
unsigned x;
unsigned p10,p10ant;
int e10 = 1;

    if( n < 0 )
        x = (unsigned) (-n);
    else
        x = n;

    p10 = p10ant = 1;
    e10 = 0;
    while( (p10 < x) && (p10ant <= p10) ) {
        p10ant = p10;
        p10 *= 10;
        e10++;
    }
    if( e10 > len ) {
        char *q = s+len-1;
        while( s < q ) *s++ = '*';
        *s = 0;
        return -1;
    }
    p10 = p10ant;
    while( p10 > 1 ) {
        int d = 0;
        while( x >= p10 ) {
            x -= p10;
            d++;
        }
        *s++ = (d+'0');
        p10 /= 10;
    }
    *s++ = x+'0';
    *s = 0;
    return 0;
}

/**
 * @brief   hexdump
 *
 * @note    print a memory dump
 */
static int
hexdump(void *area, int size, unsigned addr) {
unsigned offset;
unsigned a = addr;
unsigned char *c;
int i;

    for(offset=0; offset<size; offset+=16) {
        printf("%04X ",a);
        c = ((unsigned char *) area) + offset;
        for(i=0;i<16;i++) {
            if( i == 8 )
                printf("  ");
            printf("%02X",c[i]);
        }
        printf("  ");
        for(i=0;i<16;i++) {
            if( i == 8 )
                printf(" ");
            if( isprint(c[i])) {
                putchar(c[i]);
            } else {
                putchar('.');
            }
        }
        putchar('\n');
        a += 16;
    }
    return 0;
}


/**
 * @brief message
 *
 * @note  This version uses the standard library vprintf function.
 *
 *
 * @note  Another version using a macro with variable number of arguments was tried. But
 *        such macros must have at least two arguments. Dismissed!!!!
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


///////////////////// LWIP Functions //////////////////////////////////////////////////////////////

/**
 * @brief stnetif_output
 *
 * @note  Called by user to transmit data
 */

err_t
stnetif_output(struct netif *netif, struct pbuf *p) {

#if MIB2_STATS
  LINK_STATS_INC(link.xmit);
  /* Update SNMP stats (only if you use SNMP) */
  MIB2_STATS_NETIF_ADD(netif, ifoutoctets, p->tot_len);
  int unicast = ((p->payload[0] & 0x01) == 0);
  if (unicast) {
    MIB2_STATS_NETIF_INC(netif, ifoutucastpkts);
  } else {
    MIB2_STATS_NETIF_INC(netif, ifoutnucastpkts);
  }
#endif

  lock_interrupts();
//  pbuf_copy_partial(p, mac_send_buffer, p->tot_len, 0);
  /* Start MAC transmit here */
  unlock_interrupts();
  return ERR_OK;
}

/**
 * @brief stnetif_input
 *
 * @note  Called by lwip when data is received
 */
err_t
stnetif_input(struct netif *netif) {


    return ERR_OK;
}

/**
 * @brief stnetif_link
 *
 * @note  
 */
err_t stnetif_link(struct netif *netif) {


    return ERR_OK;
}


/**
 * @brief stnetif_init
 *
 * @note  Called by lwip to initialize device
 */
err_t
stnetif_init(struct netif *netif) {
uint8_t macaddr[6];

    netif->linkoutput = stnetif_output;
    netif->output     = etharp_output;
//    netif->input      = etharp_input;
#if LWIP_IPV6
    netif->output_ip6 = ethip6_output;
#endif

    netif->mtu        =   ETH_MTU;
    netif->flags      =   NETIF_FLAG_BROADCAST
                        | NETIF_FLAG_ETHARP
                        | NETIF_FLAG_ETHERNET
                        | NETIF_FLAG_IGMP;

    ETH_GetMACAddressAsVector(macaddr);
    SMEMCPY(netif->hwaddr, macaddr, ETH_HWADDR_LEN);
    netif->hwaddr_len = ETH_HWADDR_LEN;

    // Initialize device
    ETH_Init();

    // Start device
    ETH_Start();

    return ERR_OK;
}


/**
 * @brief stnetif_status_callback
 *
 * @note  Called every time the status (up,down) of network connection changes
 */
void stnetif_status_callback(struct netif *netif) {

    message("netif status changed %s\n", ip4addr_ntoa(netif_ip4_addr(netif)));

}



/**
 * @brief stnetif_link_callback
 *
 * @note  Called every time the status (up,down) of network connection changes
 */
void stnetif_link_callback(struct netif *netif) {

    message("netif status changed %s\n", ip4addr_ntoa(netif_ip4_addr(netif)));

}

///////////////////// TFTP Functions //////////////////////////////////////////////////////////////


#define ONLY_FILE  ((void *) 1)
static void*
tftp_open(const char* fname, const char* mode, u8_t is_write) {

    if (is_write) {
        return NULL;                    // not yet
    } else {
        return ONLY_FILE;
    }
}

static void
tftp_close(void* handle) {

    return;
}

static int counter = 0;

static int
tftp_read(void* handle, void* buf, int len) {
int rc;

    if( handle != ONLY_FILE )
        return -1;

    rc = int2str(counter,buf,len);
    counter++;
    return rc;
}

static int
tftp_write(void* handle, struct pbuf* p) {

    if( handle != ONLY_FILE )
        return -1;

    while (p != NULL) {
        hexdump(p->payload,p->len,0);
        p = p->next;
    }
    return 0;
}

static const struct tftp_context
tftp_config = {
  tftp_open,
  tftp_close,
  tftp_read,
  tftp_write
};

//////////////////////// LWIP Data /////////////////////////////////////////////////////////////////

/**
 * brief network interface configuration
 */
static struct netif     netif;

// Generate an 32-bit integer with network order of a network address or mask
#if BYTE_ORDER == LITTLE_ENDIAN
#define IPV4(A,B,C,D) (((u32_t)(D)<<24)|((u32_t)(C)<<16)|((u32_t)(B)<<8)|((u32_t)(A)<<0))
#else
#define IPV4(A,B,C,D) (((u32_t)(A)<<24)|((u32_t)(B)<<16)|((u32_t)(C)<<8)|((u32_t)(D)<<0))
#endif

#if LWIP_DHCP
static ip4_addr_t       ipaddr;
static ip4_addr_t       netmask;
static ip4_addr_t       gateway;
#else
static ip4_addr_t       ipaddr    = { IPV4(192,168,0,201) };
static ip4_addr_t       netmask   = { IPV4(255,255,255,1)   };
static ip4_addr_t       gateway   = { IPV4(192,168,0,1) };
#endif


#define IFNAME0         'e'
#define IFNAME1         't'


#define HOSTNAME        "lwipt"

///////////////////// LWIP Functions ///////////////////////////////////////////////////////////////

#define MESSAGE(text)  message(text)


void LWIP_CheckLink(void) {

    if( netif_is_up(&netif) ) {
        netif_set_up(&netif);
    } else {
        netif_set_down(&netif);
    }

}

/**
 * @brief   Initialize lwIP
 * 
 * @note    Do all initialization for lwIP
 */

void LWIP_Init(void) {
err_t err;

    MESSAGE("Initialing lwip");
    lwip_init();
/*
    test_netif_init
 */
    MESSAGE("Initializing interface\n");
#if LWIP_DHCP
    ipaddr.addr  = 0;
    netmask.addr = 0;
    gateway.addr = 0;
#endif
    netif_add(  &netif, 
                &ipaddr, 
                &netmask, 
                &gateway, 
                NULL, 
                stnetif_init, 
                ethernet_input);

    netif.name[0] = 'e';
    netif.name[1] = 't';

    netif_set_status_callback(&netif, stnetif_status_callback);
    netif_set_default(&netif);


    netif_set_link_callback(&netif,stnetif_link_callback);

#if LWIP_DHCP
    MESSAGE("Starting DHCP\n");
    // dchp_set_struct
    err = dhcp_start(&netif);
    Delay(100);
    if( err != ERR_OK ) MESSAGE("DHCP Error");
#endif

#if USE_TFTP 
    message("Starting TFTP server\n");
    tftp_init(&tftp_config);
#endif

#if USE_HTTPD
    // not tested yet!!! Not configured too.
    // It uses TCP!!!
    message("Starting HTTP server\n");
    httpd_init();
#endif
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

}

/**
 * @brief   LWIP processing in the main loop
 * 
 * @note    See lwip-2.1.2/doc/doxygen/output/html/group__lwip__nosys.html
 *          Needs a queue data structure
 */
void LWIP_Process(void) {

    LWIP_CheckLink();

    stnetif_input(&netif);
            
    // Check timers
    sys_check_timeouts();

#if LWIP_NETIF_LOOPBACK
    netif_poll(&netif);
#endif

#if !LWIP_NETIF_LOOPBACK_MULTITHREADING
    // could call netif_poll twice for netif
    netif_poll_all();
#endif
}

///////////////////// Main Function ///////////////////////////////////////////////////////////////

/**
 * @brief   main
 *
 * @note    Initializes GPIO and SDRAM, blinks LED and test SDRAM access
 */
int main(void) {
err_t rc;

    // Disable buffering for stdout (Trying)
    setvbuf(stdout, NULL,_IONBF, 0);

    message("Starting.at %ld KHz...\n",SystemCoreClock/1000);

    /* Set Clock to 200 MHz */
    SystemConfigMainPLL(&MainPLLConfiguration_200MHz);
    SystemSetCoreClock(CLOCKSRC_PLL,1);

    message("Now running at %ld KHz...\n",SystemCoreClock/1000);

    // Set SysTick to 1 ms
    SysTick_Config(SystemCoreClock/1000);

    printf("Starting SDRAM\n");
    SDRAM_Init();

    message("Initializing LWIP\n");
    LWIP_Init();

    // Entering Main loop
    while(1) {
        LWIP_Process();

        // Application code here
    }
}
