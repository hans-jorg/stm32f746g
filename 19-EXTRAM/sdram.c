
/**
 * @file    sdram.c
 *
 * RAM Device Information
 * ======================
 *
 * @note The board uses a MT48LC4M32B2B5-6A SDRAM integrated circuit to
 *    expand its RAM using the Flexible Memory Controller.
 *
 * @note The device is a PC166/PC100 compatible SDRAM and it has four banks of 1 M cell
 *    with 32 bits. In total, it has then 128 MBits (=16 Mbytes) but only half
 *    of them will be reached due to limitations in the MCU.
 *
 * @note It has the following interface
 *
 * Signal    | Type |     Description
 * ----------|------------------------------------
 * DQ31..0   |  I/O | 32-bit data bus
 * A11..0    |  I   | 12 bit address input
 * BA1..0    |  I   | 2 bit bank selector
 * DQM3..0   |  I   | 4 bit mask input to enable byte write operation
 * CAS#      |  I   | Column Address Selector
 * RAS#      |  I   | Row Address Selector
 * WE#       |  I   | Write operation
 * CS#       |  I   | Chip select
 * CKE       |  I   | Clock enable
 * CLK       |  I   | Clock
 *
 * @note Only 16 bits of data bus are used. So it is then not neccessary to
 *   use full DQM. Only DQM0 and DQM1 need to be used.
/**
 * @brief   SDRAM characteristics
 *
 *  Parameter          |           Value
 *  -------------------|-----------------------------
 *  Configuration      | 4 banks of 1 Mega 32 bits words
 *  Capacity           | 128 Mbit = 16 MByte (Only 8 are used )
 *  Refresh count      | 4096
 *  Row addressing     | A11:0    (12 bits = 4K)
 *  Column addresssing | A7:0     (8 bits = 256)
 *  Bank addressing    | BA1:0    (2 bits = 4 )
 *                     |
 * Total addressing    | 2+8+12 = 22 bits = 4194304 addresses = 4 M
 *                     | 32 bit words  = 128 Mbit
 *                     |  4 bytes      =  16 MBytes
 *                     |
 * Burst lenghts       | 1,2,4,8
 * CAS latency         | 1,2,3
 * Autorefresh         | 64 ms x 4096
 * Cycle time          | 167 MHz (-6A) ???)
 * Self refresh        |
 * Auto precharge      |
 *                     |
 * Timing              |
 * Clock Frequency     | < 167 MHz
 * Cycle               | 3-3-3
 * t_RCD               | 18 ns
 * t_RP                | 18 ns
 * CL                  | 18 ns
 *                     |
 * Minimum frequency   |
 *     CL=3            |  166 MHz => 6 ns
 *     CL=2            |  100 MHz => 10 ns
 *     CL=1            |   50 MHz => 20 ns
 *
 *
 *  FMC Configuration Information
 *  =============================
 *
 * @note It uses Bank 4 and 5 for FMC SDRAM. This board is hardwired to use the Bank 4,
 *       a.k.a, SDRAM Bank1 due to the use of SDCKE0 and SDNE0
 *
 *   Bank |   Size     | Address range
 * -------|------------|-----------------------
 *    4   |  4x64 MB   | 0xC00_0000-0xCFFF_FFFF
 *    5   |  4x64 MB   | 0xD00_0000-0xDFFF_FFFF
 *
 * @note Example
 *
 * For a 16 bit memory width (Table 55 of RM))
 *      * Bit 28 specifies which FMC bank to be accessed.
 *      * Bits 22:21 which bank for 16-bit memory
 *      * Bits 20:9 row address
 *      * Bits 8:1  column address
 *      * Bit0 must be connected to memory A0 (BM0)
 *
 *
 * @note SDRAM Connection to MCU
 *
 *    Chip signal | Board signal | MCU Signal
 *    ------------|--------------|---------|------------
 *    DQ0         |  FMC_D0      | PD14    |  AF12
 *    DQ1         |  FMC_D1      | PD15    |  AF12
 *    DQ2         |  FMC_D2      | PD0     |  AF12
 *    DQ3         |  FMC_D3      | PD1     |  AF12
 *    DQ4         |  FMC_D4      | PE7     |  AF12
 *    DQ5         |  FMC_D5      | PE8     |  AF12
 *    DQ6         |  FMC_D6      | PE9     |  AF12
 *    DQ7         |  FMC_D7      | PE10    |  AF12
 *    DQ8         |  FMC_D8      | PE11    |  AF12
 *    DQ9         |  FMC_D9      | PE12    |  AF12
 *    DQ10        |  FMC_D10     | PE13    |  AF12
 *    DQ11        |  FMC_D11     | PE14    |  AF12
 *    DQ12        |  FMC_D12     | PE15    |  AF12
 *    DQ13        |  FMC_D13     | PD8     |  AF12
 *    DQ14        |  FMC_D14     | PD9     |  AF12
 *    DQ15        |  FMC_D15     | PD10    |  AF12
 *    A0          |  FMC_A0      | PF0     |  AF12
 *    A1          |  FMC_A1      | PF1     |  AF12
 *    A2          |  FMC_A2      | PF2     |  AF12
 *    A3          |  FMC_A3      | PF3     |  AF12
 *    A4          |  FMC_A4      | PF4     |  AF12
 *    A5          |  FMC_A5      | PF5     |  AF12
 *    A6          |  FMC_A6      | PF12    |  AF12
 *    A7          |  FMC_A7      | PF13    |  AF12
 *    A8          |  FMC_A8      | PF14    |  AF12
 *    A9          |  FMC_A9      | PF15    |  AF12
 *    A10         |  FMC_A10     | PG0     |  AF12
 *    A11         |  FMC_A11     | PG1     |  AF12
 *    BA0         |  FMC_BA0     | PG4     |  AF12
 *    BA1         |  FMC_BA1     | PG5     |  AF12
 *    RAS         |  FMC_SDNRAS  | PF11    |  AF12
 *    CAS         |  FMC_SDNCAS  | PG15    |  AF12
 *    WE          |  FMC_SNDWE   | PH5     |  AF12
 *    CLK         |  FMC_SDCLK   | PG8     |  AF12
 *    CLKE        |  FMC_SDCKE0  | PC3     |  AF12
 *    CS          |  FMC_SDNE0   | PH3     |  AF12
 *    DQM0        |  FMC_NBL0    | PE0     |  AF12
 *    DQM1        |  FMC_NBL1    | PE1     |  AF12
 *
 * @note Timing parameters (in clock units)
 *
 *  Field   |   Description
 * ---------|----------------------------------------------------
 *  TMRD    | Load Mode Register to Active
 *  TXSR    | Exit self refresh delay
 *  TRAS    | Self refresh time, i.e., the minimum Self-refresh period
 *  TRC     | Row cycle delay. i.e. the delay between the Refresh command and the Activate command
 *  TWR     | Recovery delay, i.e., delay between a Write and a Precharge command
 *  TRP     | Row precharge delay, i.e., delay between a Precharge command and another command
 *  TRCD    | Row to column delay
 *
 * @note Timing information for f_SDCLK = 100 MHz
 *
  *Parameter| Encoding | Recommended | Description
 * ---------|----------|-------------|--------------------------------------------------------
 *  TMRD    | 2        | 2 t_CK      | LOAD MODE REGISTER command to ACTIVE or REFRESH command
 *  TXSR    | 6.       | 67 ns       | Exit Self-Refresh to Active Delay
 *          |          |             |    OBS: 5 is used in the example
 *  TRAS    | 5        | 60 ns       | Self refresh time ?=? Auto refresh time
 *  TRC     |          |             |
 *  TWR     | 0        |  3 ns       | Data-in to PRECHARGE command
 *  TRP     | 1        | 18 ns       | PRECHARGE command period
 *  TRCD

 *
 * @note
 *   The initialization sequence is managed by software. If the two banks are used, the
 *   initialization sequence must be generated simultaneously to Bank 1and Bank 2 by setting
 *   the Target Bank bits CTB1 and CTB2 in the FMC_SDCMR register:
 *
 *   1. Program the memory device features into the FMC_SDCRx register. The SDRAM
 *      clock frequency, RBURST and RPIPE must be programmed in the FMC_SDCR1
 *      register.
 *   2. Program the memory device timing into the FMC_SDTRx register. The TRP and TRC
 *      timings must be programmed in the FMC_SDTR1 register.
 *   3. Set MODE bits to ‘001’ and configure the Target Bank bits (CTB1 and/or CTB2) in the
 *      FMC_SDCMR register to start delivering the clock to the memory (SDCKE is driven
 *      high).
 *   4. Wait during the prescribed delay period. Typical delay is around 100 μs (refer to the
 *      SDRAM datasheet for the required delay after power-up).
 *   5. Set MODE bits to ‘010’ and configure the Target Bank bits (CTB1 and/or CTB2) in the
 *      FMC_SDCMR register to issue a “Precharge All” command.
 *   6. Set MODE bits to ‘011’, and configure the Target Bank bits (CTB1 and/or CTB2) as well
 *      as the number of consecutive Auto-refresh commands (NRFS) in the FMC_SDCMR
 *      register. Refer to the SDRAM datasheet for the number of Auto-refresh commands that
 *      should be issued. Typical number is 8.
 *   7. Configure the MRD field according to the SDRAM device, set the MODE bits to '100',
 *      and configure the Target Bank bits (CTB1 and/or CTB2) in the FMC_SDCMR register
 *      to issue a "Load Mode Register" command in order to program the SDRAM device.
 *      In particular:
 *      a) the CAS latency must be selected following configured value in FMC_SDCR1/2
 *         registers
 *      b) the Burst Length (BL) of 1 must be selected by configuring the M[2:0] bits to 000 in
 *         the mode register. Refer to SDRAM device datasheet.
 *         If the Mode Register is not the same for both SDRAM banks, this step has to be
 *         repeated twice, once for each bank, and the Target Bank bits set accordingly.
 *   8. Program the refresh rate in the FMC_SDRTR register. The refresh rate corresponds to
 *      the delay between refresh cycles. Its value must be adapted to SDRAM devices.
 *   9. For mobile SDRAM devices, to program the extended mode register it should be done
 *      once the SDRAM device is initialized: First, a dummy read access should be performed
 *      while BA1=1 and BA=0 (refer to SDRAM address mapping section for BA[1:0] address
 *      mapping) in order to select the extended mode register instead of the load mode
 *      register and then program the needed value.
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
 * @brief   General configuration
 *
 * @note    All parameters are set for a SDRAM clock frequency of 100 MHz
 *
 * @note    The SD_CLOCK is derived from the HCLK (Core Clock).
 *
 * @note    There are one divider affecting the SDRAM CLK (SD_CLK):
 *          SDCLK1:0 of FMC SDCR1 register.
 *          with the options:
 *                  00:  Disable FCLK
 *                  01:  Do not use
 *                  10:  f_HCLK/2
 *                  11:  f_HCLK/3
 *
 * @note
 *
 *
 *
 */

#if 0
/**
 * @brief   Parameters for the MT48LC4M32B2B5
 *
 * @note    COUNT = SDRAM_Refresh_period/NROWS - 20
 *
 * @note    Refresh rate = (COUNT+1)xfreq_SDRAMCLK
 *
 * @note    Example: Refresh rate = 64 ms/8196 = 7.81 us
 *                   This times 60 MHz = 468.6
 *                   Add 20 as a safe margin
 *
 * @note    COUNT=64 ms / 4096 - 20
 *
 */
///@{
// SDCRx
#define SDBURST     1
#define SDCLOCK     2
#define SDCAS       3
#define SDBANKS     1
#define SDMEMWID    1
#define SDNROWS     1
#define SDNCOLS     2
// SDTRx
#define SDR2CDELAY  16
#define SDRTR       16
#define SDTWR       16
#define SDROWDELAY  16
#define SDTRAS      16
#define SDSSELFREF  16
#define SDRMRD      16

// Refresh timer
///@}

#endif

/**
 * @brief   Pin initialization
 *
 * @note    Uncomment the following define to initialize without GPIO routines and tables
 */
///@{
//#define USE_FAST_INITIALIZATION
///@}

#define SDRAMBIT(N) (1U<<(N))


#ifdef USE_FAST_INITIALIZATION

static void
ConfigureFMCSDRAMPins(void) {
uint32_t mAND,mOR; // Mask

    // Configure pins in GPIOC
    // 3/CLKE

    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOCEN;

    mAND = GPIO_AFRL_AFRL3_Msk;
    mOR  = (12<<GPIO_AFRL_AFRL3_Pos);
    GPIOC->AFR[0]  = (GPIOC->AFR[0]&~mAND)|mOR;

    mAND = GPIO_MODER_MODER3_Msk;
    mOR  = GPIO_MODER_MODER3;
    GPIOC->MODER   = (GPIOC->MODER&~mAND)|mOR;

    mAND = GPIO_OSPEEDR_OSPEEDR3_Msk;
    mOR  = GPIO_OSPEEDR_OSPEEDR3;
    GPIOC->OSPEEDR = (GPIOC->OSPEEDR&~mAND)|mOR;

    mAND = GPIO_PUPDR_PUPDR3_Msk;
    mOR  = GPIO_PUPDR_PUPDR3_1;
    GPIOC->PUPDR   = (GPIOC->PUPDR&~mAND)|mOR;

    mAND = GPIO_OTYPER_OT0_Msk;
    mOR  = GPIO_OTYPER_OT3;
    GPIOC->OTYPER  = (GPIOC->OTYPER&~mAND)|mOR;

    // Configure pins in GPIOD
    // 0/DQ2 1/DQ3 8/DQ13 9/DQ14 10/DQ15 14/DQ0 15/DQ1

    RCC->AHB1ENR |= RCC_AHB1ENR_GPIODEN;

    mAND =   GPIO_AFRL_AFRL0_Msk
            |GPIO_AFRL_AFRL1_Msk;
    mOR  =   (12<<GPIO_AFRL_AFRL0_Pos)
            |(12<<GPIO_AFRL_AFRL1_Pos);
    GPIOD->AFR[0]  = (GPIOD->AFR[0]&~mAND)|mOR;

    mAND =   GPIO_AFRH_AFRH0_Msk
            |GPIO_AFRH_AFRH1_Msk
            |GPIO_AFRH_AFRH2_Msk
            |GPIO_AFRH_AFRH6_Msk
            |GPIO_AFRH_AFRH7_Msk;
    mOR  =   (12<<GPIO_AFRH_AFRH0_Pos)
            |(12<<GPIO_AFRH_AFRH1_Pos)
            |(12<<GPIO_AFRH_AFRH2_Pos)
            |(12<<GPIO_AFRH_AFRH6_Pos)
            |(12<<GPIO_AFRH_AFRH7_Pos);
    GPIOD->AFR[1]  = (GPIOD->AFR[1]&~mAND)|mOR;

    mAND =   GPIO_MODER_MODER0_Msk
            |GPIO_MODER_MODER1_Msk
            |GPIO_MODER_MODER8_Msk
            |GPIO_MODER_MODER9_Msk
            |GPIO_MODER_MODER10_Msk
            |GPIO_MODER_MODER14_Msk
            |GPIO_MODER_MODER15_Msk;
    mOR  =   GPIO_MODER_MODER0
            |GPIO_MODER_MODER1
            |GPIO_MODER_MODER8
            |GPIO_MODER_MODER9
            |GPIO_MODER_MODER10
            |GPIO_MODER_MODER14
            |GPIO_MODER_MODER15;
    GPIOD->MODER   = (GPIOD->MODER&~mAND)|mOR;

    mAND =   GPIO_OSPEEDR_OSPEEDR0_Msk
            |GPIO_OSPEEDR_OSPEEDR1_Msk
            |GPIO_OSPEEDR_OSPEEDR8_Msk
            |GPIO_OSPEEDR_OSPEEDR9_Msk
            |GPIO_OSPEEDR_OSPEEDR10_Msk
            |GPIO_OSPEEDR_OSPEEDR14_Msk
            |GPIO_OSPEEDR_OSPEEDR15_Msk;
    mOR  =   GPIO_OSPEEDR_OSPEEDR0
            |GPIO_OSPEEDR_OSPEEDR1
            |GPIO_OSPEEDR_OSPEEDR8
            |GPIO_OSPEEDR_OSPEEDR9
            |GPIO_OSPEEDR_OSPEEDR10
            |GPIO_OSPEEDR_OSPEEDR14
            |GPIO_OSPEEDR_OSPEEDR15;
    GPIOD->OSPEEDR = (GPIOD->OSPEEDR&~mAND)|mOR;

    mAND =   GPIO_PUPDR_PUPDR0_Msk
            |GPIO_PUPDR_PUPDR1_Msk
            |GPIO_PUPDR_PUPDR8_Msk
            |GPIO_PUPDR_PUPDR9_Msk
            |GPIO_PUPDR_PUPDR10_Msk
            |GPIO_PUPDR_PUPDR14_Msk
            |GPIO_PUPDR_PUPDR15_Msk;
    mOR  =   GPIO_PUPDR_PUPDR0
            |GPIO_PUPDR_PUPDR1
            |GPIO_PUPDR_PUPDR8
            |GPIO_PUPDR_PUPDR9
            |GPIO_PUPDR_PUPDR10
            |GPIO_PUPDR_PUPDR14
            |GPIO_PUPDR_PUPDR15;
    GPIOD->PUPDR   = (GPIOD->PUPDR&~mAND)|mOR;

    mAND =   GPIO_OTYPER_OT0_Msk
            |GPIO_OTYPER_OT1_Msk
            |GPIO_OTYPER_OT8_Msk
            |GPIO_OTYPER_OT9_Msk
            |GPIO_OTYPER_OT10_Msk
            |GPIO_OTYPER_OT14_Msk
            |GPIO_OTYPER_OT15_Msk;
    mOR  =   GPIO_OTYPER_OT0
            |GPIO_OTYPER_OT1
            |GPIO_OTYPER_OT8
            |GPIO_OTYPER_OT9
            |GPIO_OTYPER_OT10
            |GPIO_OTYPER_OT14
            |GPIO_OTYPER_OT15;
    GPIOD->OTYPER  = (GPIOD->OTYPER&~mAND)|mOR;

    // Configure pins in GPIOE
    // 0/DQM0 1/DQM1 7/DQ4 8/DQ5 9/DQ6 10/DQ7 11/DQ8 12/DQ9 13/DQ10 14/DQ11 15/DQ12

    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOEEN;

    mAND =   GPIO_AFRL_AFRL0_Msk
            |GPIO_AFRL_AFRL1_Msk
            |GPIO_AFRL_AFRL7_Msk;
    mOR  =   (12<<GPIO_AFRL_AFRL0_Pos)
            |(12<<GPIO_AFRL_AFRL1_Pos)
            |(12<<GPIO_AFRL_AFRL7_Pos);
    GPIOE->AFR[0]  = (GPIOE->AFR[0]&~mAND)|mOR;

    mAND =   GPIO_AFRH_AFRH0_Msk
            |GPIO_AFRH_AFRH1_Msk
            |GPIO_AFRH_AFRH2_Msk
            |GPIO_AFRH_AFRH3_Msk
            |GPIO_AFRH_AFRH4_Msk
            |GPIO_AFRH_AFRH5_Msk
            |GPIO_AFRH_AFRH6_Msk
            |GPIO_AFRH_AFRH7_Msk;
    mOR  =   (12<<GPIO_AFRH_AFRH0_Pos)
            |(12<<GPIO_AFRH_AFRH1_Pos)
            |(12<<GPIO_AFRH_AFRH2_Pos)
            |(12<<GPIO_AFRH_AFRH3_Pos)
            |(12<<GPIO_AFRH_AFRH4_Pos)
            |(12<<GPIO_AFRH_AFRH5_Pos)
            |(12<<GPIO_AFRH_AFRH6_Pos)
            |(12<<GPIO_AFRH_AFRH7_Pos);
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
    mOR  =   GPIO_MODER_MODER0
            |GPIO_MODER_MODER1
            |GPIO_MODER_MODER7
            |GPIO_MODER_MODER8
            |GPIO_MODER_MODER9
            |GPIO_MODER_MODER10
            |GPIO_MODER_MODER11
            |GPIO_MODER_MODER12
            |GPIO_MODER_MODER13
            |GPIO_MODER_MODER14
            |GPIO_MODER_MODER15;
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
    mOR  =   GPIO_OSPEEDR_OSPEEDR0
            |GPIO_OSPEEDR_OSPEEDR1
            |GPIO_OSPEEDR_OSPEEDR7
            |GPIO_OSPEEDR_OSPEEDR8
            |GPIO_OSPEEDR_OSPEEDR9
            |GPIO_OSPEEDR_OSPEEDR10
            |GPIO_OSPEEDR_OSPEEDR11
            |GPIO_OSPEEDR_OSPEEDR12
            |GPIO_OSPEEDR_OSPEEDR13
            |GPIO_OSPEEDR_OSPEEDR14
            |GPIO_OSPEEDR_OSPEEDR15;
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
    mOR  =   GPIO_PUPDR_PUPDR0
            |GPIO_PUPDR_PUPDR1
            |GPIO_PUPDR_PUPDR7
            |GPIO_PUPDR_PUPDR8
            |GPIO_PUPDR_PUPDR9
            |GPIO_PUPDR_PUPDR10
            |GPIO_PUPDR_PUPDR11
            |GPIO_PUPDR_PUPDR12
            |GPIO_PUPDR_PUPDR13
            |GPIO_PUPDR_PUPDR14
            |GPIO_PUPDR_PUPDR15;
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
    mOR  =   GPIO_OTYPER_OT0
            |GPIO_OTYPER_OT1
            |GPIO_OTYPER_OT7
            |GPIO_OTYPER_OT8
            |GPIO_OTYPER_OT9
            |GPIO_OTYPER_OT10
            |GPIO_OTYPER_OT11
            |GPIO_OTYPER_OT12
            |GPIO_OTYPER_OT13
            |GPIO_OTYPER_OT14
            |GPIO_OTYPER_OT15;
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
    mOR  =   (12<<GPIO_AFRL_AFRL0_Pos)
            |(12<<GPIO_AFRL_AFRL1_Pos)
            |(12<<GPIO_AFRL_AFRL2_Pos)
            |(12<<GPIO_AFRL_AFRL3_Pos)
            |(12<<GPIO_AFRL_AFRL4_Pos)
            |(12<<GPIO_AFRL_AFRL5_Pos);
    GPIOF->AFR[0]  = (GPIOF->AFR[0]&~mAND)|mOR;

    mAND =   GPIO_AFRH_AFRH3_Msk
            |GPIO_AFRH_AFRH4_Msk
            |GPIO_AFRH_AFRH5_Msk
            |GPIO_AFRH_AFRH6_Msk
            |GPIO_AFRH_AFRH7_Msk;
    mOR  =   (12<<GPIO_AFRH_AFRH3_Pos)
            |(12<<GPIO_AFRH_AFRH4_Pos)
            |(12<<GPIO_AFRH_AFRH5_Pos)
            |(12<<GPIO_AFRH_AFRH6_Pos)
            |(12<<GPIO_AFRH_AFRH7_Pos);
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
    mOR  =   GPIO_MODER_MODER0
            |GPIO_MODER_MODER1
            |GPIO_MODER_MODER2
            |GPIO_MODER_MODER3
            |GPIO_MODER_MODER4
            |GPIO_MODER_MODER5
            |GPIO_MODER_MODER11
            |GPIO_MODER_MODER12
            |GPIO_MODER_MODER13
            |GPIO_MODER_MODER14
            |GPIO_MODER_MODER15;
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
    mOR  =   GPIO_OSPEEDR_OSPEEDR0
            |GPIO_OSPEEDR_OSPEEDR1
            |GPIO_OSPEEDR_OSPEEDR2
            |GPIO_OSPEEDR_OSPEEDR3
            |GPIO_OSPEEDR_OSPEEDR4
            |GPIO_OSPEEDR_OSPEEDR5
            |GPIO_OSPEEDR_OSPEEDR11
            |GPIO_OSPEEDR_OSPEEDR12
            |GPIO_OSPEEDR_OSPEEDR13
            |GPIO_OSPEEDR_OSPEEDR14
            |GPIO_OSPEEDR_OSPEEDR15;
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
    mOR  =   GPIO_PUPDR_PUPDR0
            |GPIO_PUPDR_PUPDR1
            |GPIO_PUPDR_PUPDR2
            |GPIO_PUPDR_PUPDR3
            |GPIO_PUPDR_PUPDR4
            |GPIO_PUPDR_PUPDR5
            |GPIO_PUPDR_PUPDR11
            |GPIO_PUPDR_PUPDR12
            |GPIO_PUPDR_PUPDR13
            |GPIO_PUPDR_PUPDR14
            |GPIO_PUPDR_PUPDR15;
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
    mOR  =   GPIO_OTYPER_OT0
            |GPIO_OTYPER_OT1
            |GPIO_OTYPER_OT2
            |GPIO_OTYPER_OT3
            |GPIO_OTYPER_OT4
            |GPIO_OTYPER_OT5
            |GPIO_OTYPER_OT11
            |GPIO_OTYPER_OT12
            |GPIO_OTYPER_OT13
            |GPIO_OTYPER_OT14
            |GPIO_OTYPER_OT15;
    GPIOF->OTYPER  = (GPIOF->OTYPER&~mAND)|mOR;

    // Configure pins in GPIOG
    // 0/A10 1/A11 4/BA0 5/BA1 8/CLK 15/CAS

    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOGEN;

    mAND =   GPIO_AFRL_AFRL0_Msk
            |GPIO_AFRL_AFRL1_Msk
            |GPIO_AFRL_AFRL4_Msk
            |GPIO_AFRL_AFRL5_Msk;
    mOR  =   (12<<GPIO_AFRL_AFRL0_Pos)
            |(12<<GPIO_AFRL_AFRL1_Pos)
            |(12<<GPIO_AFRL_AFRL4_Pos)
            |(12<<GPIO_AFRL_AFRL5_Pos);
    GPIOG->AFR[0]  = (GPIOG->AFR[0]&~mAND)|mOR;

    mAND =   GPIO_AFRH_AFRH0_Msk
            |GPIO_AFRH_AFRH7_Msk;
    mOR  =   (12<<GPIO_AFRH_AFRH0_Pos)
            |(12<<GPIO_AFRH_AFRH7_Pos);
    GPIOG->AFR[1]  = (GPIOG->AFR[1]&~mAND)|mOR;

    mAND =   GPIO_MODER_MODER0_Msk
            |GPIO_MODER_MODER1_Msk
            |GPIO_MODER_MODER4_Msk
            |GPIO_MODER_MODER5_Msk
            |GPIO_MODER_MODER8_Msk
            |GPIO_MODER_MODER15_Msk;
    mOR  =   GPIO_MODER_MODER0
            |GPIO_MODER_MODER1
            |GPIO_MODER_MODER4
            |GPIO_MODER_MODER5
            |GPIO_MODER_MODER8
            |GPIO_MODER_MODER15;
    GPIOG->MODER   = (GPIOG->MODER&~mAND)|mOR;

    mAND =   GPIO_OSPEEDR_OSPEEDR0_Msk
            |GPIO_OSPEEDR_OSPEEDR1_Msk
            |GPIO_OSPEEDR_OSPEEDR4_Msk
            |GPIO_OSPEEDR_OSPEEDR5_Msk
            |GPIO_OSPEEDR_OSPEEDR8_Msk
            |GPIO_OSPEEDR_OSPEEDR15_Msk;
    mOR  =   GPIO_OSPEEDR_OSPEEDR0
            |GPIO_OSPEEDR_OSPEEDR1
            |GPIO_OSPEEDR_OSPEEDR4
            |GPIO_OSPEEDR_OSPEEDR5
            |GPIO_OSPEEDR_OSPEEDR8
            |GPIO_OSPEEDR_OSPEEDR15;
    GPIOG->OSPEEDR = (GPIOG->OSPEEDR&~mAND)|mOR;

    mAND =   GPIO_PUPDR_PUPDR0_Msk
            |GPIO_PUPDR_PUPDR1_Msk
            |GPIO_PUPDR_PUPDR4_Msk
            |GPIO_PUPDR_PUPDR5_Msk
            |GPIO_PUPDR_PUPDR8_Msk
            |GPIO_PUPDR_PUPDR15_Msk;
    mOR  =   GPIO_PUPDR_PUPDR0
            |GPIO_PUPDR_PUPDR1
            |GPIO_PUPDR_PUPDR4
            |GPIO_PUPDR_PUPDR5
            |GPIO_PUPDR_PUPDR8
            |GPIO_PUPDR_PUPDR15;
    GPIOG->PUPDR   = (GPIOG->PUPDR&~mAND)|mOR;

    mAND =   GPIO_OTYPER_OT0_Msk
            |GPIO_OTYPER_OT1_Msk
            |GPIO_OTYPER_OT4_Msk
            |GPIO_OTYPER_OT5_Msk
            |GPIO_OTYPER_OT8_Msk
            |GPIO_OTYPER_OT15_Msk;
    mOR  =   GPIO_OTYPER_OT0
            |GPIO_OTYPER_OT1
            |GPIO_OTYPER_OT4
            |GPIO_OTYPER_OT5
            |GPIO_OTYPER_OT8
            |GPIO_OTYPER_OT15;
    GPIOG->OTYPER  = (GPIOG->OTYPER&~mAND)|mOR;

    // Configure pins in GPIOH
    // 3/CS 5/WE

    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOHEN;

    mAND =   GPIO_AFRL_AFRL3_Msk
            |GPIO_AFRL_AFRL5_Msk;
    mOR  =   (12<<GPIO_AFRL_AFRL3_Pos)
            |(12<<GPIO_AFRL_AFRL4_Pos);
    GPIOH->AFR[0]  = (GPIOH->AFR[0]&~mAND)|mOR;

    mAND =   0;
    mOR  =   0;
    GPIOH->AFR[1]  = (GPIOH->AFR[1]&~mAND)|mOR;

    mAND =   GPIO_MODER_MODER3_Msk
            |GPIO_MODER_MODER4_Msk;
    mOR  =   GPIO_MODER_MODER3
            |GPIO_MODER_MODER5;
    GPIOH->MODER   = (GPIOH->MODER&~mAND)|mOR;

    mAND =   GPIO_OSPEEDR_OSPEEDR3_Msk
            |GPIO_OSPEEDR_OSPEEDR5_Msk;
    mOR  =   GPIO_OSPEEDR_OSPEEDR3
            |GPIO_OSPEEDR_OSPEEDR5;
    GPIOH->OSPEEDR = (GPIOH->OSPEEDR&~mAND)|mOR;

    mAND =   GPIO_PUPDR_PUPDR3_Msk
            |GPIO_PUPDR_PUPDR4_Msk;
    mOR  =   GPIO_PUPDR_PUPDR3
            |GPIO_PUPDR_PUPDR5;
    GPIOH->PUPDR   = (GPIOH->PUPDR&~mAND)|mOR;

    mAND =   GPIO_OTYPER_OT3_Msk
            |GPIO_OTYPER_OT5_Msk;
    mOR  =   GPIO_OTYPER_OT3
            |GPIO_OTYPER_OT5;
    GPIOH->OTYPER  = (GPIOH->OTYPER&~mAND)|mOR;

}
#else

static const GPIO_PinConfiguration configtable[] = {
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
   {  GPIOC,   3,       12  },       //     CLKE
   {  GPIOH,   3,       12  },       //     CS
   {  GPIOE,   0,       12  },       //     DQM0
   {  GPIOE,   1,       12  },       //     DQM1
//
   {     0,    0,          0  }
};

#endif

/**
 *  @brief  FMC Commands
 *
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
 *  @note   It must be greater than 41
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
 *  @brief  Mode register format for MT48LC4M32B2
 *
 *  @note   Format according datasheet
 *
 *  Field   |   Description
 *  --------|-------------------
 *   2-0    | Burst length
 *     3    | Burst type
 *   6-4    | CAS Latency
 *   8-7    | Operation mode
 *     9    | Write burst mode
 * 12-10    | Reserved
 *
 * Burst length encoding
 *
 *    M2-0 |  Burst length
 *  -------|--------
 *    000  |     1
 *    001  |     2
 *    010  |     4
 *    011  |     8
 *    100  |     -
 *    101  |     -
 *    110  |     -
 *    111  |     -
 *
 * Burst type
 *
 *    M3   | Burst type
 *  -------|---------------
 *     0   | Sequential
 *     1   | Interleaved
 *
 * CAS Latency
 *
 *    M6-4 | CAS Latency
 *  -------|----------------
 *    000  |     -
 *    001  |     1
 *    010  |     2
 *    011  |     3
 *    100  |     -
 *    101  |     -
 *    110  |     -
 *    111  |     -
 *
 * Operation mode
 *
 *   M8-7  |  Operation mode
 *     00  |  Standard
 *  others |  Reserved
 *
 * Write Burst Mode
 *
 *    M9   |  Write burst mode
 *  -------|----------------------
 *    0    | Programmed Burst Mode
 *    1    | Single Location Access
 *
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
SendCommand(uint32_t command, uint32_t parameters, uint32_t timeout) {

    parameters &= ~(FMC_SDCMR_MODE_Msk|FMC_SDCMR_CTB1|FMC_SDCMR_CTB2);
    FMC_Bank5_6->SDCMR=(command<<FMC_SDCMR_MODE_Pos)|FMC_SDCMR_CTB1|parameters;

    while( (FMC_Bank5_6->SDSR&FMC_SDSR_BUSY) &&(timeout>0) ) {}

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
 *  @note   The SDRAM is in SDRAM Bank 1 (FMC Bank 5) and is configured to run at half the speed
 *          of the core.
 *
 *  @note   Autorefresh = 1 sempre?
 */
#if 0
SDRAM_Timing.LoadToActiveDelay    = 2;
  SDRAM_Timing.ExitSelfRefreshDelay = 6;
  SDRAM_Timing.SelfRefreshTime      = 4;
  SDRAM_Timing.RowCycleDelay        = 6;
  SDRAM_Timing.WriteRecoveryTime    = 2;
  SDRAM_Timing.RPDelay              = 2;
  SDRAM_Timing.RCDDelay             = 2;

  hsdram.Init.SDBank             = FMC_SDRAM_BANK1;
  hsdram.Init.ColumnBitsNumber   = FMC_SDRAM_COLUMN_BITS_NUM_8;
  hsdram.Init.RowBitsNumber      = FMC_SDRAM_ROW_BITS_NUM_12;
  hsdram.Init.MemoryDataWidth    = SDRAM_MEMORY_WIDTH;
  hsdram.Init.InternalBankNumber = FMC_SDRAM_INTERN_BANKS_NUM_4;
  hsdram.Init.CASLatency         = FMC_SDRAM_CAS_LATENCY_2;
  hsdram.Init.WriteProtection    = FMC_SDRAM_WRITE_PROTECTION_DISABLE;
  hsdram.Init.SDClockPeriod      = SDCLOCK_PERIOD;
  hsdram.Init.ReadBurst          = FMC_SDRAM_RBURST_ENABLE;
  hsdram.Init.ReadPipeDelay      = FMC_SDRAM_RPIPE_DELAY_0;
#endif

#define SDRAM_RPIPE             0
#define SDRAM_RBURST            1
#define SDRAM_SDCLK             2
#define SDRAM_WP                0
#define SDRAM_NB                0
#define SDRAM_MWID              1
#define SDRAM_NR                1
#define SDRAM_NC                0

#define SDRAM_TRCD              1
#define SDRAM_TRP               1
#define SDRAM_TWR               1
#define SDRAM_TRC               6
#define SDRAM_TRAS              3
#define SDRAM_TXSR              5
#define SDRAM_TMRD              1

void
ConfigureFMCSDRAM(void) {

    /* Configure and enable SDRAM bank1 */
    FMC_Bank5_6->SDCR[0]  = (0<<FMC_SDCR1_RPIPE_Pos)      // No HCLK Clock delay after CL
                           |(1<<FMC_SDCR1_RBURST_Pos)     // Burst read mode
                           |(2<<FMC_SDCR1_SDCLK_Pos)      // f_SDCLK = f_HCLK/2
                           |(0<<FMC_SDCR1_WP_Pos)         // No write protection
                           |(0<<FMC_SDCR1_CAS_Pos)        // CL=2
                           |(0<<FMC_SDCR1_NB_Pos)         // 4 banks
                           |(1<<FMC_SDCR1_MWID_Pos)       // Mem width = 16 bits
                           |(1<<FMC_SDCR1_NR_Pos)         // Rows bits = 12 bits
                           |(0<<FMC_SDCR1_NC_Pos);        // Col bits = 8 bits

    FMC_Bank5_6->SDTR[0]  = (SDRAM_TRCD<<FMC_SDTR1_TRCD_Pos)       // Row to Column delay = 2 cycles
                           |(SDRAM_TRP<<FMC_SDTR1_TRP_Pos)        // Row precharge delay = 2 cycles
                           |(SDRAM_TWR<<FMC_SDTR1_TWR_Pos)        // Recovery delay = 2 cycles
                           |(SDRAM_TRC<<FMC_SDTR1_TRC_Pos)        // Row cycle delay = 6 cycles
                           |(SDRAM_TRAS<<FMC_SDTR1_TRAS_Pos)       // Self refresh timer = 4 cycles
                           |(SDRAM_TXSR<<FMC_SDTR1_TXSR_Pos)       // Exit self refresh delay = 6 cycles
                           |(SDRAM_TMRD<<FMC_SDTR1_TMRD_Pos);      // Load mode to active delay = 2 cycles

    /*
     * SDRAM initialization sequence
     */

    /* Clock enable command */
    SendCommand(SDRAM_COMMAND_CLOCKCONFIGENABLE,0x0000,DEFAULT_TIMEOUT);

    SmallDelay(1000);       // 100 us, maybe systick is better */

    /* PALL command */
    SendCommand(SDRAM_COMMAND_PALL,0x0000,DEFAULT_TIMEOUT);

    /* Auto refresh command */
    SendCommand(SDRAM_COMMAND_AUTOREFRESH,(SDRAM_AUTOREFRESH<<FMC_SDCMR_NRFS_Pos),DEFAULT_TIMEOUT);

    /* MRD register program */
    SendCommand(SDRAM_COMMAND_LOADMODE,(SDRAM_MODE<<FMC_SDCMR_MRD_Pos),DEFAULT_TIMEOUT);

    /* Set refresh count */
    FMC_Bank5_6->SDRTR = (FMC_Bank5_6->SDRTR&~(FMC_SDRTR_COUNT_Msk))
                        |(SDRAM_REFRESH<<FMC_SDRTR_COUNT_Pos);

    /* Disable write protection */
    FMC_Bank5_6->SDCR[0] &= ~(FMC_SDCR1_WP);


}


/**
 * @brief   SDRAM Init
 *
 * @note    Initializes the FMC unit and configure access to a SDRAM
 *
 * @note    HCLK must be 200 MHz!!!!
 */
void
SDRAM_Init(void) {

    if( SystemCoreClock != SDRAMCLOCKFREQUENCY )
        return;

    // Enable clock for FMC
    RCC->AHB3ENR |= RCC_AHB3ENR_FMCEN;

#ifdef USE_FAST_INITIALIZATION
    /* Configure pins port by port*/
    ConfigureFMCSDRAMPins();
#else
    /* Configure pins from table*/
    GPIO_ConfigureAlternateFunctionMultiple(configtable);
#endif

    ConfigureFMCSDRAM();

}

