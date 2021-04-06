#ifndef SYSTEM_STM32F746_H
#define SYSTEM_STM32F746_H

/**
 * @file     system_stm32f746.h
 * @brief    utilities code according CMSIS
 * @version  V1.0
 * @date     03/10/2020
 *
 * @note     Provides CMSIS standard SystemInit and SystemCoreClockUpdate
 * @note     Provides non standard SystemCoreClockGet and
 *           SystemCoreClockSet among others
 * @note     Define symbols for Clock Sources
 * @note     This code must be adapted for processor and compiler
 *
 * @note     System Core Clock (SYSCLK) is named HCLK and AHB CLock
 *           and is derived from SYSCLK thru AHB prescaler
 *
 * @author   Hans
 *
 ******************************************************************************/

/**
 * @brief   SystemCoreClock global variable
 * @note    It must be update to contain the system core clock frequency
 * @note    CMSIS standard variable
 */
extern uint32_t SystemCoreClock;

/**
 * @brief   SystemCoreClockUpdate
 * @note    It updates SystemCoreClock variable
 * @note    Must be called every time the system core clock frequency is changed
 * @note    CMSIS standard function
 */
void SystemCoreClockUpdate(void);

/**
 * @brief   SystemCoreClockGet
 * @note    Returns the System Core Clock Frequency directly from RCC registers
 * @note    It is not a CMSIS function
 */
uint32_t SystemCoreClockGet(void);

 /**
 * @brief   SystemInit
 * @note    Performs system initialization
 * @note    It can override the SystemInit in startup_stm32f746.c file
 * @note    CMSIS standard function
 */
void SystemInit(void);


//------8<-------8<-------8<-------8<-------8<-------8<-------8<-------8<-------8<-------8<-------
/**
 * @brief   BSP Section
 *
 * @note    Maybe a better idea is to put this in a bsp.h file
 */

/**
 * @brief   Core Supply Voltage
 *
 * @note    Must be in mV
 *
 */
#define VSUPPLY 3300
/**
 * @brief   Clock configuration
 *
 * @note    Uncomment the use of crystal or external oscillator
 *
 * @note    The discovery board use an oscillator for HSE and a crystal for LSE
 */
//{

//#define HSE_CRYSTAL_FREQ   25000000L
#define HSE_OSCILLATOR_FREQ  25000000L

#define LSE_CRYSTAL_FREQ     32768L
//#define LSE_OSCILLATOR_FREQ  32768L
//}
/**
 * @briefg  HSE External crystal/oscillator frequency
 * @note    If not defined, use default. In this case, the oscillator frequency on
 *          Discovery board
 * @note    HSE_FREQ can be overriden by a compiler parameter (e,g, -DHSE_FREQ=20000000 )
 */

#ifndef HSE_FREQ
    #ifdef HSE_OSCILLATOR_FREQ
        #define HSE_FREQ  HSE_OSCILLATOR_FREQ
        #define HSE_EXTERNAL_OSCILLATOR
    #else
        #define HSE_FREQ  HSE_CRYSTAL_FREQ
    #endif
#endif

/**
 * @brief   LSE: External Low Crystal/Oscillator frequency
 * @note    It must be 32768 Hz
 */


#ifndef LSE_FREQ
    #ifdef  LSE_OSCILLATOR_FREQ
        #define LSE_FREQ  LSE_OSCILLATOR_FREQ
        #define LSE_EXTERNAL_OSCILLATOR
    #else
        #define LSE_FREQ  LSE_CRYSTAL_FREQ
    #endif
#endif
//}


/**
 * @brief  Maximal system core frequency (HCLK_max)
 */
 #define HCLKMAX 216000000

/**
 * @brief Internal Clock Source Frequencies for STM32F746 MCU
 */
//{
#define HSI_FREQ            16000000UL          /* Internal RC low precision (1%) */
#define LSI_FREQ               32000UL          /* Internal RCC low precision [17..47 KHz]) */
//}



//------8<-------8<-------8<-------8<-------8<-------8<-------8<-------8<-------8<-------8<-------

/**
 * @brief Clock Management
 */

/**
 * @brief Clocks sources for System Clock SYSCLK
 */
//{
#define CLOCKSRC_HSI        RCC_CFGR_SWS_HSI
#define CLOCKSRC_HSE        RCC_CFGR_SWS_HSE
#define CLOCKSRC_PLL        RCC_CFGR_SWS_PLL
//}

/**
 * @brief PLL parameters
 */

typedef struct {
    uint32_t    source;
    uint32_t    M;
    uint32_t    N;
    uint32_t    P;
    uint32_t    Q;              /* for other PLL units */
    uint32_t    R;
    /* filled by CalculatePLLOutFrequencies */
    uint32_t    infreq;         // = SYSFREQ
    uint32_t    pllinfreq;      // = SYSFREQ/M
    uint32_t    vcofreq;        // = PLLINFREQ*N
    uint32_t    poutfreq;       // = VCOFREQ/P
    uint32_t    qoutfreq;       // = VCOFREQ/Q
    uint32_t    routfreq;       // = VCOFREQ/R
} PLL_Configuration;

/**
 * @note    Additional functions
 */
// Get routines
uint32_t SystemGetCoreClock(void);
uint32_t SystemGetSYSCLKFrequency(void);
uint32_t SystemGetAPB1Frequency(void);
uint32_t SystemGetAPB2Frequency(void);
uint32_t SystemGetAHBFrequency(void);
uint32_t SystemGetHCLKFrequency(void);
uint32_t SystemGetCoreClock(void);
uint32_t SystemGetAPB1Prescaler(void);
uint32_t SystemGetAPB2Prescaler(void);
uint32_t SystemGetSYSCLKFrequency(void);


// Set routines
uint32_t SystemSetCoreClock(uint32_t newsrc, uint32_t newdiv);
uint32_t SystemSetCoreClockFrequency(uint32_t freq);
void     SystemSetAPB1Prescaler(uint32_t div);
void     SystemSetAPB2Prescaler(uint32_t div);

// Auxiliary routines

uint32_t SystemFindNearestPower2(uint32_t divisor);
uint32_t SystemFindNearestPower2Exp(uint32_t divisor);
uint32_t SystemFindLargestPower2(uint32_t divisor);
uint32_t SystemFindLargestPower2Exp(uint32_t divisor);

// Additional PLL routines
#define PLL_MAIN (0)
#define PLL_SAI  (1)
#define PLL_I2S  (2)

void SystemConfigMainPLL(PLL_Configuration *pllconfig);
void SystemConfigSAIPLL(PLL_Configuration *pllconfig);
void SystemConfigI2SPLL(PLL_Configuration *pllconfig);
uint32_t SystemGetPLLConfiguration(uint32_t whichone, PLL_Configuration *pllconfig);


#endif
