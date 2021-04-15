Using 200 MHz Clock
===================

Introduction
------------

The STM32F746 can use many clock sources to drive the core clock.

             HSE                    |\
    4-26 MHz ---+-------------------| \
                |                   |  |
                |  |\               |  |
                |  | \              |  |
                +--|  |  -------    |  | SYSCLK  -----------             HCLK
                   |  |--| PLL |----|  |----+----| /HPRE   |------------------
                +--|  |  -------    |  |    |    -----------
                |  | /              |  |    |
                |  |/               |  |    |    --------------          APB1
            HSI |                   |  |    |----| /PPRE1     |---------------
    16 MHz -----+-------------------|  /    |    --------------
                                    |/      |
                                            |    --------------         APB2
                                            -----| /PPRE2     |---------------
                                                 --------------



The are two direct clock sources:

* HSI - Internal RC oscillator (16 MHz)
* HSE - External crystal oscillator (4-26 MHz)

In the STM32F746G Discovery board, there is an external 25 MHz crystal oscillator driving the OSCIN input of the MCU.

The PLL unit can used either signal as a input to generate a clock signal up to 216 MHz. It uses three parameters to 
specify the output frequencies: the dividers M and P and the multiplier N.

The output frequency for the core clock is given by

$$ f_{VCOIN} = \frac{ f_{PLLIN} }{ M } $$
$$ f_{VCOOUT} = N f_{VCOIN} $$
$$ f_{PLLOUT} = \frac{ f_{VCOOUT} } { P }  $$

There are two additional dividers. The Q divider is used to generate other clock signals PLL48CLK for the USB unit. The R divider is not used on the Main PLL. There are two other PLL units: PLLSAI and PLLI2S that use the same input as the Main PLL and the same M divider.

There is restrictions for the values of M, N and P and for the frequencies $$ f_{VCOIN} $$ and $$ f_{OUT} $$.

$$ M = [2..63] $$
$$ N = [50..432]  $$
$$ P = { 2, 4, 6, 8 }  $$
$$ 2 <= f_{PLLIN} <= 26 MHz $$
$$ 1 <= f_{VCOIN} <= 2   MHz  $$
$$ 100 <= f_{PLLOUT} <= 216   MHz  $$

Care must be taken when changing the core frequency. When the frequency increases, the number of wait states for the flash memory must increase because there are speed limits. Higher supply voltages See Table 5 in section 3.3.2 of the Reference Manual [1].

VOS | max HCLK frequency w/o overdrive | max HCLK frequency with overdrive
----|----------------------------------|------------------------------------
01  |    144 MHz                       | 144 MHz
10  |    168 MHz                       | 180 MHz
11  |    180                           | 216 MHz

The overdrive can only be set for supply voltage above 2.1 V.



The clock HAL
-------------

* uint32_t SystemGetHPRE(void)
* uint32_t SystemSetHPRE(uint32_t hpre)
* uint32_t SystemGetAHBPrescaler(void)
* uint32_t SystemSetAHBPrescaler(uint32_t div)
* uint32_t SystemGetAPB1Prescaler(void)
* void     SystemSetAPB1Prescaler(uint32_t div)
* uint32_t SystemGetAPB2Prescaler(void)
* void     SystemSetAPB2Prescaler(uint32_t div)
* uint32_t SystemGetSYSCLKFrequency(void)
* uint32_t SystemGetCoreClock(void)
* uint32_t SystemGetAPB1Frequency(void)
* uint32_t SystemGetAPB2Frequency(void)
* uint32_t SystemGetAHBFrequency(void)
* uint32_t SystemGetHCLKFrequency(void)
* void     SystemConfigMainPLL(PLL_Configuration *pllconfig)
* void     SystemConfigSAIPLL( PLL_Configuration *pllconfig)
* void     SystemConfigI2SPLL( PLL_Configuration *pllconfig)
* uint32_t SystemSetCoreClock(uint32_t newsrc, uint32_t newdiv)
* uint32_t SystemSetCoreClockFrequency(uint32_t freq)

The routines use a struct to store the PLL configuration.

    typedef struct {
        uint32_t    source;      // CLOCKSRC_HSI or CLOCKSRC_HSE
        uint32_t    M;
        uint32_t    N;
        uint32_t    P;
        uint32_t    Q;              /* for other PLL units */
        uint32_t    R;
    } PLL_Configuration;

The symbols CLOCKSRC_HSI, CLOCKSRC_HSE and CLOCKSRC_PLL are used to specify the clock source.

These routines allows the configuration of all clock signals, including all PLL units.


References
----------

1. [STM32F75xxx and STM32F74xxx advanced Arm ® -based 32-bit MCUs Reference Manual - RM0385](https://www.st.com/resource/en/reference_manual/dm00124865-stm32f75xxx-and-stm32f74xxx-advanced-arm-based-32-bit-mcus-stmicroelectronics.pdf)

