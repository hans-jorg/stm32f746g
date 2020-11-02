
/**
 * @file     system_stm32l476.c
 * @brief    utilities code according CMSIS
 * @version  V1.0
 * @date     03/10/2020
 *
 * @note     Includes standard SystemInit
 * @note     Includes non standard SystemCoreClockSet
 * @note
 * @note     Calls SystemInit
 * @note     Calls _main (It provides one, but it is automatically redefined)
 * @note     Calls main
 * @note     This code must be adapted for processor and compiler
 *
 ******************************************************************************/

#include "stm32f746xx.h"
#include "system_stm32f746.h"

/**
 * @brief   SystemCoreClock
 * @note    Global variable holding System Clock Frequency (HCLK)
 * @note    It is part of CMSIS
 */
uint32_t SystemCoreClock = HSI_FREQ;


//////////////// Clock Management /////////////////////////////////////////////

/**
 * @brief   Flag to indicate that the Main PLL was configured
 */
static uint32_t MainPLLConfigured = 0;


/**
 * @brief   AHB prescaler table
 * @note    It is a power of 2 in range 1 to 512 but different to 32
 */
static const uint32_t hpre_table[] = {
    1,1,1,1,1,1,1,1,                /* 0xxx: No division */
    2,4,8,16,64,128,256,512         /* 1000-1111: division by */
};


/**
 * @brief   APB prescaler table
 * @note    It is a power of 2 in range 1 to 16
 */
static const uint32_t ppre_table[] = {
    1,1,1,1,                        /* 0xxx: No division */
    2,4,8,16                        /* 1000-1111: division by */
};


/**
 * @brief   Clock Configuration for 200 MHz
 * @note    It is a power of 2 in range 1 to 16
 */
static PLL_Configuration ClockConfiguration200MHz = {
    .source = CLOCKSRC_HSE,     /* Clock source = HSE */
    .M = HSE_FREQ/1000000,      /* f_IN = 1 MHz   */
    .N = 400,                   /* f_PLL = 400 MHz*/
    .P = 2,                     /* f_OUT = 200 MHz*/
    .Q = 2,                     /* Not used */
    .R = 2                      /* Not used */
};


/**
 * @brief   Tables relating Flash Wait States to Clock Frequency and Supply Voltage
 *
 * @note    Is used the info on Table 5 of Section 3.3.2 of RM
 */
///@{
typedef struct {
        uint32_t    vmin;          /* minimum voltage in mV */
        uint32_t    freqmax[11];   /* maximal frequency in MHz for the number of WS */
} FlashWaitStates_Type;

FlashWaitStates_Type const flashwaitstates_tab[] = {
    /*  minimum                 Maximum frequency for Wait states                   */
    /*  voltage      0    1     2     3      4     5     6     7     8     9        */
    {   2700,     { 30,   60,   90,  120,  150,  180,  210,  216,    0,    0,   0}  },
    {   2400,     { 24,   48,   72,   96,  120,  144,  168,  192,  216,    0,   0}  },
    {   2100,     { 22,   44,   66,   88,  110,  132,  154,  176,  198,  216,   0}  },
    {   1800,     { 20,   40,   60,   80,  100,  120,  140,  160,  180,    0,   0}  },
    {      0,     {  0,    0,    0,    0,    0,    0,    0,    0,    0,    0,   0}  }
};

/// Used when increasing clock frequency
#define MAXWAITSTATES 9
///@}

/**
 * @brief iabs: find absolute value of an integer
 */
static inline int iabs(int k) { return k<0?-k:k; }


/**
 * @brief HSE Clock Enable/Disable
 *
 * @note    Do not disable it, if it drives the core
 **/
///@{
static inline void EnableHSE(void) {
#ifdef HSE_EXTERNAL_OSCILLATOR
    RCC->CR |= RCC_CR_HSEON|RCC_CR_HSEBYP;
#else
    RCC->CR |= RCC_CR_HSEON;
#endif
    while( (RCC->CR&RCC_CR_HSERDY) == 0 ) {}
}

static inline void DisableHSE(void) {
    RCC->CR &= ~(RCC_CR_HSEON|RCC_CR_HSEBYP);
}
///@}

/**
 * @brief HSI Clock Enable/Disable
 *
 * @note    Do not disable it, if it drives the core
 **/
///@{
static inline void EnableHSI(void) {
    RCC->CR |= RCC_CR_HSION;
    while( (RCC->CR&RCC_CR_HSIRDY) == 0 ) {}
}

static inline void DisableHSI(void) {
    RCC->CR &= ~(RCC_CR_HSION);
}
///@}

/**
 * @brief   Main PLL Disable
 *
 * @note    Do not disable it, if it drives the core
 **/
///@{
static inline void EnableMainPLL(void) {

    RCC->CR |= RCC_CR_PLLON;

    // Wait until it stabilizes
    while( (RCC->CR&RCC_CR_PLLRDY)!=RCC_CR_PLLRDY ) {}
}
static inline void DisableMainPLL(void) {

    RCC->CR &= ~RCC_CR_PLLON;

}
///@}


/**
 * @brief LSE Clock Enable/Disable
 **/
///@{
static inline void EnableLSE(void) {
#ifdef LSE_EXTERNAL_OSCILLATOR
    RCC->BDCR |= RCC_BDCR_LSEON|RCC_BDCR_LSEBYP;
#else
    RCC->BDCR |= RCC_BDCR_LSEON;
#endif
    while( (RCC->CR&RCC_BDCR_LSERDY) == 0 ) {}
}

static inline void DisableLSE(void) {
    RCC->BDCR &= ~(RCC_BDCR_LSEON|RCC_BDCR_LSEBYP);
}
///@}


/**
 * @brief   UnlockFlashRegisters
 **/
static inline void UnlockFlashRegisters(void) {
    FLASH->KEYR = 0x45670123;
    FLASH->KEYR = 0xCDEF89AB;
}

/**
 * @brief   LockFlashRegisters
 **/
static inline void LockFlashRegisters(void) {
    FLASH->CR |= FLASH_CR_LOCK;
}

/**
 * @brief   SetFlashWaitStates
 *
 * @note    Set FLASH to have n wait states
 **/

static void inline SetFlashWaitStates(int n) {

    FLASH->ACR = (FLASH->CR&~FLASH_ACR_LATENCY)|((n)<<FLASH_ACR_LATENCY_Pos);

}


/**
 * @brief   Find number of Wait States according
 *
 * @note    Given Core Clock Frequency and Voltage, find the number of Wait States
 *          needed for correct access to flash memory
 **/
static int
FindFlashWaitStates(uint32_t freq, uint32_t voltage) {
int i,j;

    /* Look for a line with tension not greater than voltage parameter */
    for(i=0;flashwaitstates_tab[i].vmin && voltage<flashwaitstates_tab[i].vmin;i++) {}
    if( flashwaitstates_tab[i].vmin == 0 )
        return -1;

    for(j=0;flashwaitstates_tab[i].freqmax[j]&&freq>flashwaitstates_tab[i].freqmax[j];j++) {}
    if( flashwaitstates_tab[i].freqmax[j] == 0 )
        return -1;

    return j;
}


/**
 * @brief   Configure Flash Wait State according core frequency and voltage
 *
 **/
static void inline ConfigureFlashWaitStates(uint32_t freq, uint32_t voltage) {
uint32_t ws;

    ws = FindFlashWaitStates(freq,voltage);

    if( ws < 0 )
        return;

    SetFlashWaitStates(ws);

}

/**
 * @brief   Get APB1 Prescaler
 *
 * @note    This prescaler divides the HCLK to generate the APB1 clock, the high speed
 *          peripheral clock
 *
 * @note    It must be set so the APB1 frequency is not greater than 54 MHz.
 *
 */

uint32_t SystemGetAPB1Prescaler(uint32_t div) {
    return ppre_table[(RCC->CFGR&~RCC_CFGR_PPRE1_Msk)>>RCC_CFGR_PPRE1_Pos];
}


/**
 * @brief   SetAPB1 Prescaler
 *
 * @note    This prescaler divides the HCLK to generate the APB1, that is
 *          the slow speed peripheral bus
 *
 * @note    It must be set so the APB1 frequency is not greater than 54 MHz.
 *
 */

void SystemSetAPB1Prescaler(uint32_t div) {
uint32_t ppre1;
uint32_t p2;


    if( SystemCoreClock/div > 54000000 )
        return;
        
    p2 = SystemFindLargestPower2Exp(div);

    if (p2 == 0)
        ppre1 = 0;
    else {
        ppre1 = 4+p2-1;
    }

    RCC->CFGR =  (RCC->CFGR&~RCC_CFGR_PPRE1_Msk)|(ppre1)<<RCC_CFGR_PPRE1_Pos;

}

/**
 * @brief   Get APB2 Prescaler
 *
 * @note    This prescaler divides the HCLK to generate the APB1 clock, the high speed
 *          peripheral clock
 *
 * @note    It must be set so the APB1 frequency is not greater than 108 MHz.
 *
 */

uint32_t SystemGetAPB2Prescaler(uint32_t div) {
    return ppre_table[(RCC->CFGR&~RCC_CFGR_PPRE2_Msk)>>RCC_CFGR_PPRE2_Pos];
}


/**
 * @brief   SetAPB2 Prescaler
 *
 * @note    This prescaler divides the HCLK to generate the APB1, the high speed
 *          peripheral clock
 *
 * @note    It must be set so the APB1 frequency is not greater than 108 MHz.
 *
 */

void SystemSetAPB2Prescaler(uint32_t div) {
uint32_t ppre2;
uint32_t p2;

    if( SystemCoreClock/div > 54000000 )
        return;
        
    p2 = SystemFindLargestPower2Exp(div);

    if (p2 == 0)
        ppre2 = 0;
    else {
        ppre2 = 4+p2-1;
    }

    RCC->CFGR =  (RCC->CFGR&~RCC_CFGR_PPRE2_Msk)|(ppre2)<<RCC_CFGR_PPRE2_Pos;

}
///@}

/**
 * @brief   CalculateMainPLLOutFrequency
 *
 * @note    BASE_FREQ = HSE_FREQ or HSI_FREQ or MSI_FREQ
 *          PLL_VCO = (BASE_FREQ / PLL_M) * PLL_N
 *          SYSCLK = PLL_VCO / PLL_R
 */
static uint32_t
CalculateMainPLLOutFrequency(PLL_Configuration *pllconfig) {
uint32_t outfreq,infreq;
uint32_t clocksource;

    clocksource = pllconfig->source;
    
    if( clocksource == CLOCKSRC_HSI) {
        infreq = HSI_FREQ;
    } else if ( clocksource == CLOCKSRC_HSE ){
        infreq = HSE_FREQ;
    } else {
        return 0;
    }
    outfreq  = (infreq*pllconfig->N)/pllconfig->M/pllconfig->P;  // Overflow possible ?
    return outfreq;
}

/**
 * @brief   CalculatePLLI2SOutFrequency
 *
 * @note    BASE_FREQ = HSE_FREQ or HSI_FREQ or MSI_FREQ
 *          PLL_VCO = (BASE_FREQ / PLL_M) * PLL_N
 *          OUTP = PLL_VCO / PLL_P
 *          OUTQ = PLL_VCO / PLL_Q
 *          OUTR = PLL_VCO / PLL_R
 *
 *          PLLI2SQ = PLLSAIQ = PLLQ = OUTQ
 *          PLLI2SR = PLLSAIR = OUTR
 *          MAINOUT = PLLCLK = PLLSAIP = OUTP
 */
static uint32_t
CalculatePLLOutFrequencies(PLL_Configuration *pllconfig) {
uint32_t outfreq,infreq;
uint32_t clocksource;

    clocksource = pllconfig->source;

    if( clocksource == CLOCKSRC_HSI) {
        infreq = HSI_FREQ;
    } else if ( clocksource == CLOCKSRC_HSE ){
        infreq = HSE_FREQ;
    } else {
        return 0;
    }
    // Overflow possible ?
    if( pllconfig->P )
        pllconfig->poutfreq  = (infreq*pllconfig->N)/pllconfig->M/pllconfig->P;
    if( pllconfig->Q )
        pllconfig->qoutfreq  = (infreq*pllconfig->N)/pllconfig->M/pllconfig->Q;
    if( pllconfig->R )
        pllconfig->routfreq  = (infreq*pllconfig->N)/pllconfig->M/pllconfig->R;

    return pllconfig->poutfreq;
}

/**
 * @brief   SystemSystemClockGet
 *
 * @note    returns the SYSCLK, i.e., the Core Clock before the prescaler
 */

uint32_t SystemGetSYSCLKFrequency(void) {
uint32_t rcc_cr, rcc_cfgr, rcc_pllcfgr;
uint32_t src;
uint32_t sysclk_freq;
uint32_t base_freq;
uint32_t pllsrc;
PLL_Configuration pllconfig;

    rcc_cr = RCC->CR;
    rcc_cfgr = RCC->CFGR;
    rcc_pllcfgr = RCC->PLLCFGR;
    sysclk_freq = 0;
    
    /* Get source */
    src = rcc_cfgr & RCC_CFGR_SWS;
    switch (src) {
    case RCC_CFGR_SWS_HSI:  /* HSI used as system clock source */
        sysclk_freq = HSI_FREQ;
        break;
    case RCC_CFGR_SWS_HSE:  /* HSE used as system clock source */
        sysclk_freq = HSE_FREQ;
        break;
    case RCC_CFGR_SW_PLL:  /* PLL used as system clock source */

        pllsrc = (rcc_pllcfgr & RCC_PLLCFGR_PLLSRC);
        if ( (pllsrc & RCC_PLLCFGR_PLLSRC_HSI) == RCC_PLLCFGR_PLLSRC_HSI )
            pllsrc = CLOCKSRC_HSI;
        else
            pllsrc = CLOCKSRC_HSE;

        pllconfig.source = pllsrc;
        pllconfig.M = (rcc_pllcfgr & RCC_PLLCFGR_PLLM)>>RCC_PLLCFGR_PLLM_Pos;
        pllconfig.N = (rcc_pllcfgr & RCC_PLLCFGR_PLLN)>>RCC_PLLCFGR_PLLN_Pos;
        pllconfig.P = (rcc_pllcfgr & RCC_PLLCFGR_PLLP)>>RCC_PLLCFGR_PLLP_Pos;
        sysclk_freq = CalculateMainPLLOutFrequency(&pllconfig);
      break;
    }

    return sysclk_freq;
}

/**
 * @brief   SystemGetCoreClock
 *
 * @note    Returns the System Core Clock based on information contained in the
 *          Clock Register Values (RCC)
 */

uint32_t
SystemGetCoreClock(void) {
uint32_t sysclk_freq, prescaler, hpre;

    sysclk_freq = SystemGetSYSCLKFrequency();
    
    /* Get HCLK prescaler */
    hpre = (RCC->CFGR&RCC_CFGR_HPRE_Msk)>>RCC_CFGR_HPRE_Pos;
    prescaler = hpre_table[hpre];

    /* HCLK frequency */
    return sysclk_freq/prescaler;
}

/**
 * @brief   SystemGetAPB1Frequency
 *
 * @note    Returns the APB1 (low speed peripheral) clock frequency 
 */
uint32_t SystemGetAPB1Frequency(void) {
uint32_t freq;
uint32_t ppre1;

    freq = SystemGetCoreClock();
    ppre1 = (RCC->CFGR&RCC_CFGR_PPRE1_Msk)>>RCC_CFGR_PPRE1_Pos;
    return freq/ppre_table[ppre1];
}

/**
 * @brief   SystemGetAPB1Frequency
 *
 * @note    Returns the System Core Clock based on information contained in the
 *          Clock Register Values (RCC)
 */

uint32_t SystemGetAPB2Frequency(void) {
uint32_t freq;
uint32_t ppre2;

    freq = SystemGetCoreClock();
    ppre2 = (RCC->CFGR&RCC_CFGR_PPRE2_Msk)>>RCC_CFGR_PPRE2_Pos;
    return freq/ppre_table[ppre2];
}

/**
 * @brief   SystemGetAHBFrequency
 *
 * @note    It is the same as SystemCoreClock and HCLK
 */

uint32_t SystemGetAHBFrequency(void) {

    return SystemGetCoreClock();

}

/**
 * @brief   SystemGetHCLKFrequency
 *
 * @note    It is the same as SystemCoreClock and HCLK
 */

uint32_t SystemGetHCLKFrequency(void) {

    return SystemGetCoreClock();

}

/**
 * @brief   FindHPRE
 *
 * @note    Given a divisor, find the best HPRE returning its encoding
 *
 * @note    Could use the hpre_table table above, as the alternative below.
 */
static uint32_t FindHPRE(uint32_t divisor) {
uint32_t k;

    k = SystemFindLargestPower2(divisor);   // 2 exponent of divisor
#if 1
    if( k == 0 )
        return 0;
    if( k < 5 ) {
        return 0x8+k-1;
    } else { // There is no divisor 32. It is changed to 64
        return 0x8+k-2;
    }
#else
    for(int i=0;i<sizeof(hpre_table)/sizeof(uint32_t);i++) {
        if( hpre_table[i]>=divisor)
            return i;
    }
#endif
}

/**
 * @brief   SystemConfigMainPLL
 *
 * @note    Configure Main PLL unit
 *
 * @note    If core clock source (HCLK) is PLL, it is changed to HSI
 *
 * @note    It does not switch the core clock source (HCLK) to PLL
 */

void
SystemConfigMainPLL(PLL_Configuration *pllconfig) {
uint32_t freq,src;
uint32_t rcc_pllcfgr;
uint32_t clocksource;

    clocksource = pllconfig->source;

    // If core clock source is PLL change it to HSI and disable PLL
    if( RCC->CFGR&RCC_CFGR_SWS == RCC_CFGR_SWS_PLL ) {
        EnableHSI();
        RCC->CFGR = (RCC->CFGR&RCC_CFGR_SW)|RCC_CFGR_SW_HSI;
        DisableMainPLL();
    }

    // Configure it
    switch(clocksource) {
    case CLOCKSRC_HSI:
        freq = HSI_FREQ;
        src  = RCC_CFGR_SW_HSI;
        break;
    case CLOCKSRC_HSE:
        freq = HSE_FREQ;
        src  = RCC_CFGR_SW_HSE;
        break;
    default:
        return;
    }
    // Get PLLCFGR and clear fields to be set
    rcc_pllcfgr = RCC->PLLCFGR
             &  ~(  RCC_PLLCFGR_PLLQ
                   |RCC_PLLCFGR_PLLSRC
                   |RCC_PLLCFGR_PLLP
                   |RCC_PLLCFGR_PLLN
                   |RCC_PLLCFGR_PLLM
                 );

    rcc_pllcfgr |= (pllconfig->P<<RCC_PLLCFGR_PLLP_Pos)
                  |(pllconfig->N<<RCC_PLLCFGR_PLLN_Pos)
                  |(pllconfig->M<<RCC_PLLCFGR_PLLM_Pos)
                  |(pllconfig->Q<<RCC_PLLCFGR_PLLQ_Pos)
                  |src;

    RCC->PLLCFGR = rcc_pllcfgr;

    EnableMainPLL();

    MainPLLConfigured = 1;
}
///@}


/**
 * @brief   SystemSetCoreClock
 *
 * @note    Configure to use clock source. If not enabled, enable it and wait for
 *          stabilization
 *
 * @note    If the PLL clock is not configure, it is configured to generate a
 *          200 MHz clock signal
 *
 * @note    To increase the clock frequency (Section 3.3.2 of RM)
 *          1. Program the new number of wait states to the LATENCY bits
 *             in the FLASH_ACR register
 *          2. Check that the new number of wait states is taken into account
 *             to access the Flash memory by reading the FLASH_ACR register
 *          3. Modify the CPU clock source by writing the SW bits in the RCC_CFGR
 *             register
 *          4  If needed, modify the CPU clock prescaler by writing the HPRE bits
 *             in RCC_CFGR
 *          5. Check that the new CPU clock source or/and the new CPU clock prescaler
 *             value is/are taken into account by reading the clock source status
 *             (SWS bits) or/and the AHB prescaler value (HPRE bits), respectively,
 *             in the RCC_CFGR register.
 *
 * @note   To decrease the clock frequency (Section 3.3.2 of RM)
 *         1. Modify the CPU clock source by writing the SW bits in
 *            the RCC_CFGR register
 *         2. If needed, modify the CPU clock prescaler by writing the HPRE
 *            bits in RCC_CFGR
 *         3. Check that the new CPU clock source or/and the new CPU clock
 *            prescaler value is/are taken into account by reading the
 *            clock source status (SWS bits) or/and the AHB prescaler value
 *            (HPRE bits), respectively, in the RCC_CFGR register
 *         4. Program the new number of wait states to the LATENCY bits in FLASH_ACR
 *         5. Check that the new number of wait states is used to access
 *            the Flash memory by reading the FLASH_ACR register
 *
 */
uint32_t SystemSetCoreClock(uint32_t newsrc, uint32_t newdiv) {
uint32_t src,div;
uint32_t hpre,newhpre;

    src = RCC->CFGR & RCC_CFGR_SW;

    if( newsrc == src ) { // Just change the prescaler
        hpre = (RCC->CFGR&RCC_CFGR_HPRE_Msk)>>RCC_CFGR_HPRE_Pos;
        div = hpre_table[hpre];
        newhpre = FindHPRE(newdiv);
        if( newdiv < div ) {                    // Increasing clock frequency
            SetFlashWaitStates(MAXWAITSTATES);  // Worst case
            SystemSetAPB1Prescaler(4);
            SystemSetAPB2Prescaler(2);
        }
        RCC->CFGR = (RCC->CFGR&~RCC_CFGR_HPRE)|(newhpre<<RCC_CFGR_HPRE_Pos);
    } else {
        // There is a change of clock source

        SetFlashWaitStates(MAXWAITSTATES);  // Worst case
        switch(newsrc) {
        case CLOCKSRC_HSI:
            EnableHSI();
            RCC->CFGR = (RCC->CFGR&~RCC_CFGR_SW)|RCC_CFGR_SW_HSI;
            break;
        case CLOCKSRC_HSE:
            EnableHSE();
            RCC->CFGR = (RCC->CFGR&~RCC_CFGR_SW)|RCC_CFGR_SW_HSE;
            break;
        case CLOCKSRC_PLL:
            if( !MainPLLConfigured ) {
                SystemSetAPB1Prescaler(4);
                SystemSetAPB2Prescaler(2);
                SystemConfigMainPLL(&ClockConfiguration200MHz);
            }
            RCC->CFGR = (RCC->CFGR&~RCC_CFGR_SW)|RCC_CFGR_SW_PLL;
        }
    }

    // Set SystemCoreClock to the new frequency and adjust flash wait states
    SystemCoreClockUpdate();
    ConfigureFlashWaitStates(SystemCoreClock,VSUPPLY);
    return 0;
 }


/**
 * @brief   SystemSetCoreClockFrequency
 *
 * @note    Configure to use PLL as a clock source and to run at the given
 *          frequency.
  */
uint32_t SystemSetCoreClockFrequency(uint32_t freq) {
PLL_Configuration clockconf;

    if( freq >= HCLKMAX ) {
        freq = HCLKMAX;
    }
    clockconf.source = CLOCKSRC_HSE;     /* Clock source   */
    clockconf.M = HSE_FREQ/1000000;      /* f_IN = 1 MHz   */
    clockconf.N = 2*freq;                /* f_PLL = 400 MHz*/
    clockconf.P = 2;                     /* f_OUT = 200 MHz*/
    clockconf.Q = 2;                     /* Not used */
    clockconf.R = 2;                     /* Not used */

    SystemConfigMainPLL(&clockconf);
    SystemSetCoreClock(CLOCKSRC_PLL,1);
    return freq;
}

///////////////////////////Auxiliary functions ////////////////////////////////

/**
 * @brief   FindNearestPower2Divisor
 *
 * @note    Given a number, find a power of 2 nearest to it
 */
uint32_t SystemFindNearestPower2(uint32_t divisor) {
int n = 1;
int err = 10000000;

    for(int i=0;i<20;i++) {
        int k = 1<<i;
        int e = iabs((int)divisor-(int)k);
        if( e >= err)
            break;
        err = e;
        n = k;
    }
    return n;
}

uint32_t SystemFindNearestPower2Exp(uint32_t divisor) {
int n = 1;
int err = 10000000;
int i;

    for(i=0;i<20;i++) {
        int k = 1<<i;
        int e = iabs((int)divisor-(int)k);
        if( e >= err){
            i--;
            break;
        }
        err = e;
        n = k;
    }
    return i;
}

uint32_t SystemFindLargestPower2(uint32_t divisor) {
int n = 1;
int err = 10000000;
int i;

    for(i=0;i<20;i++) {
        int k = 1<<i;
        int e = iabs((int)divisor-(int)k);
        if( e >= err) {
            i--;
            break;
        }
        err = e;
        n = k;
    }
    if( n < divisor )
        n<<=1;
    return n;
}

uint32_t SystemFindLargestPower2Exp(uint32_t divisor) {
int n = 1;
int err = 10000000;
int i;

    for(i=0;i<20;i++) {
        int k = 1<<i;
        int e = iabs((int)divisor-(int)k);
        if( e >= err) {
            i--;
            break;
        }
        err = e;
        n = k;
    }
    if( n < divisor )
        i++;
    return i;
}


//////////////// CMSIS  ///////////////////////////////////////////////////////

/**
 * @brief SystemCoreClockUpdate
 *
 * @note Updates the SystemCoreClock variable using information contained in the
 *       Clock Register Values (RCC)
 *
 * @note This function must be called to update SystemCoreClock variable every time
 *       the clock configuration is modified.
 *
 * @note It is part of CMSIS
 */

void
SystemCoreClockUpdate(void) {

    SystemCoreClock = SystemGetCoreClock();

}


/**
 * @brief SystemInit
 *
 * @note  Resets to default configuration for clock and disables all interrupts
 *
 * @note  It is part of CMSIS
 *
 * @note  Replaces the one (dummy) contained in start_DEVICE.c
 */

void
SystemInit(void) {

    /* Configure FPU */
    #if (__FPU_PRESENT == 1) && (__FPU_USED == 1)
        /* enable CP10 and CP11 coprocessors */
        SCB->CPACR |= (0x0FUL << 20);
        __DSB();
        __ISB();
    #endif

    /* Reset HSEON, CSSON and PLLON bits */
    RCC->CR = 0x00000083;

    /* Reset CFGR register */
    RCC->CFGR = 0x00000000;

    /* Reset PLLCFGR register */
    RCC->PLLCFGR = 0x24003010;

    /* Disable all interrupts */
    RCC->CIR = 0x00000000;

    /* Enable HSE but do not switch to it */
    EnableHSE();

    /* Update SystemCoreClock */
    SystemCoreClockUpdate();

    /* Enable cache for Instruction and Data . Only for AXIM interface */
    SCB_EnableICache();
    SCB_EnableDCache();

    /* Enable ART (ST technology). Only for TCM interface  */
    FLASH->ACR &= ~FLASH_ACR_ARTEN;         /* Disable ART */
    FLASH->ACR |= FLASH_ACR_ARTRST;         /* Reset ART */
    //FLASH->ACR &= ~FLASH_ACR_ARTRST;      /* Reset ART */

    FLASH->ACR |= FLASH_ACR_ARTEN;          /* Enable ART */
    FLASH->ACR |= FLASH_ACR_PRFTEN;         /* Enable ART Prefetch*/

    /* It is possible to relocate Vector Table Must be a 512 byte boundary. Bits 8:0 = 0 */
    //SCB->VTOR = FLASH_BASE;                 /* Vector Table Relocation in Internal FLASH */


    /* Additional initialization here */

}



