
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

#include "stm32f746xx.h"
#include "system_stm32f746.h"
#include "sdram.h"
#include "led.h"




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
 * @brief Probe an address to see if can be read without generating a bus fault
 * @details This function must be called with the processor in privileged mode.
 *          It:
 *          - Clear any previous indication of a bus fault in the BFARV bit
 *          - Temporarily sets the processor to Ignore Bus Faults with all interrupts and fault handlers disabled
 *          - Attempt to read from read_address, ignoring the result
 *          - Checks to see if the read caused a bus fault, by checking the BFARV bit is set
 *          - Re-enables Bus Faults and all interrupts and fault handlers
 * @param[in] read_address The address to try reading a byte from
 * @return Returns true if no bus fault occurred reading from read_address, or false if a bus fault occurred.
 */


/*
 * From
 * System control block (SCB)
 * The System Control Block (SCB) provides system implementation information,
 * and system control. This includes configuration, control, and reporting
 * of the system exceptions. The system control block registers are:
 * ...
 * 0xE000ED14 CCR
 * ...
 * 0xE000ED28 CFSR (Word access)
 * 0xE000ED29 BFSR (Byte access)
 * ...
 *
 * Configurable fault status register (CFSR)
 * The CFSR indicates the cause of a MemManage fault, BusFault, or UsageFault.
 * It has three fields: UFSR, BFSR, MMFSR
 *
 * Flag BFARVALID in BFSR
 * * BusFault Address register (BFAR) valid flag:
 * 0: Value in BFAR is not a valid fault address.
 * 1: BFAR holds a valid fault address.
 * The processor sets this bit to 1 after a BusFault where the address is known.
 *  Other faults can set this bit to 0, such as a MemManage fault occurring
 * later. If a BusFault occurs and is escalated to a hard fault because of
 *  priority, the hard fault handler must set this bit to 0. This prevents
 *  problems if returning to a stacked active BusFault handler whose BFAR value
 *  has been overwritten.

 * Configuration and control register (CCR)
 * The CCR controls entry to Thread mode and enables:
 *
 * BFHFNMIGN Flag
 * Enables handlers with priority -1 or -2 to ignore data BusFaults
 * caused by load and store instructions. This applies to the hard
 * fault, NMI, and FAULTMASK escalated handlers:
 * 0:Data bus faults caused by load and store instructions
 * cause a lock-up.
 * 1: Handlers running at priority -1 and -2 ignore data bus
 * faults caused by load and store instructions.
 * Set this bit to 1 only when the handler and its data are in
 * absolutely safe memory. The normal use of this bit is to probe
 * system devices and bridges to detect control path problems
 * and fix them
 */
#define NVIC_FAULT_STAT             (*(uint32_t) 0xE000ED28)
// Bus Fault Address Register Valid
#define NVIC_FAULT_STAT_BFARV       0x00008000

#define NVIC_CFG_CTRL               (*(uint32_t) 0xE000ED14)
#define  NVIC_CFG_CTRL_BFHFNMIGN    0x00000100


int read_probe (volatile const char *address) {
int status;
volatile char ch;

    /*
     * Clear an existing indication of a bus fault. Just in case.
     * Writing 1 to BFARVALID bit in CFSR clears it
     */
    SCB->CFSR |= SCB_CFSR_BFARVALID_Msk;

    /*
     * Ignores BusFaults by load and store instructions
     * by setting BFHFNMIGN to 1
     */
    SCB->CCR |= SCB_CCR_BFHFNMIGN_Msk;

    status = 1;
    __disable_fault_irq(); /* CPSID f */

    ch = *address;  /* <=== Memory access */
    if( (SCB->CFSR&SCB_CFSR_BFARVALID_Msk) != 0 ) {
        status = 0;
    }
    __enable_fault_irq(); /* CPSIE f */

    /* Reenable BusFaults */
    SCB->CCR &= ~SCB_CCR_BFHFNMIGN_Msk;

    return status;
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

/**
 * @brief   main
 *
 * @note    Initializes GPIO and SDRAM, blinks LED and test SDRAM access
 */
#define LINEMAX 100
int main(void) {
char line[LINEMAX+1];

    printf("Starting.at %ld KHz...\n",SystemCoreClock/1000);

    /* Set Clock to 200 MHz */
    SystemConfigMainPLL(&MainPLLConfiguration_200MHz);
    SystemSetCoreClock(CLOCKSRC_PLL,1);

    printf("Now running at %ld KHz...\n",SystemCoreClock/1000);

    SysTick_Config(SystemCoreClock/1000);

    printf("Press ENTER to initialize ExtRAM\n");
    fgets(line,LINEMAX,stdin);
    SDRAM_Init();

    uint16_t w = 0x1234;
    uint16_t wr;
    uint16_t *p = (uint16_t *) 0xC0000000;
    uint32_t lw = 0x12345678;
    uint32_t lwr;
    uint32_t *lp = (uint32_t *) 0xC0000000;
    while(1) {
        puts("Choose test");
        puts("1 - Write pattern using 16 bit access");
        puts("2 - Write random pattern using 16-bit access");
        puts("3 - Write random pattern using 16-bit access");
        puts("4 - Write pattern using 32 bit access");
        puts("5 - Write random pattern using 32-bit access");
        puts("6 - Write random pattern using 32-bit access");
        puts("7 - Reset apontadores");
        fputs(">",stdout);
        fgets(line,100,stdin);
        int test = atoi(line);
        if( test == 0 ) continue;
        int k;

        switch(test) {
        case 1:
            for (k=0;k<16;k++) {
                printf("Write %04X to %p. ",w,p);
                *p = w;
                __DSB();
                wr = *p;
                Delay(10);
                printf("Read %04X =>  %s\n",wr,(w==wr)?"OK":"Error");
                w++;
                p++;
            }
            break;
        case 2:
            for(k=0;k<16;k++) {
                w = my_rand();
                printf("Wrote %04X to %p  ",w,p);
                *p = w;
                __DSB();
                Delay(10);
                wr = *p;
                if ( w == wr )
                    printf("OK\n",wr);
                else
                    printf("Read %04X\n",wr);
                p++;
            }
            break;
        case 3:
            for(k=0;k<16;k++) {
                printf("%p\r",p);
                w = my_rand();
                *p = w;
                __DSB();
                Delay(10);
                wr = *p;
                if( w != wr )
                    printf("\nWrote %04X Read %04X\n",w,wr);
                p++;
            }
            break;
        case 4:
            for (k=0;k<16;k++) {
                printf("Write %08X to %p\n",lw,lp);
                *lp = lw;
                __DSB();
                Delay(10);
                lwr = *lp;
                printf("Read %04X =>  %s\n",lwr,(lw==lwr)?"OK":"Error");
                lw++;
                lp++;
            }
            break;
        case 5:
            for(k=0;k<16;k++) {
                lw = my_rand();
                printf("Wrote %08X to %p  ",lw,lp);
                *lp = lw;
                __DSB();
                Delay(10);
                lwr = *lp;
                if ( lw == lwr )
                    printf("OK\n",lwr);
                else
                    printf("Read %08\n",lwr);
                lp++;
            }
            break;
        case 6:
            for(k=0;k<16;k++) {
                printf("%p\r",lp);
                lw = my_rand();
                *lp = lw;
                 __DSB();
                Delay(10);
                lwr = *lp;
                if( lw != lwr )
                    printf("\nWrote %08X Read %08X\n",lw,lwr);
                lp++;
            }
            break;
        case 7:
            w = 0x1234;
            p = (uint16_t *) 0xC0000000;
            lw = 0x12345678;
            lp = (uint32_t *) 0xC0000000;
        }
    }
}
