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
#include <string.h>

#include "stm32f746xx.h"
#include "system_stm32f746.h"
#include "led.h"
#include "sdram.h"
#include "buddy.h"



/*
 * @brief   Configuration for Main PLL
 *
 * @note    PLL Main will use HSE (crystal) and uses a 1 MHz input for PLL
 */

static PLL_Configuration Clock200MHz = {
    .source = CLOCKSRC_HSE,
    .M = HSE_OSCILLATOR_FREQ/1000000,       // f_INT = 1 MHz
    .N = 400,                               // f_VCO = 400 MHz
    .P = 2,                                 // f_OUT = 200 MHz
    .Q = 2,                                 // not used
    .R = 2                                  // not used
};


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
 *  @brief  my_rand
 *
 *  @note my_rand is an implementation of random number generater described by S. Park
 *        and K. Miller, in Communications of the ACM, Oct 88, 31:10, p. 1192-1201.
 */

long seed = 313;
long int my_rand(void) {
long int lo, hi, test;
static long int a = 16807L, m = 2147483647L, q = 127773L, r = 2836L;

    hi = seed / q;
    lo = seed % q;
    test = a * lo - r * hi;

    if (test > 0) {
        seed = test; /* test for overflow */
    } else {
        seed = test + m;
    }
    return seed;
}

int round2(int x) {
int p = 1;
int pant = 1;

    if( x < 0 )
        return 0;

    while( (p>pant) && (p < x) )  {
        pant = p;
        p<<=1;
    }

    return p;
}

/**
 * @brief   main
 *
 * @note    Initializes GPIO and blinks LED
 *
 * @note    Really a bad idea to blink LED
 */
#define LINEMAX 100
#define TRIES  1000


int main(void) {
char line[LINEMAX+1];
const long MINSIZE = 8192;
typedef struct {
    char    *address;
    long    size;
    long    size2;
    long    pattern;
} info_t;
info_t info[TRIES];
int ninfo = 0;


    LED_Init();

    printf("Starting.at %ld KHz...\n",SystemCoreClock/1000);

    /* Set Clock to 200 MHz */
    SystemConfigMainPLL(&Clock200MHz);
    SystemSetCoreClock(CLOCKSRC_PLL,1);

    printf("Now running at %ld KHz...\n",SystemCoreClock/1000);

    SysTick_Config(SystemCoreClock/1000);

    printf("Press ENTER to initialize ExtRAM\n");
    fgets(line,LINEMAX,stdin);
    SDRAM_Init(SDRAM_BANK1);

    printf("Initializing buddy allocator\n");
    Buddy_Init((char *) SDRAM_ADDRESS,SDRAM_SIZE,MINSIZE);

    /*
     * Blink LED
     */
    while ( ninfo<TRIES ) {
        LED_Toggle();
        unsigned s = my_rand();
        s %= (SDRAM_SIZE/4);
        char *p = Buddy_Alloc(s);
        printf("Allocated block #%1d with size %6d at address %p\n",ninfo,s,p);
        if( p ) {
            int pat = my_rand();
            info[ninfo].address = p;
            info[ninfo].size    = s;
            info[ninfo].size2   = round2(s);
            info[ninfo].pattern = pat;
            memset(p,pat,s);
            ninfo++;
        }
        int x;
        if( ((x=my_rand()>>3)&1) && (ninfo>0) ) {
            x = x%ninfo;
            p = info[x].address;
            if( p ) {
                printf("Freed #%1d at address %p\n",x,p);
                Buddy_Free(p);
                info[x].address = 0;
                info[x].size    = 0;
            }
        Buddy_PrintMap();
        }
    }
    printf("\n\nSTOP\n");
    while(1) {} // STOP
}
