
/**
 * @file    sdram.c
 *
 * @note    SDRAM_Init configures FMC and SDRAM to be accessed in the memory range
 *          0xC000_0000-0xC07F_FFFF (8 MBytes)
 *
 *
 * @date    07/10/2020
 * @author  Hans
 */

#include "stm32f746xx.h"
#include "system_stm32f746.h"
#include "gpio.h"
#include "sdram.h"

/**
 *  @brief  Pin initialization routines
 *
 *  @note   There are two versions:
 *          1: using direct access to register (faster but larger)
 *          2: using GPIO routines from a pin configuration table (smaller but slow))
 */
#ifndef SDRAM_FAST_INITIALIZATION
#define SDRAM_FAST_INITIALIZATION (1)
#endif


/**
 *  @brief  SDRAMBIT    generates bit mask
 */
#define SDRAMBIT(N) (1U<<(N))

/**
 * @brief   General configuration
 *
 * @note    All parameters are set for a SDRAM clock frequency of 100 MHz
 *
 * @note    The SD_CLOCK is derived from the HCLK (Core Clock).
 *
 *
 * @brief   Parameters for the MT48LC4M32B2B5
 *
 * @note    COUNT = SDRAM_Refresh_period/NROWS - 20
 *
 * @note    Refresh rate = (COUNT+1)xfreq_SDRAMCLK
 *
 * @note    Refresh rate = 64 ms/4096 = 15.625 us
 *                   This times 100 MHz = 1562
 *                   Subtract 20 as a safe margin = 1542
 *
 * @note    COUNT=(freq/64 ms)/ 4096 = 1562
 *                   Subtract 20 as a safe margin = 1542
 *
 *
 */

/*
 * OBS: There are some differences between STM32F746G Discovery Demo and
 * the code in BSP/stm32g_discovery_sdram.c
 *
 * TRAS = 6      7
 * TRC  = 6      7
 *
 */


// Configuration of SDCRx
#define SDRAM_RPIPE             0
#define SDRAM_RBURST            1
#define SDRAM_SDCLK             2
#define SDRAM_WP                0
#define SDRAM_CAS               0
#define SDRAM_NB                1
#define SDRAM_MWID              1
#define SDRAM_NR                1
#define SDRAM_NC                0

// Configuration of SDTRx
#define SDRAM_TRCD              2
#define SDRAM_TRP               2
#define SDRAM_TWR               2
#define SDRAM_TRC               6
#define SDRAM_TRAS              4
#define SDRAM_TXSR              6
#define SDRAM_TMRD              2


#define SDRAM_REFRESHCOUNT      1539

/**
 *  @brief  FMC Commands
 */
///@{
#define SDRAM_COMMAND_NORMAL                0x0
#define SDRAM_COMMAND_CLOCKCONFIGENABLE     0x1
#define SDRAM_COMMAND_PALL                  0x2
#define SDRAM_COMMAND_AUTOREFRESH           0x3
#define SDRAM_COMMAND_LOADMODE              0x4
#define SDRAM_COMMAND_SELF_REFRESH          0x5
#define SDRAM_COMMAND_POWERDOWN             0x6
///@}

/**
 *  @brief  SDRAM Autorefresh
 *
 *  @note   8 auto-refresh cycles every time AUTOREFRESH command is issued
 */
#define SDRAM_AUTOREFRESH                   0x7

/*
 *  @brief  Refresh count
 *
 *  @note   All rows must be refreshed every 64 ms. This can be done distributed along this time
 *          or as a burst with an interval of 60 ns.
 *
 *  @note   Refresh count depends on the SD_CLK signal
 *
 *          64 ms/4096 rows = 15.625 us
 *          15.625 us *100 MHz = 1562
 *          Subtract a safety margin (20)
 *          Counter = 1542
 *          Refresh rate = (1582+1)*100 MHz
 *
 *          OBS: 20 or 20% ??

 *  @note   It must be different from TWR+TRP+TRC+TRCD+4 memory cycles
 *
 *  @note   It must be greater than 40
 *
 */
#define SDRAM_REFRESH                       1542

/**
 *  @brief  Default timeout
 *
 *  @note   Number of tries until operation completed
 */
#define DEFAULT_TIMEOUT                     0xFFFF

/**
 *  @brief  Mode register for MT48LC4M32B2
 *
 * Configuration used
 * Burst length     =  000 (1)
 * Burst type       =    0 (Sequential)
 * CAS Latency      =  010 (2)
 * Operation mode   =   00 (Standard Operation)
 * Write Burst Mode =    1 (Single Location Access)
 *
 */

#define SDRAM_MODE   0x220


/*******************^^^^^^ To be rewamped ^^^^^^ *************************************************/



/**
 * @brief   Pin initialization
 *
 * @note    Uncomment the define USE_FAST_INITIALIZATION  to initialize without GPIO routines and
 *          tables
 *
 * @note    In initializes FMC for 12-bit column address and 16-bit data bus
 *
 * @note    Pins must be configured as follows
 *
 *          | Parameter         |   Value   | Description              |
 *          |-------------------|-----------|--------------------------|
 *          | AF                |    12     | Alternate function FMC   |
 *          | Mode              |     2     | Alternate function       |
 *          | OType             |     0     | Push pull                |
 *          | OSpeed            |     3     | Very High Speed          |
 *          | Pull-up/Push down |     0     | No push-up nor pull down |
 */


#if SDRAM_FAST_INITIALIZATION == 1


#define SD_AF      (12)
#define SD_MODE    (2)
#define SD_OTYPE   (0)
#define SD_OSPEED  (3)
#define SD_PUPD    (0)


static void
ConfigureFMCSDRAMPins(int bank) {
uint32_t mAND,mOR; // Mask

    // Configure pins in GPIOD
    // 0/DQ2 1/DQ3 8/DQ13 9/DQ14 10/DQ15 14/DQ0 15/DQ1

    RCC->AHB1ENR |= RCC_AHB1ENR_GPIODEN;

    mAND =   GPIO_AFRL_AFRL0_Msk
            |GPIO_AFRL_AFRL1_Msk;
    mOR  =   (SD_AF<<GPIO_AFRL_AFRL0_Pos)
            |(SD_AF<<GPIO_AFRL_AFRL1_Pos);
    GPIOD->AFR[0]  = (GPIOD->AFR[0]&~mAND)|mOR;

    mAND =   GPIO_AFRH_AFRH0_Msk
            |GPIO_AFRH_AFRH1_Msk
            |GPIO_AFRH_AFRH2_Msk
            |GPIO_AFRH_AFRH6_Msk
            |GPIO_AFRH_AFRH7_Msk;
    mOR  =   (SD_AF<<GPIO_AFRH_AFRH0_Pos)
            |(SD_AF<<GPIO_AFRH_AFRH1_Pos)
            |(SD_AF<<GPIO_AFRH_AFRH2_Pos)
            |(SD_AF<<GPIO_AFRH_AFRH6_Pos)
            |(SD_AF<<GPIO_AFRH_AFRH7_Pos);
    GPIOD->AFR[1]  = (GPIOD->AFR[1]&~mAND)|mOR;

    mAND =   GPIO_MODER_MODER0_Msk
            |GPIO_MODER_MODER1_Msk
            |GPIO_MODER_MODER8_Msk
            |GPIO_MODER_MODER9_Msk
            |GPIO_MODER_MODER10_Msk
            |GPIO_MODER_MODER14_Msk
            |GPIO_MODER_MODER15_Msk;
    mOR  =   (SD_MODE<<GPIO_MODER_MODER0_Pos)
            |(SD_MODE<<GPIO_MODER_MODER1_Pos)
            |(SD_MODE<<GPIO_MODER_MODER8_Pos)
            |(SD_MODE<<GPIO_MODER_MODER9_Pos)
            |(SD_MODE<<GPIO_MODER_MODER10_Pos)
            |(SD_MODE<<GPIO_MODER_MODER14_Pos)
            |(SD_MODE<<GPIO_MODER_MODER15_Pos);
    GPIOD->MODER   = (GPIOD->MODER&~mAND)|mOR;

    mAND =   GPIO_OSPEEDR_OSPEEDR0_Msk
            |GPIO_OSPEEDR_OSPEEDR1_Msk
            |GPIO_OSPEEDR_OSPEEDR8_Msk
            |GPIO_OSPEEDR_OSPEEDR9_Msk
            |GPIO_OSPEEDR_OSPEEDR10_Msk
            |GPIO_OSPEEDR_OSPEEDR14_Msk
            |GPIO_OSPEEDR_OSPEEDR15_Msk;
    mOR  =   (SD_OSPEED<<GPIO_OSPEEDR_OSPEEDR0_Pos)
            |(SD_OSPEED<<GPIO_OSPEEDR_OSPEEDR1_Pos)
            |(SD_OSPEED<<GPIO_OSPEEDR_OSPEEDR8_Pos)
            |(SD_OSPEED<<GPIO_OSPEEDR_OSPEEDR9_Pos)
            |(SD_OSPEED<<GPIO_OSPEEDR_OSPEEDR10_Pos)
            |(SD_OSPEED<<GPIO_OSPEEDR_OSPEEDR14_Pos)
            |(SD_OSPEED<<GPIO_OSPEEDR_OSPEEDR15_Pos);
    GPIOD->OSPEEDR = (GPIOD->OSPEEDR&~mAND)|mOR;

    mAND =   GPIO_PUPDR_PUPDR0_Msk
            |GPIO_PUPDR_PUPDR1_Msk
            |GPIO_PUPDR_PUPDR8_Msk
            |GPIO_PUPDR_PUPDR9_Msk
            |GPIO_PUPDR_PUPDR10_Msk
            |GPIO_PUPDR_PUPDR14_Msk
            |GPIO_PUPDR_PUPDR15_Msk;
    mOR  =   (SD_PUPD<<GPIO_PUPDR_PUPDR0_Pos)
            |(SD_PUPD<<GPIO_PUPDR_PUPDR1_Pos)
            |(SD_PUPD<<GPIO_PUPDR_PUPDR8_Pos)
            |(SD_PUPD<<GPIO_PUPDR_PUPDR9_Pos)
            |(SD_PUPD<<GPIO_PUPDR_PUPDR10_Pos)
            |(SD_PUPD<<GPIO_PUPDR_PUPDR14_Pos)
            |(SD_PUPD<<GPIO_PUPDR_PUPDR15_Pos);
    GPIOD->PUPDR   = (GPIOD->PUPDR&~mAND)|mOR;

    mAND =   GPIO_OTYPER_OT0_Msk
            |GPIO_OTYPER_OT1_Msk
            |GPIO_OTYPER_OT8_Msk
            |GPIO_OTYPER_OT9_Msk
            |GPIO_OTYPER_OT10_Msk
            |GPIO_OTYPER_OT14_Msk
            |GPIO_OTYPER_OT15_Msk;
    mOR  =   (SD_OTYPE<<GPIO_OTYPER_OT0_Pos)
            |(SD_OTYPE<<GPIO_OTYPER_OT1_Pos)
            |(SD_OTYPE<<GPIO_OTYPER_OT8_Pos)
            |(SD_OTYPE<<GPIO_OTYPER_OT9_Pos)
            |(SD_OTYPE<<GPIO_OTYPER_OT10_Pos)
            |(SD_OTYPE<<GPIO_OTYPER_OT14_Pos)
            |(SD_OTYPE<<GPIO_OTYPER_OT15_Pos);
    GPIOD->OTYPER  = (GPIOD->OTYPER&~mAND)|mOR;

    // Configure pins in GPIOE
    // 0/DQM0 1/DQM1 7/DQ4 8/DQ5 9/DQ6 10/DQ7 11/DQ8 AF/DQ9 13/DQ10 14/DQ11 15/DQAF

    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOEEN;

    mAND =   GPIO_AFRL_AFRL0_Msk
            |GPIO_AFRL_AFRL1_Msk
            |GPIO_AFRL_AFRL7_Msk;
    mOR  =   (SD_AF<<GPIO_AFRL_AFRL0_Pos)
            |(SD_AF<<GPIO_AFRL_AFRL1_Pos)
            |(SD_AF<<GPIO_AFRL_AFRL7_Pos);
    GPIOE->AFR[0]  = (GPIOE->AFR[0]&~mAND)|mOR;

    mAND =   GPIO_AFRH_AFRH0_Msk
            |GPIO_AFRH_AFRH1_Msk
            |GPIO_AFRH_AFRH2_Msk
            |GPIO_AFRH_AFRH3_Msk
            |GPIO_AFRH_AFRH4_Msk
            |GPIO_AFRH_AFRH5_Msk
            |GPIO_AFRH_AFRH6_Msk
            |GPIO_AFRH_AFRH7_Msk;
    mOR  =   (SD_AF<<GPIO_AFRH_AFRH0_Pos)
            |(SD_AF<<GPIO_AFRH_AFRH1_Pos)
            |(SD_AF<<GPIO_AFRH_AFRH2_Pos)
            |(SD_AF<<GPIO_AFRH_AFRH3_Pos)
            |(SD_AF<<GPIO_AFRH_AFRH4_Pos)
            |(SD_AF<<GPIO_AFRH_AFRH5_Pos)
            |(SD_AF<<GPIO_AFRH_AFRH6_Pos)
            |(SD_AF<<GPIO_AFRH_AFRH7_Pos);
    GPIOE->AFR[1]  = (GPIOE->AFR[1]&~mAND)|mOR;

    mAND =   GPIO_MODER_MODER0_Msk
            |GPIO_MODER_MODER1_Msk
            |GPIO_MODER_MODER7_Msk
            |GPIO_MODER_MODER8_Msk
            |GPIO_MODER_MODER9_Msk
            |GPIO_MODER_MODER10_Msk
            |GPIO_MODER_MODER11_Msk
            |GPIO_MODER_MODER12_Msk
            |GPIO_MODER_MODER13_Msk
            |GPIO_MODER_MODER14_Msk
            |GPIO_MODER_MODER15_Msk;
    mOR  =   (SD_MODE<<GPIO_MODER_MODER0_Pos)
            |(SD_MODE<<GPIO_MODER_MODER1_Pos)
            |(SD_MODE<<GPIO_MODER_MODER7_Pos)
            |(SD_MODE<<GPIO_MODER_MODER8_Pos)
            |(SD_MODE<<GPIO_MODER_MODER9_Pos)
            |(SD_MODE<<GPIO_MODER_MODER10_Pos)
            |(SD_MODE<<GPIO_MODER_MODER11_Pos)
            |(SD_MODE<<GPIO_MODER_MODER12_Pos)
            |(SD_MODE<<GPIO_MODER_MODER13_Pos)
            |(SD_MODE<<GPIO_MODER_MODER14_Pos)
            |(SD_MODE<<GPIO_MODER_MODER15_Pos);
    GPIOE->MODER   = (GPIOE->MODER&~mAND)|mOR;

    mAND =   GPIO_OSPEEDR_OSPEEDR0_Msk
            |GPIO_OSPEEDR_OSPEEDR1_Msk
            |GPIO_OSPEEDR_OSPEEDR7_Msk
            |GPIO_OSPEEDR_OSPEEDR8_Msk
            |GPIO_OSPEEDR_OSPEEDR9_Msk
            |GPIO_OSPEEDR_OSPEEDR10_Msk
            |GPIO_OSPEEDR_OSPEEDR11_Msk
            |GPIO_OSPEEDR_OSPEEDR12_Msk
            |GPIO_OSPEEDR_OSPEEDR13_Msk
            |GPIO_OSPEEDR_OSPEEDR14_Msk
            |GPIO_OSPEEDR_OSPEEDR15_Msk;
    mOR  =   (SD_OSPEED<<GPIO_OSPEEDR_OSPEEDR0_Pos)
            |(SD_OSPEED<<GPIO_OSPEEDR_OSPEEDR1_Pos)
            |(SD_OSPEED<<GPIO_OSPEEDR_OSPEEDR7_Pos)
            |(SD_OSPEED<<GPIO_OSPEEDR_OSPEEDR8_Pos)
            |(SD_OSPEED<<GPIO_OSPEEDR_OSPEEDR9_Pos)
            |(SD_OSPEED<<GPIO_OSPEEDR_OSPEEDR10_Pos)
            |(SD_OSPEED<<GPIO_OSPEEDR_OSPEEDR11_Pos)
            |(SD_OSPEED<<GPIO_OSPEEDR_OSPEEDR12_Pos)
            |(SD_OSPEED<<GPIO_OSPEEDR_OSPEEDR13_Pos)
            |(SD_OSPEED<<GPIO_OSPEEDR_OSPEEDR14_Pos)
            |(SD_OSPEED<<GPIO_OSPEEDR_OSPEEDR15_Pos);
    GPIOE->OSPEEDR = (GPIOE->OSPEEDR&~mAND)|mOR;

    mAND =   GPIO_PUPDR_PUPDR0_Msk
            |GPIO_PUPDR_PUPDR1_Msk
            |GPIO_PUPDR_PUPDR7_Msk
            |GPIO_PUPDR_PUPDR8_Msk
            |GPIO_PUPDR_PUPDR9_Msk
            |GPIO_PUPDR_PUPDR10_Msk
            |GPIO_PUPDR_PUPDR11_Msk
            |GPIO_PUPDR_PUPDR12_Msk
            |GPIO_PUPDR_PUPDR13_Msk
            |GPIO_PUPDR_PUPDR14_Msk
            |GPIO_PUPDR_PUPDR15_Msk;
    mOR  =   (SD_PUPD<<GPIO_PUPDR_PUPDR0_Pos)
            |(SD_PUPD<<GPIO_PUPDR_PUPDR1_Pos)
            |(SD_PUPD<<GPIO_PUPDR_PUPDR7_Pos)
            |(SD_PUPD<<GPIO_PUPDR_PUPDR8_Pos)
            |(SD_PUPD<<GPIO_PUPDR_PUPDR9_Pos)
            |(SD_PUPD<<GPIO_PUPDR_PUPDR10_Pos)
            |(SD_PUPD<<GPIO_PUPDR_PUPDR11_Pos)
            |(SD_PUPD<<GPIO_PUPDR_PUPDR12_Pos)
            |(SD_PUPD<<GPIO_PUPDR_PUPDR13_Pos)
            |(SD_PUPD<<GPIO_PUPDR_PUPDR14_Pos)
            |(SD_PUPD<<GPIO_PUPDR_PUPDR15_Pos);
    GPIOE->PUPDR   = (GPIOE->PUPDR&~mAND)|mOR;

    mAND =   GPIO_OTYPER_OT0_Msk
            |GPIO_OTYPER_OT1_Msk
            |GPIO_OTYPER_OT7_Msk
            |GPIO_OTYPER_OT8_Msk
            |GPIO_OTYPER_OT9_Msk
            |GPIO_OTYPER_OT10_Msk
            |GPIO_OTYPER_OT11_Msk
            |GPIO_OTYPER_OT12_Msk
            |GPIO_OTYPER_OT13_Msk
            |GPIO_OTYPER_OT14_Msk
            |GPIO_OTYPER_OT15_Msk;
    mOR  =   (SD_OTYPE<<GPIO_OTYPER_OT0_Pos)
            |(SD_OTYPE<<GPIO_OTYPER_OT1_Pos)
            |(SD_OTYPE<<GPIO_OTYPER_OT7_Pos)
            |(SD_OTYPE<<GPIO_OTYPER_OT8_Pos)
            |(SD_OTYPE<<GPIO_OTYPER_OT9_Pos)
            |(SD_OTYPE<<GPIO_OTYPER_OT10_Pos)
            |(SD_OTYPE<<GPIO_OTYPER_OT11_Pos)
            |(SD_OTYPE<<GPIO_OTYPER_OT12_Pos)
            |(SD_OTYPE<<GPIO_OTYPER_OT13_Pos)
            |(SD_OTYPE<<GPIO_OTYPER_OT14_Pos)
            |(SD_OTYPE<<GPIO_OTYPER_OT15_Pos);
    GPIOE->OTYPER  = (GPIOE->OTYPER&~mAND)|mOR;

    // Configure pins in GPIOF
    // 0/A0 1/A1 2/A2 3/A3 4/A4 5/A5 11/RAS 12/A6 13/A7 14/A8 15/A9

    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOFEN;

    mAND =   GPIO_AFRL_AFRL0_Msk
            |GPIO_AFRL_AFRL1_Msk
            |GPIO_AFRL_AFRL2_Msk
            |GPIO_AFRL_AFRL3_Msk
            |GPIO_AFRL_AFRL4_Msk
            |GPIO_AFRL_AFRL5_Msk;
    mOR  =   (SD_AF<<GPIO_AFRL_AFRL0_Pos)
            |(SD_AF<<GPIO_AFRL_AFRL1_Pos)
            |(SD_AF<<GPIO_AFRL_AFRL2_Pos)
            |(SD_AF<<GPIO_AFRL_AFRL3_Pos)
            |(SD_AF<<GPIO_AFRL_AFRL4_Pos)
            |(SD_AF<<GPIO_AFRL_AFRL5_Pos);
    GPIOF->AFR[0]  = (GPIOF->AFR[0]&~mAND)|mOR;

    mAND =   GPIO_AFRH_AFRH3_Msk
            |GPIO_AFRH_AFRH4_Msk
            |GPIO_AFRH_AFRH5_Msk
            |GPIO_AFRH_AFRH6_Msk
            |GPIO_AFRH_AFRH7_Msk;
    mOR  =   (SD_AF<<GPIO_AFRH_AFRH3_Pos)
            |(SD_AF<<GPIO_AFRH_AFRH4_Pos)
            |(SD_AF<<GPIO_AFRH_AFRH5_Pos)
            |(SD_AF<<GPIO_AFRH_AFRH6_Pos)
            |(SD_AF<<GPIO_AFRH_AFRH7_Pos);
    GPIOF->AFR[1]  = (GPIOF->AFR[1]&~mAND)|mOR;

    mAND =   GPIO_MODER_MODER0_Msk
            |GPIO_MODER_MODER1_Msk
            |GPIO_MODER_MODER2_Msk
            |GPIO_MODER_MODER3_Msk
            |GPIO_MODER_MODER4_Msk
            |GPIO_MODER_MODER5_Msk
            |GPIO_MODER_MODER11_Msk
            |GPIO_MODER_MODER12_Msk
            |GPIO_MODER_MODER13_Msk
            |GPIO_MODER_MODER14_Msk
            |GPIO_MODER_MODER15_Msk;
    mOR  =   (SD_MODE<<GPIO_MODER_MODER0_Pos)
            |(SD_MODE<<GPIO_MODER_MODER1_Pos)
            |(SD_MODE<<GPIO_MODER_MODER2_Pos)
            |(SD_MODE<<GPIO_MODER_MODER3_Pos)
            |(SD_MODE<<GPIO_MODER_MODER4_Pos)
            |(SD_MODE<<GPIO_MODER_MODER5_Pos)
            |(SD_MODE<<GPIO_MODER_MODER11_Pos)
            |(SD_MODE<<GPIO_MODER_MODER12_Pos)
            |(SD_MODE<<GPIO_MODER_MODER13_Pos)
            |(SD_MODE<<GPIO_MODER_MODER14_Pos)
            |(SD_MODE<<GPIO_MODER_MODER15_Pos);
    GPIOF->MODER   = (GPIOF->MODER&~mAND)|mOR;

    mAND =   GPIO_OSPEEDR_OSPEEDR0_Msk
            |GPIO_OSPEEDR_OSPEEDR1_Msk
            |GPIO_OSPEEDR_OSPEEDR2_Msk
            |GPIO_OSPEEDR_OSPEEDR3_Msk
            |GPIO_OSPEEDR_OSPEEDR4_Msk
            |GPIO_OSPEEDR_OSPEEDR5_Msk
            |GPIO_OSPEEDR_OSPEEDR11_Msk
            |GPIO_OSPEEDR_OSPEEDR12_Msk
            |GPIO_OSPEEDR_OSPEEDR13_Msk
            |GPIO_OSPEEDR_OSPEEDR14_Msk
            |GPIO_OSPEEDR_OSPEEDR15_Msk;
    mOR  =   (SD_OSPEED<<GPIO_OSPEEDR_OSPEEDR0_Pos)
            |(SD_OSPEED<<GPIO_OSPEEDR_OSPEEDR1_Pos)
            |(SD_OSPEED<<GPIO_OSPEEDR_OSPEEDR2_Pos)
            |(SD_OSPEED<<GPIO_OSPEEDR_OSPEEDR3_Pos)
            |(SD_OSPEED<<GPIO_OSPEEDR_OSPEEDR4_Pos)
            |(SD_OSPEED<<GPIO_OSPEEDR_OSPEEDR5_Pos)
            |(SD_OSPEED<<GPIO_OSPEEDR_OSPEEDR11_Pos)
            |(SD_OSPEED<<GPIO_OSPEEDR_OSPEEDR12_Pos)
            |(SD_OSPEED<<GPIO_OSPEEDR_OSPEEDR13_Pos)
            |(SD_OSPEED<<GPIO_OSPEEDR_OSPEEDR14_Pos)
            |(SD_OSPEED<<GPIO_OSPEEDR_OSPEEDR15_Pos);
    GPIOF->OSPEEDR = (GPIOF->OSPEEDR&~mAND)|mOR;

    mAND =   GPIO_PUPDR_PUPDR0_Msk
            |GPIO_PUPDR_PUPDR1_Msk
            |GPIO_PUPDR_PUPDR2_Msk
            |GPIO_PUPDR_PUPDR3_Msk
            |GPIO_PUPDR_PUPDR4_Msk
            |GPIO_PUPDR_PUPDR5_Msk
            |GPIO_PUPDR_PUPDR11_Msk
            |GPIO_PUPDR_PUPDR12_Msk
            |GPIO_PUPDR_PUPDR13_Msk
            |GPIO_PUPDR_PUPDR14_Msk
            |GPIO_PUPDR_PUPDR15_Msk;
    mOR  =   (SD_PUPD<<GPIO_PUPDR_PUPDR0_Pos)
            |(SD_PUPD<<GPIO_PUPDR_PUPDR1_Pos)
            |(SD_PUPD<<GPIO_PUPDR_PUPDR2_Pos)
            |(SD_PUPD<<GPIO_PUPDR_PUPDR3_Pos)
            |(SD_PUPD<<GPIO_PUPDR_PUPDR4_Pos)
            |(SD_PUPD<<GPIO_PUPDR_PUPDR5_Pos)
            |(SD_PUPD<<GPIO_PUPDR_PUPDR11_Pos)
            |(SD_PUPD<<GPIO_PUPDR_PUPDR12_Pos)
            |(SD_PUPD<<GPIO_PUPDR_PUPDR13_Pos)
            |(SD_PUPD<<GPIO_PUPDR_PUPDR14_Pos)
            |(SD_PUPD<<GPIO_PUPDR_PUPDR15_Pos);
    GPIOF->PUPDR   = (GPIOF->PUPDR&~mAND)|mOR;

    mAND =   GPIO_OTYPER_OT0_Msk
            |GPIO_OTYPER_OT1_Msk
            |GPIO_OTYPER_OT2_Msk
            |GPIO_OTYPER_OT3_Msk
            |GPIO_OTYPER_OT4_Msk
            |GPIO_OTYPER_OT5_Msk
            |GPIO_OTYPER_OT11_Msk
            |GPIO_OTYPER_OT12_Msk
            |GPIO_OTYPER_OT13_Msk
            |GPIO_OTYPER_OT14_Msk
            |GPIO_OTYPER_OT15_Msk;
    mOR  =   (SD_OTYPE<<GPIO_OTYPER_OT0_Pos)
            |(SD_OTYPE<<GPIO_OTYPER_OT1_Pos)
            |(SD_OTYPE<<GPIO_OTYPER_OT2_Pos)
            |(SD_OTYPE<<GPIO_OTYPER_OT3_Pos)
            |(SD_OTYPE<<GPIO_OTYPER_OT4_Pos)
            |(SD_OTYPE<<GPIO_OTYPER_OT5_Pos)
            |(SD_OTYPE<<GPIO_OTYPER_OT11_Pos)
            |(SD_OTYPE<<GPIO_OTYPER_OT12_Pos)
            |(SD_OTYPE<<GPIO_OTYPER_OT13_Pos)
            |(SD_OTYPE<<GPIO_OTYPER_OT14_Pos)
            |(SD_OTYPE<<GPIO_OTYPER_OT15_Pos);
    GPIOF->OTYPER  = (GPIOF->OTYPER&~mAND)|mOR;

    // Configure pins in GPIOG
    // 0/A10 1/A11 4/BA0 5/BA1 8/CLK 15/CAS

    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOGEN;

    mAND =   GPIO_AFRL_AFRL0_Msk
            |GPIO_AFRL_AFRL1_Msk
            |GPIO_AFRL_AFRL4_Msk
            |GPIO_AFRL_AFRL5_Msk;
    mOR  =   (SD_AF<<GPIO_AFRL_AFRL0_Pos)
            |(SD_AF<<GPIO_AFRL_AFRL1_Pos)
            |(SD_AF<<GPIO_AFRL_AFRL4_Pos)
            |(SD_AF<<GPIO_AFRL_AFRL5_Pos);
    GPIOG->AFR[0]  = (GPIOG->AFR[0]&~mAND)|mOR;

    mAND =   GPIO_AFRH_AFRH0_Msk
            |GPIO_AFRH_AFRH7_Msk;
    mOR  =   (SD_AF<<GPIO_AFRH_AFRH0_Pos)
            |(SD_AF<<GPIO_AFRH_AFRH7_Pos);
    GPIOG->AFR[1]  = (GPIOG->AFR[1]&~mAND)|mOR;

    mAND =   GPIO_MODER_MODER0_Msk
            |GPIO_MODER_MODER1_Msk
            |GPIO_MODER_MODER4_Msk
            |GPIO_MODER_MODER5_Msk
            |GPIO_MODER_MODER8_Msk
            |GPIO_MODER_MODER15_Msk;
    mOR  =   (SD_MODE<<GPIO_MODER_MODER0_Pos)
            |(SD_MODE<<GPIO_MODER_MODER1_Pos)
            |(SD_MODE<<GPIO_MODER_MODER4_Pos)
            |(SD_MODE<<GPIO_MODER_MODER5_Pos)
            |(SD_MODE<<GPIO_MODER_MODER8_Pos)
            |(SD_MODE<<GPIO_MODER_MODER15_Pos);
    GPIOG->MODER   = (GPIOG->MODER&~mAND)|mOR;

    mAND =   GPIO_OSPEEDR_OSPEEDR0_Msk
            |GPIO_OSPEEDR_OSPEEDR1_Msk
            |GPIO_OSPEEDR_OSPEEDR4_Msk
            |GPIO_OSPEEDR_OSPEEDR5_Msk
            |GPIO_OSPEEDR_OSPEEDR8_Msk
            |GPIO_OSPEEDR_OSPEEDR15_Msk;
    mOR  =   (SD_OSPEED<<GPIO_OSPEEDR_OSPEEDR0_Pos)
            |(SD_OSPEED<<GPIO_OSPEEDR_OSPEEDR1_Pos)
            |(SD_OSPEED<<GPIO_OSPEEDR_OSPEEDR4_Pos)
            |(SD_OSPEED<<GPIO_OSPEEDR_OSPEEDR5_Pos)
            |(SD_OSPEED<<GPIO_OSPEEDR_OSPEEDR8_Pos)
            |(SD_OSPEED<<GPIO_OSPEEDR_OSPEEDR15_Pos);
    GPIOG->OSPEEDR = (GPIOG->OSPEEDR&~mAND)|mOR;

    mAND =   GPIO_PUPDR_PUPDR0_Msk
            |GPIO_PUPDR_PUPDR1_Msk
            |GPIO_PUPDR_PUPDR4_Msk
            |GPIO_PUPDR_PUPDR5_Msk
            |GPIO_PUPDR_PUPDR8_Msk
            |GPIO_PUPDR_PUPDR15_Msk;
    mOR  =   (SD_PUPD<<GPIO_PUPDR_PUPDR0_Pos)
            |(SD_PUPD<<GPIO_PUPDR_PUPDR1_Pos)
            |(SD_PUPD<<GPIO_PUPDR_PUPDR4_Pos)
            |(SD_PUPD<<GPIO_PUPDR_PUPDR5_Pos)
            |(SD_PUPD<<GPIO_PUPDR_PUPDR8_Pos)
            |(SD_PUPD<<GPIO_PUPDR_PUPDR15_Pos);
    GPIOG->PUPDR   = (GPIOG->PUPDR&~mAND)|mOR;

    mAND =   GPIO_OTYPER_OT0_Msk
            |GPIO_OTYPER_OT1_Msk
            |GPIO_OTYPER_OT4_Msk
            |GPIO_OTYPER_OT5_Msk
            |GPIO_OTYPER_OT8_Msk
            |GPIO_OTYPER_OT15_Msk;
    mOR  =   (SD_OTYPE<<GPIO_OTYPER_OT0_Pos)
            |(SD_OTYPE<<GPIO_OTYPER_OT1_Pos)
            |(SD_OTYPE<<GPIO_OTYPER_OT4_Pos)
            |(SD_OTYPE<<GPIO_OTYPER_OT5_Pos)
            |(SD_OTYPE<<GPIO_OTYPER_OT8_Pos)
            |(SD_OTYPE<<GPIO_OTYPER_OT15_Pos);
    GPIOG->OTYPER  = (GPIOG->OTYPER&~mAND)|mOR;

    // Configure pins in GPIOH
    // 5/WE

    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOHEN;

    mAND =   GPIO_AFRL_AFRL5_Msk;
    mOR  =   (SD_AF<<GPIO_AFRL_AFRL5_Pos);
    GPIOH->AFR[0]  = (GPIOH->AFR[0]&~mAND)|mOR;

    mAND =   0;
    mOR  =   0;
    GPIOH->AFR[1]  = (GPIOH->AFR[1]&~mAND)|mOR;

    mAND =   GPIO_MODER_MODER5_Msk;
    mOR  =   (SD_MODE<<GPIO_MODER_MODER5_Pos);
    GPIOH->MODER   = (GPIOH->MODER&~mAND)|mOR;

    mAND =   GPIO_OSPEEDR_OSPEEDR5_Msk;
    mOR  =   (SD_OSPEED<<GPIO_OSPEEDR_OSPEEDR5_Pos);
    GPIOH->OSPEEDR = (GPIOH->OSPEEDR&~mAND)|mOR;

    mAND =   GPIO_PUPDR_PUPDR5_Msk;
    mOR  =   (SD_PUPD<<GPIO_PUPDR_PUPDR5_Pos);
    GPIOH->PUPDR   = (GPIOH->PUPDR&~mAND)|mOR;

    mAND =   GPIO_OTYPER_OT5_Msk;
    mOR  =   (SD_OTYPE<<GPIO_OTYPER_OT5_Pos);
    GPIOH->OTYPER  = (GPIOH->OTYPER&~mAND)|mOR;

    /*
     * SDCKEx and SDNEx are bank specific
     * SDCKE0 : PH2 or PC3 (PC3 used in the Discovery board)
     * SDNE0  : PH3 or PC4 (PH3 used in the Discovery board)
     * SDCKE1 : PH7
     * SDNE1  : PH6
     *
     ()*/
    if( bank == SDRAM_BANK1 ) {
        // Configure pins in GPIOC
        // 3/CLKE

        RCC->AHB1ENR |= RCC_AHB1ENR_GPIOCEN;

        mAND = GPIO_AFRL_AFRL3_Msk;
        mOR  = (SD_AF<<GPIO_AFRL_AFRL3_Pos);
        GPIOC->AFR[0]  = (GPIOC->AFR[0]&~mAND)|mOR;

        mAND = GPIO_MODER_MODER3_Msk;
        mOR  = (SD_MODE<<GPIO_MODER_MODER3_Pos);
        GPIOC->MODER   = (GPIOC->MODER&~mAND)|mOR;

        mAND = GPIO_OSPEEDR_OSPEEDR3_Msk;
        mOR  = (SD_OSPEED<<GPIO_OSPEEDR_OSPEEDR3_Pos);
        GPIOC->OSPEEDR = (GPIOC->OSPEEDR&~mAND)|mOR;

        mAND = GPIO_PUPDR_PUPDR3_Msk;
        mOR  = (SD_PUPD<<GPIO_PUPDR_PUPDR3_Pos);
        GPIOC->PUPDR   = (GPIOC->PUPDR&~mAND)|mOR;

        mAND = GPIO_OTYPER_OT0_Msk;
        mOR  = (SD_OTYPE<<GPIO_OTYPER_OT3_Pos);
        GPIOC->OTYPER  = (GPIOC->OTYPER&~mAND)|mOR;

        // Configure pins in GPIOH
        // 3/CS

        RCC->AHB1ENR |= RCC_AHB1ENR_GPIOHEN;

        mAND =   GPIO_AFRL_AFRL3_Msk;
        mOR  =   (SD_AF<<GPIO_AFRL_AFRL3_Pos);
        GPIOH->AFR[0]  = (GPIOH->AFR[0]&~mAND)|mOR;

        mAND =   0;
        mOR  =   0;
        GPIOH->AFR[1]  = (GPIOH->AFR[1]&~mAND)|mOR;

        mAND =   GPIO_MODER_MODER3_Msk;
        mOR  =   (SD_MODE<<GPIO_MODER_MODER3_Pos);
        GPIOH->MODER   = (GPIOH->MODER&~mAND)|mOR;

        mAND =   GPIO_OSPEEDR_OSPEEDR3_Msk;
        mOR  =   (SD_OSPEED<<GPIO_OSPEEDR_OSPEEDR3_Pos);
        GPIOH->OSPEEDR = (GPIOH->OSPEEDR&~mAND)|mOR;

        mAND =   GPIO_PUPDR_PUPDR3_Msk;
        mOR  =   (SD_PUPD<<GPIO_PUPDR_PUPDR3_Pos);
        GPIOH->PUPDR   = (GPIOH->PUPDR&~mAND)|mOR;

        mAND =   GPIO_OTYPER_OT3_Msk;
        mOR  =   (SD_OTYPE<<GPIO_OTYPER_OT3_Pos);
        GPIOH->OTYPER  = (GPIOH->OTYPER&~mAND)|mOR;

    } else if ( bank == SDRAM_BANK2 ) {
        // Configure pins in GPIOH
        // 6/CS 7/CKE for Bank2 (There are alternatives on PB6 and PB5)

        RCC->AHB1ENR |= RCC_AHB1ENR_GPIOHEN;

        mAND =   GPIO_AFRL_AFRL6_Msk
                |GPIO_AFRL_AFRL7_Msk;
        mOR  =   (SD_AF<<GPIO_AFRL_AFRL0_Pos)
                |(SD_AF<<GPIO_AFRL_AFRL5_Pos);
        GPIOH->AFR[0]  = (GPIOG->AFR[0]&~mAND)|mOR;

        mAND =   GPIO_AFRH_AFRH6_Msk
                |GPIO_AFRH_AFRH7_Msk;
        mOR  =   (SD_AF<<GPIO_AFRH_AFRH6_Pos)
                |(SD_AF<<GPIO_AFRH_AFRH7_Pos);
        GPIOH->AFR[1]  = (GPIOH->AFR[1]&~mAND)|mOR;

        mAND =   GPIO_MODER_MODER6_Msk
                |GPIO_MODER_MODER7_Msk;
        mOR  =   (SD_MODE<<GPIO_MODER_MODER6_Pos)
                |(SD_MODE<<GPIO_MODER_MODER7_Pos);
        GPIOH->MODER   = (GPIOH->MODER&~mAND)|mOR;

        mAND =   GPIO_OSPEEDR_OSPEEDR6_Msk
                |GPIO_OSPEEDR_OSPEEDR7_Msk;
        mOR  =   (SD_OSPEED<<GPIO_OSPEEDR_OSPEEDR6_Pos)
                |(SD_OSPEED<<GPIO_OSPEEDR_OSPEEDR7_Pos);
        GPIOH->OSPEEDR = (GPIOH->OSPEEDR&~mAND)|mOR;

        mAND =   GPIO_PUPDR_PUPDR6_Msk
                |GPIO_PUPDR_PUPDR7_Msk;
        mOR  =   (SD_PUPD<<GPIO_PUPDR_PUPDR6_Pos)
                |(SD_PUPD<<GPIO_PUPDR_PUPDR7_Pos);
        GPIOH->PUPDR   = (GPIOH->PUPDR&~mAND)|mOR;

        mAND =   GPIO_OTYPER_OT6_Msk
                |GPIO_OTYPER_OT7_Msk;
        mOR  =   (SD_OTYPE<<GPIO_OTYPER_OT6_Pos)
                |(SD_OTYPE<<GPIO_OTYPER_OT7_Pos);
        GPIOH->OTYPER  = (GPIOH->OTYPER&~mAND)|mOR;

    }

}
#else

static const GPIO_PinConfiguration pinconfig_common[] = {
   {  GPIOD,   14,      12  },       //     DQ0
   {  GPIOD,   15,      12  },       //     DQ1
   {  GPIOD,   0,       12  },       //     DQ2
   {  GPIOD,   1,       12  },       //     DQ3
   {  GPIOE,   7,       12  },       //     DQ4
   {  GPIOE,   8,       12  },       //     DQ5
   {  GPIOE,   9,       12  },       //     DQ6
   {  GPIOE,   10,      12  },       //     DQ7
   {  GPIOE,   11,      12  },       //     DQ8
   {  GPIOE,   12,      12  },       //     DQ9
   {  GPIOE,   13,      12  },       //     DQ10
   {  GPIOE,   14,      12  },       //     DQ11
   {  GPIOE,   15,      12  },       //     DQ12
   {  GPIOD,   8,       12  },       //     DQ13
   {  GPIOD,   9,       12  },       //     DQ14
   {  GPIOD,   10,      12  },       //     DQ15
   {  GPIOF,   0,       12  },       //     A0
   {  GPIOF,   1,       12  },       //     A1
   {  GPIOF,   2,       12  },       //     A2
   {  GPIOF,   3,       12  },       //     A3
   {  GPIOF,   4,       12  },       //     A4
   {  GPIOF,   5,       12  },       //     A5
   {  GPIOF,   12,      12  },       //     A6
   {  GPIOF,   13,      12  },       //     A7
   {  GPIOF,   14,      12  },       //     A8
   {  GPIOF,   15,      12  },       //     A9
   {  GPIOG,   0,       12  },       //     A10
   {  GPIOG,   1,       12  },       //     A11
   {  GPIOG,   4,       12  },       //     BA0
   {  GPIOG,   5,       12  },       //     BA1
   {  GPIOF,   11,      12  },       //     RAS
   {  GPIOG,   15,      12  },       //     CAS
   {  GPIOH,   5,       12  },       //     WE
   {  GPIOG,   8,       12  },       //     CLK
   {  GPIOE,   0,       12  },       //     DQM0
   {  GPIOE,   1,       12  },       //     DQM1
//
   {     0,    0,       0  }         // End of List Mark
};


static const GPIO_PinConfiguration pinconfig_bank1[] = {
    // PC3/CLKE, PH3/CS
   {  GPIOC,   3,       12  },       //     CS = SDNE0
   {  GPIOH,   3,       12  },       //     CLKE = SDNE0
   {     0,    0,       0  }         // End of List Mark
};

static const GPIO_PinConfiguration pinconfig_bank2[] = {
    // 6/CS 7/CLKE for Bank2 (There are alternatives on PB6 and PB5)
   {  GPIOH,   6,       12  },       //     SDNE1
   {  GPIOH,   7,       12  },       //     SDCKE1
   {     0,    0,       0  }         // End of List Mark
};

static void
ConfigureFMCSDRAMPins(int bank) {

    /* Configure pins from table*/
    GPIO_ConfigureMultiplePins(pinconfig_common);

    if( bank == SDRAM_BANK1 ) {
        GPIO_ConfigureMultiplePins(pinconfig_bank1);
    } else {
        GPIO_ConfigureMultiplePins(pinconfig_bank2);
    }
}
#endif


/**
 *  @brief  EnableFMCClock
 *
 */

static void
EnableFMCClock(void) {

    RCC->AHB3ENR |= RCC_AHB3ENR_FMCEN;

}


/**
 *  @brief  Send Command to FMC
 *
 *  @note   Only for SDRAM Bank 1 (=FMC Bank 5)
 *
 *  @note   Parameters are in the same format of SDCMR Register
 *
 *  @note   returns 0 when runs OK or -1 if a timeout occurs
 */
static int
SendCommand(int bank, uint32_t command, uint32_t parameters, uint32_t timeout) {

    parameters &= ~(FMC_SDCMR_MODE_Msk|FMC_SDCMR_CTB1|FMC_SDCMR_CTB2);
    FMC_Bank5_6->SDCMR=(command<<FMC_SDCMR_MODE_Pos)|FMC_SDCMR_CTB1|parameters;

    while( (FMC_Bank5_6->SDSR&FMC_SDSR_BUSY) &&(timeout-->0) ) {}

    if( FMC_Bank5_6->SDSR&FMC_SDSR_BUSY )
        return 0;
    else
        return -1;
}


/**
 * @brief   SmallDelay
 *
 * @note    Quick and dirty small delay routine
 */
static void
SmallDelay(volatile uint32_t v) {

    while(v--) {}
}

/**
 *  @brief  ConfigureFMC for SDRAM
 *
 *  @note   All parameters for f_SDCLOCK = 100 MHz
 *
 *  @note   The SDRAM is in SDRAM Bank 1 (FMC Bank 5)
 *
 *  @note   FMC is configured to run at half the speed of the core.
 *
 *  @note   Autorefresh = 1 always?
 */

static void
ConfigureFMCSDRAM(int bank) {
uint32_t sdcr1,sdcr2;
uint32_t sdtr1,sdtr2;


    if( bank == SDRAM_BANK1 ) {
        sdcr1 = FMC_Bank5_6->SDCR[SDRAM_BANK1];
        sdtr1 = FMC_Bank5_6->SDTR[SDRAM_BANK1];
        /* Clear fields in SDCR1 */
        sdcr1 &=   ~(FMC_SDCR1_RPIPE_Msk
                    |FMC_SDCR1_RBURST_Msk
                    |FMC_SDCR1_SDCLK_Msk
                    |FMC_SDCR1_WP_Msk
                    |FMC_SDCR1_CAS_Msk
                    |FMC_SDCR1_MWID_Msk
                    |FMC_SDCR1_NR_Msk
                    |FMC_SDCR1_NC_Msk);
        /* Set fields in SDCR1 */
        sdcr1 |=    (SDRAM_RPIPE<<FMC_SDCR1_RPIPE_Pos)
                    |(SDRAM_RBURST<<FMC_SDCR1_RBURST_Pos)
                    |(SDRAM_SDCLK<<FMC_SDCR1_SDCLK_Pos)
                    |(SDRAM_WP<<FMC_SDCR1_WP_Pos)
                    |(SDRAM_CAS<<FMC_SDCR1_CAS_Pos)
                    |(SDRAM_NB<<FMC_SDCR1_NB_Pos)
                    |(SDRAM_MWID<<FMC_SDCR1_MWID_Pos)
                    |(SDRAM_NR<<FMC_SDCR1_NR_Pos)
                    |(SDRAM_NC<<FMC_SDCR1_NC_Pos);
        /* Clear fields in SDTR1 */
        sdtr1  &=   ~(FMC_SDTR1_TRCD_Msk)
                    |(FMC_SDTR1_TRP_Msk)
                    |(FMC_SDTR1_TWR_Msk)
                    |(FMC_SDTR1_TRC_Msk)
                    |(FMC_SDTR1_TRAS_Msk)
                    |(FMC_SDTR1_TXSR_Msk)
                    |(FMC_SDTR1_TMRD_Msk);
        /* Set fields in SDTR1 */
        sdtr1  |=    (SDRAM_TRCD<<FMC_SDTR1_TRCD_Pos)
                    |(SDRAM_TRP<<FMC_SDTR1_TRP_Pos)
                    |(SDRAM_TWR<<FMC_SDTR1_TWR_Pos)
                    |(SDRAM_TRC<<FMC_SDTR1_TRC_Pos)
                    |(SDRAM_TRAS<<FMC_SDTR1_TRAS_Pos)
                    |(SDRAM_TXSR<<FMC_SDTR1_TXSR_Pos)
                    |(SDRAM_TMRD<<FMC_SDTR1_TMRD_Pos);

        FMC_Bank5_6->SDCR[SDRAM_BANK1] = sdcr1;
        FMC_Bank5_6->SDTR[SDRAM_BANK1] = sdtr1;
    } else {
        sdcr1 = FMC_Bank5_6->SDCR[SDRAM_BANK1];
        sdcr2 = FMC_Bank5_6->SDCR[SDRAM_BANK2];
        sdtr1 = FMC_Bank5_6->SDTR[SDRAM_BANK1];
        sdtr2 = FMC_Bank5_6->SDTR[SDRAM_BANK2];
        /* Clear fields that only can be written in SDCR1 */
        sdcr1 &=   ~(FMC_SDCR1_RPIPE_Msk
                    |FMC_SDCR1_RBURST_Msk
                    |FMC_SDCR1_SDCLK_Msk);
        /* Set fields in SDCR1 */
        sdcr1 |=    (SDRAM_RPIPE<<FMC_SDCR1_RPIPE_Pos)
                    |(SDRAM_RBURST<<FMC_SDCR1_RBURST_Pos)
                    |(SDRAM_SDCLK<<FMC_SDCR1_SDCLK_Pos);
        /* Clear fields in SDCR2 */
        sdcr2 &=   ~(FMC_SDCR1_WP_Msk
                    |FMC_SDCR1_CAS_Msk
                    |FMC_SDCR1_MWID_Msk
                    |FMC_SDCR1_NR_Msk
                    |FMC_SDCR1_NC_Msk);
        /* Set fields in SDCR2 */
        sdcr2 |=     (SDRAM_WP<<FMC_SDCR1_WP_Pos)
                    |(SDRAM_CAS<<FMC_SDCR1_CAS_Pos)
                    |(SDRAM_NB<<FMC_SDCR1_NB_Pos)
                    |(SDRAM_MWID<<FMC_SDCR1_MWID_Pos)
                    |(SDRAM_NR<<FMC_SDCR1_NR_Pos)
                    |(SDRAM_NC<<FMC_SDCR1_NC_Pos);
        /* Clear fields that only can be written in SDTR1 */
        sdtr1 &=   ~(FMC_SDTR1_TWR_Msk);
        /* Set fields that only can be writter in SDTR1 */
        sdtr1 |=    (SDRAM_TWR<<FMC_SDTR1_TWR_Pos);
        /* Clear fields in SDTR2 */
        sdtr2  &    ~(FMC_SDTR1_TRCD_Msk)
                    |(FMC_SDTR1_TRP_Msk)
                    |(FMC_SDTR1_TRC_Msk)
                    |(FMC_SDTR1_TRAS_Msk)
                    |(FMC_SDTR1_TXSR_Msk)
                    |(FMC_SDTR1_TMRD_Msk);
        /* Set fields in SDTR2 */
        sdtr2  =     (SDRAM_TRCD<<FMC_SDTR1_TRCD_Pos)
                    |(SDRAM_TRP<<FMC_SDTR1_TRP_Pos)
                    |(SDRAM_TRC<<FMC_SDTR1_TRC_Pos)
                    |(SDRAM_TRAS<<FMC_SDTR1_TRAS_Pos)
                    |(SDRAM_TXSR<<FMC_SDTR1_TXSR_Pos)
                    |(SDRAM_TMRD<<FMC_SDTR1_TMRD_Pos);

        FMC_Bank5_6->SDCR[SDRAM_BANK1] = sdcr1;
        FMC_Bank5_6->SDCR[SDRAM_BANK2] = sdcr2;
        FMC_Bank5_6->SDTR[SDRAM_BANK1] = sdtr1;
        FMC_Bank5_6->SDTR[SDRAM_BANK2] = sdtr2;
    }

    sdtr1  =     (SDRAM_TRCD<<FMC_SDTR1_TRCD_Pos)
                |(SDRAM_TRP<<FMC_SDTR1_TRP_Pos)
                |(SDRAM_TWR<<FMC_SDTR1_TWR_Pos)
                |(SDRAM_TRC<<FMC_SDTR1_TRC_Pos)
                |(SDRAM_TRAS<<FMC_SDTR1_TRAS_Pos)
                |(SDRAM_TXSR<<FMC_SDTR1_TXSR_Pos)
                |(SDRAM_TMRD<<FMC_SDTR1_TMRD_Pos);


}

/**
 *  @brief  Configure Refresh Rate
 *
 */
static void
ConfigureSDRAMRefresh(int bank) {

    /* Set refresh count */
    FMC_Bank5_6->SDRTR = (FMC_Bank5_6->SDRTR&~(FMC_SDRTR_COUNT_Msk))
                |(SDRAM_REFRESH<<FMC_SDRTR_COUNT_Pos);

    /* Disable write protection */
    FMC_Bank5_6->SDCR[bank] &= ~(FMC_SDCR1_WP);

}
/**
 *  @brief  ConfigureFMC for SDRAM
 *
 *  @note   All parameters for f_SDCLOCK = 100 MHz
 *
 *  @note   The FCM SDRAM interface must be configured to run at half the speed
 *          of the core.
 *
 *  @note   Autorefresh = 1 always
 */

static void
ConfigureSDRAM(int bank) {

    /*
     * SDRAM initialization sequence
     */

    /* Clock enable command */
    SendCommand(bank,SDRAM_COMMAND_CLOCKCONFIGENABLE,0x0000,DEFAULT_TIMEOUT);

    SmallDelay(1000);       // 100 us, maybe systick is better */

    /* PALL command */
    SendCommand(bank,SDRAM_COMMAND_PALL,0x0000,DEFAULT_TIMEOUT);

    /* Auto refresh command */
    SendCommand(bank,SDRAM_COMMAND_AUTOREFRESH,(SDRAM_AUTOREFRESH<<FMC_SDCMR_NRFS_Pos),DEFAULT_TIMEOUT);

    /* MRD register program */
    SendCommand(bank,SDRAM_COMMAND_LOADMODE,(SDRAM_MODE<<FMC_SDCMR_MRD_Pos),DEFAULT_TIMEOUT);


}

/**
 * @brief   SDRAM Init
 *
 * @note    Initializes the FMC unit and configure access to a SDRAM
 *
 * @note    HCLK must be 200 MHz!!!!
 */
int
SDRAM_Init(int bank) {

    /* The board has only one SDRAM, at Bank 1 */
    if( bank != SDRAM_BANK1 )
        return -1;

    if( SystemCoreClock != SDRAM_CLOCKFREQUENCY )
        return -1;

    // Enable clock for FMC
    EnableFMCClock();

    /* Configure FMC pins for SDRAM interface*/
    ConfigureFMCSDRAMPins(SDRAM_BANK1);

    /* Configure FMC interface for SDRAM */
    ConfigureFMCSDRAM(SDRAM_BANK1);

    /* Configure SDRAM chip */
    ConfigureSDRAM(SDRAM_BANK1);

    /* Configure Refresh */
    ConfigureSDRAMRefresh(SDRAM_BANK1);

    return 0;
}

