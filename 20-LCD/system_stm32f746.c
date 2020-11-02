
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

    SystemCoreClock = SystemCoreClockGet();

}

/**
 * @brief   AHB prescaler table
 * @note    It is a power of 2 in range 1 to 512 but different to 32
 */
static uint32_t hpre_table[] = {
    1,1,1,1,1,1,1,1,                /* 0xxx: No division */
    2,4,8,16,64,128,256,512         /* 1000-1111: division by */
};



/**
 * @brief   Maximal clock frequencies
 *
 * @note    APB1 (Low Speed Prescaler)  : 54 MHz
 * @note    APB2 (High Speed Prescaler) : 108 MHz

 */
///@{
#define MAXAPB1FREQ      54000000
#define MAXAPB2FREQ     108000000
#define MAXAHB1FREQ     216000000
///@}

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

FlashWaitStates_Type flashwaitstates_tab[] = {
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
 */
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
 */
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
 * @brief   Main PLL Enable/Disable
 *
 * @note    Do not disable it, if it drives the core
 */
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
 * @brief   PLLSAI Enable/Disable
 *
 */
///@{
static inline void EnablePLLSAI(void) {

    RCC->CR |= RCC_CR_PLLSAION;

    // Wait until it stabilizes
    while( (RCC->CR&RCC_CR_PLLSAIRDY)!=RCC_CR_PLLSAIRDY ) {}
}

static inline void DisablePLLSAI(void) {

    RCC->CR &= ~RCC_CR_PLLSAION;

}
///@}

/**
 * @brief   PLLI2S Enable/Disable
 *
 */
///@{
static inline void EnablePLLI2S(void) {

    RCC->CR |= RCC_CR_PLLI2SON;

    // Wait until it stabilizes
    while( (RCC->CR&RCC_CR_PLLI2SRDY)!=RCC_CR_PLLI2SRDY ) {}
}

static inline void DisablePLLI2S(void) {

    RCC->CR &= ~RCC_CR_PLLI2SON;

}
///@}



/**
 * @brief LSE Clock Enable/Disable
 *
 */
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
 */
static inline void UnlockFlashRegisters(void) {
    FLASH->KEYR = 0x45670123;
    FLASH->KEYR = 0xCDEF89AB;
}

/**
 * @brief   LockFlashRegisters
 */
static inline void LockFlashRegisters(void) {
    FLASH->CR |= FLASH_CR_LOCK;
}

/**
 * @brief   SetFlashWaitStates
 *
 * @note    Set FLASH to have n wait states
 */

static void inline SetFlashWaitStates(int n) {

    FLASH->ACR = (FLASH->CR&~FLASH_ACR_LATENCY)|((n)<<FLASH_ACR_LATENCY_Pos);

}


/**
 * @brief   Find number of Wait States according
 *
 * @note    Given Core Clock Frequency and Voltage, find the number of Wait States
 *          needed for correct access to flash memory
 */
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
 */
static void inline ConfigureFlashWaitStates(uint32_t freq, uint32_t voltage) {
uint32_t ws;

    ws = FindFlashWaitStates(freq,voltage);

    if( ws < 0 )
        return;

    SetFlashWaitStates(ws);

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
 * @brief   SetPrescalers for AHB, APB1 and APB2
 *
 * @note    This prescaler divides the SYSCLK to generate the HCLK, i.e, core clock frequency
 *
 * @note    APB1 is the low speed prescaler. It must be set so the APB1 frequency is not greater
 *          than 54 MHz.
 *
 * @note    APB2 is the high speed prescaler. It must be set so the APB2 frequency is not greater
 *          than 108 MHz.
 */
 ///@{

void
SystemSetAHB1Prescaler(uint32_t newdiv) {
uint32_t hpre, div;
uint32_t newhpre;


        hpre = (RCC->CFGR&RCC_CFGR_HPRE_Msk)>>RCC_CFGR_HPRE_Pos;
        div = hpre_table[hpre];
        newhpre = FindHPRE(newdiv);
        if( newdiv < div ) {                    // Increasing clock frequency
            SetFlashWaitStates(MAXWAITSTATES);  // Worst case
        }
        RCC->CFGR = (RCC->CFGR&~RCC_CFGR_HPRE)|(newhpre<<RCC_CFGR_HPRE_Pos);

}
 

void
SystemSetAPB1Prescaler(uint32_t div) {
uint32_t ppre1;
uint32_t p2;

    p2 = SystemFindLargestPower2Exp(div);

    if (p2 == 0)
        ppre1 = 0;
    else {
        ppre1 = 4+p2-1;
    }

    if( SystemCoreClock/div > 54000000 )
        return;

    RCC->CFGR =  (RCC->CFGR&~RCC_CFGR_PPRE1)|(ppre1)<<RCC_CFGR_PPRE1_Pos;

}

void
SystemSetAPB2Prescaler(uint32_t div) {
uint32_t ppre2;
uint32_t p2;

    p2 = SystemFindLargestPower2Exp(div);

    if (p2 == 0)
        ppre2 = 0;
    else {
        ppre2 = 4+p2-1;
    }

    if( SystemCoreClock/div > 54000000 )
        return;

    RCC->CFGR =  (RCC->CFGR&~RCC_CFGR_PPRE2)|(ppre2)<<RCC_CFGR_PPRE2_Pos;

}
///@}


/**
 * @brief   SystemSetPeripheralClocks
 *
 * @note    Set Peripheral Prescalers so the APBx maximal clock frequencies are not
 *          exceeded.
 */
static void SetPeripheralClocks(uint32_t div1, uint32_t div2) {
uint32_t div1min,div2min;

    div1min = (SystemCoreClock+MAXAPB1FREQ-1)/MAXAPB1FREQ;
    if( div1 < div1min ) div1 = div1min;

    div2min = (SystemCoreClock+MAXAPB2FREQ-1)/MAXAPB2FREQ;
    if( div2 < div2min ) div2 = div2min;
    
    SystemSetAPB1Prescaler(div1);
    SystemSetAPB2Prescaler(div2);
}

/**
 * @brief   CalculateMainPLLOutFrequency
 *
 * @note    BASE_FREQ = HSE_FREQ or HSI_FREQ or MSI_FREQ
 *          PLL_VCO = (BASE_FREQ / PLL_M) * PLL_N
 *          SYSCLK = PLL_VCO / PLL_R
 */
static uint32_t
CalculateMainPLLOutFrequency(uint32_t clocksource, PLL_Configuration *pllconfig) {
uint32_t outfreq,infreq;

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
CalculatePLLOutFrequencies(uint32_t clocksource, PLL_Configuration *pllconfig) {
uint32_t outfreq,infreq;

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

static uint32_t
GetSYSCLKFreq(void) {
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
            
        pllconfig.M = (rcc_pllcfgr & RCC_PLLCFGR_PLLM)>>RCC_PLLCFGR_PLLM_Pos;
        pllconfig.N = (rcc_pllcfgr & RCC_PLLCFGR_PLLN)>>RCC_PLLCFGR_PLLN_Pos;
        pllconfig.P = (rcc_pllcfgr & RCC_PLLCFGR_PLLP)>>RCC_PLLCFGR_PLLP_Pos;
        sysclk_freq = CalculateMainPLLOutFrequency(pllsrc,&pllconfig);
      break;
    }

    return sysclk_freq;
}



  
/**
 * @brief   SystemCoreClockGet
 *
 * @note    Returns the System Core Clock based on information contained in the
 *          Clock Register Values (RCC)
 */

uint32_t
SystemCoreClockGet(void) {
uint32_t sysclk_freq, prescaler, hpre;

    sysclk_freq = GetSYSCLKFreq();
    
    /* Get HCLK prescaler */
    hpre = (RCC->CFGR&RCC_CFGR_HPRE_Msk)>>RCC_CFGR_HPRE_Pos;
    prescaler = hpre_table[hpre];

    /* HCLK frequency */
    return sysclk_freq/prescaler;
}

/**
 * @brief   In any PLL already configured?
 *
 * @note    Indicates if any PLL unit has been configured
 *
 * @note    It is used to avoid the reconfiguration of common parts
 *          like clock source and M divider
 *
 */
///@{
static uint32_t MainPLLConfigured = 0;
static uint32_t PLLSAIConfigured = 0;
static uint32_t PLLI2SConfigured = 0;

static inline int IsAnyPLLConfigured(void) {

    return RCC->CR&(RCC_CR_PLLON|RCC_CR_PLLSAION|RCC_CR_PLLI2SON)
        | (MainPLLConfigured + PLLSAIConfigured + PLLI2SConfigured);
}
///@}
/**
 * @brief   Find P divisor encoding
 *
 * @note    Only values 2,4,6,8 are valid and encoded as 0,1,2,3 respesctively

 * @note    Invalid values are rounded to next large one
 *
 */

///@{
//                           div:  0, 1, 2, 3, 4, 5, 6, 7, 8
static uint32_t p_encoding[] = {   0 ,0, 0, 1, 1, 2, 2, 3, 3 };

static inline uint32_t
FindPDivEncoding(uint32_t div) {

    if( div <= 8 ) return p_encoding[div];
    else return 3;

#if 0
uint32_t newp;
    if( pllconfig->P <= 2 ) newp = 0x0;
    else if (pllconfig->P > 2 && pllconfig <= 4 ) newp = 0x1;
    else if (pllconfig->P > 4 && pllconfig <= 6 ) newp = 0x2;
    else if newp = 0x3;
    return newp;
#endif
}
///@}

/**
 * @brief   CheckPLLConfiguration
 *
 */
int
CheckPLLConfiguration(PLL_Configuration *pllconfig) {

    if( pllconfig->M < 2 ) pllconfig->M = 2;
    if( pllconfig->M > 63 ) pllconfig->M = 63;

    if( pllconfig->N < 50 ) pllconfig->N = 50;
    if( pllconfig->N > 432 ) pllconfig->N = 432;

    if( pllconfig->P < 2 ) pllconfig->P = 2;
    else if (pllconfig->P == 3 ) pllconfig->P = 4;
    else if ( pllconfig->P > 4 ) pllconfig->P = 8;

    if( pllconfig->P <= 2 ) pllconfig->P = 2;
    else if (pllconfig->P > 2 && pllconfig->P <= 4 ) pllconfig->P = 4;
    else if (pllconfig->P > 4 && pllconfig->P <= 6 ) pllconfig->P = 6;
    else pllconfig->P = 8;

    if( pllconfig->Q < 2 ) pllconfig->Q = 2;
    if( pllconfig->Q > 15 ) pllconfig->Q = 15;

    if( pllconfig->R < 2 ) pllconfig->R = 2;
    if( pllconfig->R > 7 ) pllconfig->R = 7;

}


/**
 * @brief   GetPLLConfiguration
 *
 */
uint32_t SystemGetPLLConfiguration(uint32_t whichone,PLL_Configuration *pllconfig) {
uint32_t rc = 0;

    // Source and M registers are shared by all PLL units
    pllconfig->M = (RCC->PLLCFGR&RCC_PLLCFGR_PLLM)>>RCC_PLLCFGR_PLLM_Pos;
    if( (RCC->PLLCFGR&RCC_PLLCFGR_PLLSRC) == RCC_PLLCFGR_PLLSRC_HSI )
        pllconfig->source = HSI_FREQ;
    else
        pllconfig->source = HSE_FREQ;

    switch(whichone) {
    case PLL_MAIN:
        pllconfig->N = (RCC->PLLCFGR&RCC_PLLCFGR_PLLN)>>RCC_PLLCFGR_PLLN_Pos;
        pllconfig->P = 2*(RCC->PLLCFGR&RCC_PLLCFGR_PLLQ)>>RCC_PLLCFGR_PLLP_Pos+2;
        pllconfig->Q = (RCC->PLLCFGR&RCC_PLLCFGR_PLLQ)>>RCC_PLLCFGR_PLLQ_Pos;
        pllconfig->R = 1;
        rc = RCC->CR&RCC_CR_PLLON;
        break;
    case PLL_SAI:
        pllconfig->N = (RCC->PLLSAICFGR&RCC_PLLSAICFGR_PLLSAIN)>>RCC_PLLSAICFGR_PLLSAIN_Pos;
        pllconfig->P = 2*(RCC->PLLSAICFGR&RCC_PLLSAICFGR_PLLSAIP)>>RCC_PLLSAICFGR_PLLSAIP_Pos+2;
        pllconfig->Q = (RCC->PLLSAICFGR&RCC_PLLSAICFGR_PLLSAIQ)>>RCC_PLLSAICFGR_PLLSAIQ_Pos;
        pllconfig->R = (RCC->PLLSAICFGR&RCC_PLLSAICFGR_PLLSAIR)>>RCC_PLLSAICFGR_PLLSAIR_Pos;
        rc = RCC->CR&RCC_CR_PLLSAION;
        break;
    case PLL_I2S:
        pllconfig->N = (RCC->PLLSAICFGR&RCC_PLLI2SCFGR_PLLI2SN)>>RCC_PLLI2SCFGR_PLLI2SN_Pos;
        pllconfig->P = 2*(RCC->PLLI2SCFGR&RCC_PLLI2SCFGR_PLLI2SP)>>RCC_PLLI2SCFGR_PLLI2SP_Pos+2;
        pllconfig->Q = (RCC->PLLI2SCFGR&RCC_PLLI2SCFGR_PLLI2SQ)>>RCC_PLLI2SCFGR_PLLI2SQ_Pos;
        pllconfig->R = (RCC->PLLI2SCFGR&RCC_PLLI2SCFGR_PLLI2SR)>>RCC_PLLI2SCFGR_PLLI2SR_Pos;
        rc = RCC->CR&RCC_CR_PLLI2SON;
        break;
    }
    return rc;
}
/**
 * @brief   SystemMainPLLConfig
 *
 * @note    Configure Main PLL unit
 *
 * @note    If core clock source (HCLK) is PLL, it is changed to HSI
 *          and restored at the end
 *
 * @note    It does not switch the core clock source (HCLK) to PLL
 */

///@{
void
SystemMainPLLConfig(PLL_Configuration *pllconfig) {
uint32_t rcc_pllcfgr,freq;
uint32_t returntopll = 0;
uint32_t newp = 0;

    // Filter bad parameters
    CheckPLLConfiguration(pllconfig);

    // If core clock source is PLL change it to HSI and disable PLL
    if( RCC->CFGR&RCC_CFGR_SWS == RCC_CFGR_SWS_PLL ) {
        EnableHSI();
        RCC->CFGR = (RCC->CFGR&RCC_CFGR_SW)|RCC_CFGR_SW_HSI;
        DisableMainPLL();
        returntopll = 1;
    }
    
    // Clear all fields except
    rcc_pllcfgr = RCC->PLLCFGR
             &  ~(  RCC_PLLCFGR_PLLQ
                   |RCC_PLLCFGR_PLLP
                   |RCC_PLLCFGR_PLLN
                 );


    // Only configure source and M, it is not previously configured
    if( IsAnyPLLConfigured() ) {
        // If already configure, update M in pllconfig
        pllconfig->M = (RCC->PLLCFGR&RCC_PLLCFGR_PLLM)>>RCC_PLLCFGR_PLLM_Pos;
        // Get frequency from registers
        if( (RCC->PLLCFGR&RCC_PLLCFGR_PLLSRC) == RCC_PLLCFGR_PLLSRC_HSI )
            freq = HSI_FREQ;
        else
            freq = HSE_FREQ;

    } else {
        //  Clear fields for source and M
        rcc_pllcfgr &= ~(RCC_PLLCFGR_PLLM|RCC_PLLCFGR_PLLSRC);
        // Configure them

        rcc_pllcfgr  |= pllconfig->source;
        rcc_pllcfgr |= pllconfig->M<<RCC_PLLCFGR_PLLM_Pos;
        if( pllconfig->source ==  RCC_PLLCFGR_PLLSRC_HSI)
            freq = HSI_FREQ;
        else
            freq = HSE_FREQ;
    }

    // New configuration
    newp = FindPDivEncoding(pllconfig->P);
    rcc_pllcfgr |= (newp<<RCC_PLLCFGR_PLLP_Pos)
                  |(pllconfig->N<<RCC_PLLCFGR_PLLN_Pos)
                  |(pllconfig->Q<<RCC_PLLCFGR_PLLQ_Pos);

    // Calculate frequencies
    pllconfig->infreq = freq;
    pllconfig->pllinfreq = freq/pllconfig->M;
    pllconfig->vcofreq = (freq*pllconfig->N)/pllconfig->M;
    pllconfig->poutfreq = pllconfig->vcofreq/pllconfig->P;
    pllconfig->qoutfreq = pllconfig->vcofreq/pllconfig->Q;
    pllconfig->routfreq = 0;

    // Set new configuration
    RCC->PLLCFGR = rcc_pllcfgr;

    EnableMainPLL();

    if( returntopll ) {
        RCC->CFGR = (RCC->CFGR&~RCC_CFGR_SW)|RCC_CFGR_SW_PLL;
    }
    MainPLLConfigured = 1;
}
//@}


/**
 * @brief   SystemPLLSAIConfig
 *
 * @note    Configure PLL SAI unit
 *
 * @note    It shares the M divider and clock source with Main PLL
 *
 * @note    Main before PLLSAI and PLLI2S ??????
 */

///@{
void
SystemPLLSAIConfig(PLL_Configuration *pllconfig) {
uint32_t freq,src;
uint32_t rcc_pllcfgr;
uint32_t rcc_pllsaicfgr;
uint32_t newp;

    // Filter bad parameters
    CheckPLLConfiguration(pllconfig);

    // Disable PLLSAI
    DisablePLLSAI();

    // Clear all fields of PLLSAICFGR
    rcc_pllsaicfgr = RCC->PLLSAICFGR
             &  ~(  RCC_PLLSAICFGR_PLLSAIQ
                   |RCC_PLLSAICFGR_PLLSAIP
                   |RCC_PLLSAICFGR_PLLSAIN
                   |RCC_PLLSAICFGR_PLLSAIR
                 );

    // Get PLLCFGR in case it is neccessary to configurate clock source and M
    rcc_pllcfgr = RCC->PLLCFGR;

    // Only configure source and M, it is not previously configured
    if( IsAnyPLLConfigured() ) {
        // If already configure, update M in pllconfig
        pllconfig->M = (RCC->PLLCFGR&RCC_PLLCFGR_PLLM)>>RCC_PLLCFGR_PLLM_Pos;
        // Get frequency from registers
        if( (RCC->PLLCFGR&RCC_PLLCFGR_PLLSRC) == RCC_PLLCFGR_PLLSRC_HSI )
            freq = HSI_FREQ;
        else
            freq = HSE_FREQ;
    } else {
        //  Clear fields for source and M
        rcc_pllcfgr &= ~(RCC_PLLCFGR_PLLM|RCC_PLLCFGR_PLLSRC);
        // Configure them
        rcc_pllcfgr |= pllconfig->source;
        rcc_pllcfgr |= pllconfig->M<<RCC_PLLCFGR_PLLM_Pos;
        if( pllconfig->source == RCC_CFGR_SW_HSI )
            freq = HSI_FREQ;
        else
            freq = HSE_FREQ;
    }

    // New configuration
    newp = FindPDivEncoding(pllconfig->P);
    rcc_pllsaicfgr |= (newp<<RCC_PLLSAICFGR_PLLSAIP_Pos)
                  |(pllconfig->N<<RCC_PLLSAICFGR_PLLSAIN_Pos)
                  |(pllconfig->Q<<RCC_PLLSAICFGR_PLLSAIQ_Pos)
                  |(pllconfig->R<<RCC_PLLSAICFGR_PLLSAIR_Pos);

    // Calculate frequencies
    pllconfig->infreq = freq;
    pllconfig->pllinfreq = freq/pllconfig->M;
    pllconfig->vcofreq = (freq*pllconfig->N)/pllconfig->M;
    pllconfig->poutfreq = pllconfig->vcofreq/pllconfig->P;
    pllconfig->qoutfreq = pllconfig->vcofreq/pllconfig->Q;
    pllconfig->routfreq = pllconfig->vcofreq/pllconfig->R;

    // Set new configuration
    RCC->PLLCFGR    = rcc_pllcfgr;
    RCC->PLLSAICFGR    = rcc_pllsaicfgr;
    
    EnablePLLSAI();

    PLLSAIConfigured = 1;
}


/**
 * @brief   SystemPLLI2SConfig
 *
 * @note    Configure PLL SAI unit
 *
 * @note    It shares the M divider and clock source with Main PLL
 *
 * @note    Main before PLLSAI and PLLI2S ????????
 */

///@{
void
SystemPLLI2SConfig(PLL_Configuration *pllconfig) {
uint32_t freq,src;
uint32_t rcc_pllcfgr;
uint32_t rcc_plli2scfgr;
uint32_t newp;

    // Filter bad parameters
    CheckPLLConfiguration(pllconfig);

    // Disable PLLI2S
    DisablePLLI2S();

    // Clear all fields of PLLI2SCFGR
    rcc_plli2scfgr = RCC->PLLI2SCFGR
             &  ~(  RCC_PLLI2SCFGR_PLLI2SQ
                   |RCC_PLLI2SCFGR_PLLI2SP
                   |RCC_PLLI2SCFGR_PLLI2SN
                   |RCC_PLLI2SCFGR_PLLI2SR
                 );

    // Get PLLCFGR in case it is neccessary to configurate clock source and M
    rcc_pllcfgr = RCC->PLLCFGR;

    // Only configure source and M, it is not previously configured
    if( IsAnyPLLConfigured() ) {
        // If already configure, update M in pllconfig
        pllconfig->M = (RCC->PLLCFGR&RCC_PLLCFGR_PLLM)>>RCC_PLLCFGR_PLLM_Pos;
        // Get frequency from registers
        if( (RCC->PLLCFGR&RCC_PLLCFGR_PLLSRC) == RCC_PLLCFGR_PLLSRC_HSI )
            freq = HSI_FREQ;
        else
            freq = HSE_FREQ;
    } else {
        //  Clear fields for source and M
        rcc_pllcfgr &= ~(RCC_PLLCFGR_PLLM|RCC_PLLCFGR_PLLSRC);
        // Configure them
        rcc_pllcfgr |= pllconfig->source;
        rcc_pllcfgr |= pllconfig->M<<RCC_PLLCFGR_PLLM_Pos;
        if( pllconfig->source == CLOCKSRC_HSI )
            freq = HSI_FREQ;
        else
            freq = HSE_FREQ;
    }

    // New configuration
    newp = FindPDivEncoding(pllconfig->P);
    rcc_plli2scfgr |= (newp<<RCC_PLLI2SCFGR_PLLI2SP_Pos)
                  |(pllconfig->N<<RCC_PLLI2SCFGR_PLLI2SN_Pos)
                  |(pllconfig->Q<<RCC_PLLI2SCFGR_PLLI2SQ_Pos)
                  |(pllconfig->R<<RCC_PLLI2SCFGR_PLLI2SR_Pos);

    // Calculate frequencies
    pllconfig->infreq = freq;
    pllconfig->pllinfreq = freq/pllconfig->M;
    pllconfig->vcofreq = (freq*pllconfig->N)/pllconfig->M;
    pllconfig->poutfreq = pllconfig->vcofreq/pllconfig->P;
    pllconfig->qoutfreq = pllconfig->vcofreq/pllconfig->Q;
    pllconfig->routfreq = pllconfig->vcofreq/pllconfig->R;

    // Set new configuration
    RCC->PLLCFGR    = rcc_pllcfgr;
    RCC->PLLI2SCFGR = rcc_plli2scfgr;
    
    EnablePLLI2S();

    PLLI2SConfigured = 1;
}

/**
 * @brief   SystemCoreClockSet
 *
 * @note    Configure to use clock source. If not enabled, enable it and wait for
 *          stabilization
 *
 * @note    PLL must be previously configured by calling SystemMainPLLConfig
 *
 * @note    To increase the clock frequency (Section 3.3.2 of RM)
 *          1. Program the new number of wait states to the LATENCY bits in the FLASH_ACR register
 *          2. Check that the new number of wait states is taken into account to access the
 *             Flash memory by reading the FLASH_ACR register
 *          3. Modify the CPU clock source by writing the SW bits in the RCC_CFGR register
 *          4  If needed, modify the CPU clock prescaler by writing the HPRE bits in RCC_CFGR
 *          5. Check that the new CPU clock source or/and the new CPU clock prescaler value
 *              is/are taken into account by reading the clock source status (SWS bits)
 *              or/and the AHB prescaler value (HPRE bits), respectively, in the RCC_CFGR register.
 *
 * @note   To decrease the clock frequency (Section 3.3.2 of RM)
 *         1. Modify the CPU clock source by writing the SW bits in the RCC_CFGR register
 *         2. If needed, modify the CPU clock prescaler by writing the HPRE bits in RCC_CFGR
 *         3. Check that the new CPU clock source or/and the new CPU clock prescaler value is/are
 *            taken into account by reading the clock source status (SWS bits) or/and the AHB
 *            prescaler value (HPRE bits), respectively, in the RCC_CFGR register
 *         4. Program the new number of wait states to the LATENCY bits in FLASH_ACR
 *         5. Check that the new number of wait states is used to access the Flash memory by
 *            reading the FLASH_ACR register
 *
 */
uint32_t SystemCoreClockSet(uint32_t newsrc, uint32_t newdiv) {
uint32_t src,div;
uint32_t hpre,newhpre;

    src = RCC->CFGR & RCC_CFGR_SW;

    SetPeripheralClocks(2,2); // Worst case values

    if( newsrc == src ) { // Just change the prescaler
        SystemSetAHB1Prescaler(newdiv);
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
            if( MainPLLConfigured ) {
                RCC->CFGR = (RCC->CFGR&~RCC_CFGR_SW)|RCC_CFGR_SW_PLL;
            }
        }
    }

    // Set SystemCoreClock to the new frequency and adjust flash wait states
    SystemCoreClockUpdate();
    ConfigureFlashWaitStates(SystemCoreClock,VSUPPLY);

    // Adjust peripheral clocks
    SetPeripheralClocks(0,0);

    
    return 0;
 }


///////////////////////////Auxiliary functions ///////////////////////////////////////////////////

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

///////////////////////////SystemInit//////////////////////////////////////////////////////////////

/**
 * @brief SystemInit
 *
 * @note Resets to default configuration for clock and disables all interrupts
 *
 * @note Replaces the one (dummy) contained in start_DEVICE.c
 *
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




