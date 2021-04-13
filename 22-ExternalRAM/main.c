
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

void Delay(uint32_t delay) {

    delay_ms = delay;
    while( delay_ms ) {}

}


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

/*
 * @brief   Configuration for PLLSAI
 *
 * @note    Assumes PLL Main will use HSE (crystal) and have a 1 MHz input for PLL
 *
 * @note    LCD_CLK should be in range 5-12, with typical value 9 MHz.
 *
 * @note    There is an extra divisor in PLLSAIDIVR[1:0] of RCC_DCKCFGR, that can
 *          have value 2, 4, 8 or 16.
 *
 * @note    So the R output must be 18, 36, 72 or 144 MHz.
 *          But USB, RNG and SDMMC needs 48 MHz. The LCM of 48 and 9 is 144.
 *
 *          f_LCDCLK  = 9 MHz        PLLSAIRDIV=8
 *
 */

PLL_Configuration  pllsaiconfig  = {
    .source         = RCC_PLLCFGR_PLLSRC_HSI,
    .M              = HSE_FREQ/1000,                        // f_IN = 1 MHz
    .N              = 144,                                  // f_VCO = 144 MHz
    .P              = 3,                                    // f_P = 48 MHz
    .Q              = 3,                                    // f_Q = 48 MHz
    .R              = 2                                     // f_R = 72 MHz
};


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
 * @brief   main
 *
 * @note    Initializes GPIO and blinks LED
 */

int main(void) {

    /* Set Clock to 200 MHz */
    SystemConfigMainPLL(&Clock200MHz);
    SystemSetCoreClock(CLOCKSRC_PLL,1);


    SysTick_Config(SystemCoreClock/1000);

    printf("Starting....\n");

    /* WTF */
    //RCC->DCKCFGR1 = (RCC->DCKCFGR1&~RCC_DCKCFGR1_PLLSAIDIVR)|(8<<RCC_DCKCFGR1_PLLSAIDIVR_Pos);

    //SystemConfigSAIPLL(&pllsaiconfig);


    /*
     * Blink LED
     */
    for (;;) {
        putchar('+');
    }
}
