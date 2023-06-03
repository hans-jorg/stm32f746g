Using the Touch Panel
=====================

Introduction
------------

The board has a 4.3" LCD display with Capacitive Touch Panel (CTP). 

The STM32F646 has a LCD control interface and an accelerator, 
called ChromeART (DMA2D).

The display
-----------

It is produced by Rocktech under the code RK043FN48H-CT672B. Its resolution 
is 480x272 and can  display 16777216 colors (RGB888 interface).
The board uses a customized version of the commercial  display (It uses other
cable/connector).
 
The display builtin LCD controller is the OTA5180A and the CTP controller is
a FT5336GQQ.

> NOTE: There is a HT043c in the schematics!!!!

> NOTE: This controller is considered obsolete and should not be used for new projects!!!!


The FT5336GQQ True Multi-Touch Capacitive Touch Panel Controller
----------------------------------------------------------------

The controller is integrated into the display.

The FT5336GQQ is a single chip capacitive touch panel controller with the
following features:

* Multi-touch (5 points) capability
* I2C interface
* Support to screens up to 6,1" diagonal
* Built-in MCU with DSP accelerator
* Built-in LDO

The FT5336GQQ has the following interface pins:

* SDA: I2C bidirectional data line
* SCL: I2C clock input
* INT: interrupt output

The I2C interface works with a SCL frequency in the range 10-400 KHz.

It has a general input/output pin too.


Capacitive Touch Panel (CTP) interface
--------------------------------------


The LCD display has a Capacitive Touch Display (CTP), controlled thru a FT5336 touch controller from newhaven []. There are versions with a GT911 touch controller. Somewhere a ht043c controller is also mentioned. The FT5336 datasheet does not describe its registers but they are similar to the ones in the FT5306.
 
 It uses an I2C interface shared with the Audio WM8994ECS/R  device.
 
| Board Signal    | Signal | Pin  | AF | Interface| Description            |
|-----------------|--------|------|---------------|------------------------|
|  LCD\_SCL       | SCL    | PH7  |  4 | I2C3\_SCL| Shared with AUDIO\_SCL |
|  LCD\_SDA       | SDA    | PH8  |  4 | I2C3\_SDA| Shared with AUDIO\_SDA |
|  LCD_INT        | INT    | PI13 |  0 | GPIOI13  | Interrupt              |
|  LCD\_RST       | RST    | NRST |  - |     -    | Global reset           |


As described in the schematics the I2C interface of the CTP uses the default address 01110000 (=0x70). The Audio WM8994ECS/R uses either the address 0x00110100 (=0x34), as in the Discovery board or 0x00110110 (=0x36) according the pin CS/ADDR, low or high, respectively.

I2C interface
-------------

The MCU has four I2C units: I2C1, I2C2, I2C3 and I2C4.

Its main features are:

* Slave/Master
* 400 kHz, 400 kHz, 1 MHz rates
* 7 and 10 bit addressing
* DMA support
* SMB compatibility mode


The I2C interface needs two clock signals

* I2CCLK: Kernel clock
* PCLK: Peripheral clock


The I2CCLK clock source for the I2C units are independent and are set by the
 I2CxSEL fields in the DCKCFGR2 register.

* APB1CLK
* System Clock
* HSI Clock

Enabling and disabling the I2C peripheral is done by:

* Configuring and enabling the clock in the clock controller (RCC)
* Configuring the noise filters
* Enabling the I2C by setting PE bit in the I2C_CR1 register

APB1 is derived from the HCLK signal thru the APB1 prescaler, that can have 
the values 1,2,4,8 or 16.

Restrictions for I2CCLK (See RM 30.4.2):

$$ t_{I2CCLK} =  1 / f_{I2CLK} $$
$$ t_{I2CCLK} < t_{HIGH} $$
$$ t_{I2CCLK} <= (tT_{LOW-t_{filters})/4  $$
$$ t_{filters} = 260 ns + DNF*t_{I2CCLK}  $$
$$ t_{PCLK} = 4/3 * t_{SCL}  $$

The I2C peripheral clock is enabled by the PE bit in I2C_CR1 register.

The minimum I2CCLK frequency is given in Table 74 of datasheet.

| Velocity    |   Rate        | Analog Filter|  Digital Filter | Clock frequency 
|-------------|---------------|--------------|-----------------|---------------
| Standard    |     100 kHz   |              |                 |    ~= 2 MHz
| Fast        |     400 kHz   |      ON      |      0          |    ~= 10 MHz
| Fast        |     400 kHz   |      OFF     |      1          |    ~= 9 MHz
| Fast Plus   |       1 MHz   |      ON      |      0          |    ~= 22.5 MHz
| Fast Plus   |       1 MHz   |      OFF     |      1          |    ~= 16 MHz


There are three filters alternatives for I2C signals:

* Analog filter: Default. 
* Digital filter: Configured by DNF field in the I2C_CR1.
* None: Analog filter can be turned off by setting ANFOFF bit and the digital 
  filter by setting DNF to 0000.


Configuration
-------------

The HSE clock signal is generated from an external 25 MHz crystal oscillator. 
To get a 200 MHz clock signal needed for the SDRAM, it must be divided by 25, 
multiplied by 400 and then divided by 2, since the minimum for Q is 2.

$$ f_{} = (( f_{HSI}/M )*N / Q ) $$

| Input |  Freq |  VCOIN |  M  |   N   |   Q   | SYSCLK |
|-------|-------|--------|-----|-------|-------|--------|
|  HSE  | 25 MHz|  1 MHz | 25  |  400  |   2   | 200 MHz|
|  HSE  | 25 MHz|  1 MHz | 25  |  432  |   2   | 216 MHz|
|  HSI  | 16 MHz|  2 MHz |  8  |  200  |   2   | 200 MHz|
|  HSI  | 16 MHz|  2 MHz |  8  |  216  |   2   | 216 MHz|

The HCLK can be divided by the AHB PRESC prescaler to generate de HCLK 
(=SystemCoreClock). It is specified by the HPRE field in RCC_CFGR.

The input for the other PLL generator is then 1 MHz.


The LCD_CLK frequency has a 12 MHz maximal value. It is generated from
PLLSAIR clock signal thru a divider that can have the value 2, 4, 6 or 8.

Working backwards

|  VCOIN    |  N    |  VCO   |  /R  |   PLLSAIR | /PLLDIVR  |  LCD_CLK
|-----------|-------|--------|------|-----------|-----------|---------------
|   1 MHz   |   48  |  48 MHz|   2  |   24 MHz  |      2    |  12 MHz
|   1 MHz   |   96  |  96 MHz|   2  |   48 MHz  |      4    |  12 MHz
|   1 MHz   |  192  | 192 MHz|   2  |   96 MHz  |      8    |  12 MHz
|   1 MHz   |  384  | 384 MHz|   2  |  192 MHz  |     16    |  12 MHz
|   1 MHz   |   72  |  72 MHz|   3  |   24 MHz  |      2    |  12 MHz
|   1 MHz   |  144  | 144 MHz|   3  |   48 MHz  |      4    |  12 MHz
|   1 MHz   |  288  | 288 MHz|   3  |   96 MHz  |      8    |  12 MHz
|   1 MHz   |   72  |  96 MHz|   4  |   24 MHz  |      2    |  12 MHz
|   1 MHz   |  192  | 192 MHz|   4  |   48 MHz  |      4    |  12 MHz
|   1 MHz   |  384  | 384 MHz|   4  |   96 MHz  |      8    |  12 MHz


The combination of parameters N, R, PLLDIVR depends on the other uses of the
PLLSAI signals, SAI1 and SAI2 clock signals, used by DMA, Serial Audio
Interface (SAI) 1 and 2. 

The I2C can be configured to use the HSI or the SystemCoreClock clock. 
The advantage of using HSI is that it is fixed. 
The generation of the clock signal for the I2C is a complex process and there
is a configuration tool and a Application Note about it [8]. A configurator for
i2C is also  integrated into STM32CubeMX Initialization Code Generator [9].
 
Configuration generated by STM32 CubeMX for a 16 MHz I2C clock signal with 
analog filter and digital filters using DNF = 1 or 2.

| I2C mode     | velocity  | No filter | Analog Filter | Digital Filter(=1)| Digital Filter(=2)|
|--------------|-----------|-----------|---------------|-------------------|-------------------|
| Standard     |  100 kpbs | 0x00503D5A|   0x00503D58  |   0x00503C59      |   0x00503B58      |
| Fast mode    |  400 Kbps | 0x00300718|   0x00300617  |   0x00300617      |   0x00300912      |
| Fast mode +  | 1000 Kpbs | 0x00200205|   0x00200105  |   0x00200004      |   0x00200003      |

> NOTE: The generated values contradict the Table 74 of datasheet.

Using a self developed python script, the table is a little bit different!!!

| I2C mode     | velocity  | No filter | Analog Filter | Digital Filter(=1)| Digital Filter(=2)|
|--------------|-----------|-----------|---------------|-------------------|-------------------|
| Standard     |  100 kpbs | 0x00603d49|   0x00603d48  |   0x00603c48      |   0x00603b47      |
| Fast mode    |  400 Kbps | 0x00300712|   0x00300611  |   0x00060611      |   0x00300510      |
| Fast mode +  | 1000 Kpbs | 0x00200205|   0x00200105  |   0x00200004      |   0x00200003      |



I2CCLK frequency 
   
I2CCLK Clock is configured in the RCC->DCKCFGR2 register.
       They are I2CxSEL fields, one for each I2C unit
 
Clock sources

 | Source         | I2CxSEL |
 |----------------|---------|
 |  APB1 (PCLK1)  |   00    |
 |  SYSCLK        |   01    |
 |  HSI           |   10    |

Minimal frequencies

| Mode                | Analog filter |  DNF = 1 |
|---------------------|---------------|----------|
| Standard mode       |    2 MHz      |   2 MHz  |
| Fast mode           |   10 MHz      |   9 MHz  |
| Fast plus mode      |   22.5 MHz    |  16 MHz  |

OBS: Using HSI (=16 MHz) as filter, it is not possible to use Fast plus mode

From Table 182 of the STM32F746NG Datasheet

| Parameter     |  Standard  |   Fast     | Fast Plus  |
|---------------|------------|------------|------------|
|  PRESC        |      3     |       1    |       0    |
|  SCLL         |     0x13   |     0x9    |     0x4    |
|  SCLH         |     0xF    |     0x3    |     0x2    |
|  SDADEL       |     0x2    |     0x2    |     0x0    |
|  SCLDEL       |     0x4    |     0x3    |     0x2    |

All timing for 16 MHz (=HSI)
Generated by STM3CubeMX for a 16 MHz I2CCLK
  
The timing is set by
       PRESC (31-28:4 bits):   fPRESC = fI2CCLK/(PRESC+1)
       SCLDEL(23-20:4 bits):   tSCLDEL= tPRESC*(SCLDEL+1)
       SDADEL(19-10:4 bits):   tSDADEL= tPRESC*(SDADEL+1)
       SCLH  (15-8:8 bits):    tSCLH = (SCLH+1)*tPRESC
       SCLL  ( 7-0:8 bits):    tSCLL = (SCLL+1)*tPRESC
   
The restrictions are:
 
       SDADEL >= (tfmax+tHDDATmin-tAFmin-(DNF+3)*tI2CCLK)/(PRESC+1)*tI2CCLK
       SDADEL <= (tHDDATmax-fAFmax-(DNF+4)xtI2CCLK)/(PRESC+1)*tI2CCLK
       SCLDEL >= (trmax+tSUDAT,om)/(PRESC+1)*tI2CCLK-1
  
The I2C standard specifies the following parameters (RM Table 178)
   
| Speed     |tHDDATmin|tVDDATmax|tSUDATmin|trmax |tfmax|
|-----------|---------|---------|---------|------|-----|
| Standard  |    0    |   3.45  |  0.250  |  1   | 0.3 |
| Fast      |    0    |   0.9   |  0.100  |  0.3 | 0.3 |
| Fast plus |    0    |   0.45  |  0.050  |  0.12| 0.12|
| SMBUS     |    0.3  |     -   |  0.250  |  1   | 0.3 |
 
tAF (Maximum pulse width of spikes that are suppressed by the analog
       filter) is defined in the STM32F746NG datasheet

| Parameter |  Min  |  Max  |
|-----------|-------|-------|
|  tAF (ns) |   50  |  150  |

Table with values for TIMINGR generated by STM32CubeMX

| Speed     |   None     |   Analog   | Digital=1  | Digital=2  |
|-----------|------------|------------|------------|------------|
|  100 KHz  | 0x00303D5D | 0x00303D5B | 0x00303C5C | 0x00303B5B |
|  400 KHz  | 0x0010071B | 0x0010061A | 0x0010061A | 0x00100519 |
| 1000 KHz  | 0x00000208 | 0x00000107 | 0x00000107 | 0x00000006 |

TIMINGR = 0x00303D5D means
                 PRESC=0   -> fPRESC = fI2CCLK/(PRESC+1)
                 SCLDEL=3
                 SDADEL=3
                 SCLH=3D   = 61
                 SCLL=5D   = 91
Table 182 of RM shows another set of values for a 16 MHz clock.
      It is not clear which kind of filter is beeing used!

| Speed          | TIMINGR     | PRESC | SCLDEL | SDADEL | SCLH | SCLL |
|----------------|-------------|-------|--------|--------|------|------|
|      100 KHz   | 0x30420F13  |  0x3  |   0x4  |   0x2  | 0x0F | 0x13 | 
|      400 KHz   | 0x10320309  |  0x1  |   0x3  |   0x2  | 0x03 | 0x09 |
|     1000 KHz   | 0x00200204  |  0x0  |   0x2  |   0x0  | 0x02 | 0x04 |
  

Configuration for STM32F746G Discovery Board
-------------------------------------------- 

From the datasheet and board schematics
  
|  I2C   |    SCL           |           SDA      | Conflicts                        |
|--------|------------------|--------------------|----------------------------------|
|  I2C1  |  PB6 *PB8*       |  PB7 *PB9*         | QSPI, -, VCP, -                  |
|  I2C2  |  PB10 PF1 PH4    |  PB11 PF0 PH5      | ULPI, FMC, ULPI, ULPI, FMC, ULPI |
|  I2C3  |  PA8 *PH7*       |  PC8 *PH8*         | (ARD, uSD), -                    |
|  I2C4  |  PD12 PF14 PH11  |  PD13 PF15 PH12    | QSPI, LCD, ULPI, QSPI, FMC, DCMI |
 
* ARD = Arduino interface


I2C1 at PB8 and PB9 used for EXT I2C (Arduino connectors), DCMI, ARD
I2C3 at PH7 and PH8 used for LCD Touch and AUDIO I2C
Other I2C has pin usage conflicts

All SCL and SDA pins must be configured as 
                    ALTERNATE FUNCTION (=4)
                    OPEN DRAIN
                    HIGH SPEED
                    PULL-UP


The I2C Controller for touchscreen
----------------------------------


The I2C Controller I2C3 is shared between 

* The FT5336 Touch Controller (inside the LCD panel)
* The WM8994ECS/R Audio Controller
  
The I2C controller must be configured as:

* Own Address 1 = 0
* Own Address 2 = 0
* Addressing Mode = 7 bits
* Dual Address Mode = Disabled
* General Call Mode = Disabled
* No Stretch Mode = Disabled
* I2C speed:  Fast I2C (400 KHz)
* FT5336 I2C address: 0x70
* WM8994 I2C Address: 0x34
* I2C SCL Pin must use AF = 5, Mode = OD, No Pull, Speed = Fast,


>NOTE: Int the STM32F7Cube, the speed is set to 100 KHz



### 

### Initialization Procedure in STM32F7Cube

  ft5336_Init                                 ft5336.c
    Delay
    I2C_InitializeRequired
      TS_IO_Init                              stm32f746g_discovery.c
        I2Cx_Init( &hI2cAudioHandler )
          Timing = 0x40912732                 
          OwnAddress1 = 0
          OwnAddress2 = 0
          AddressingMode = 7BIT
          DualAddressMode = Disabled
          GeneralCallMode = Disabled
          NoStretchMode   = Disabled
          I2Cx_MspInit
            DISCOVERY_AUDIO_I2Cx_SCL_SDA_GPIO_CLK_ENABLE
              __HAL_RCC_GPIOH_CLK_ENABLE       stm32f7xx_hal_rcc_ex.h
                RCC->AHB1ENR |= RCC_AHB1ENR_GPIOHEN;
                uint32_t tmp = RCC->AHB1ENR;
                tmp = tmp;
            Pin = SCL
            Mode = GPIO_MODE_AF_OD         Alternate Function/Open Drain
            Pull = No Pull
            Speed = Fast                        stm32_hal_legacy.h
              GPIO_SPEED_FREQ_HIGH              stm32_hal_legacy.h
            Alternate Function = GPIO_AF4_I2C3
            Pin = SDA
            ---Idem---
            DISCOVERY_AUDIO_I2Cx_CLK_ENABLE
              __HAL_RCC_I2C3_CLK_ENABLE
                RCC->APB1ENR |= RCC_APB1ENR_I2C3EN;
                uint32_t tmp = RCC->APB1ENR;
                tmp = tmp;
            DISCOVERY_AUDIO_I2Cx_FORCE_RESET
              __HAL_RCC_I2C3_FORCE_RESET
                RCC->APB1RSTR |= (RCC_APB1RSTR_I2C3RST);
            DISCOVERY_AUDIO_I2Cx_RELEASE_RESET
              __HAL_RCC_I2C3_RELEASE_RESET
                RCC->APB1RSTR &= ~(RCC_APB1RSTR_I2C3RST)
            HAL_NVIC_SetPriority(DISCOVERY_AUDIO_I2Cx_EV_IRQn, 0x0F, 0)
              I2C3_EV_IRQn 72
              ????
            HAL_NVIC_EnableIRQ(DISCOVERY_AUDIO_I2Cx_EV_IRQn);
              I2C3_EV_IRQn 72
              ????
            HAL_NVIC_SetPriority(DISCOVERY_AUDIO_I2Cx_ER_IRQn, 0x0F, 0)
              I2C3_ER_IRQn 73
              ????
            HAL_NVIC_EnableIRQ(DISCOVERY_AUDIO_I2Cx_ER_IRQn);
              I2C3_ER_IRQn 73
              ????
          HAL_I2C_Init
            Set Callbacks
            HAL_I2C_MspInit
            Disable I2C
            





Summarizing

  Prepar     
    uint32_t timing =  ((uint32_t)0x40912732);
  GPIO CLK Enable
    RCC->AH1xxENR |= RCC_AH1xxENR_GPIOH;

  Configure Pins for SCL and SDA
  I2C CLK Enable
    RCC->APB1ENR |= RCC_APB1ENR_I2C3EN;
    uint32_t x = RCC->APB1ENR&RCC_APB1ENR_I2C3EN;
  Force Reset
  Release Reset
  Set Interrupt Priority for EV (?)
  Enable IRQ for EV
  Set Interrupt Priority for ER (?)
  Enable IRQ for ER (?)


The FT5336 Touch Controller
---------------------------


It uses an I2C interface.

### I2C protocol

>> No mixed READ/WRITE I2C sequences are used.

Writing N bytes to the Touch Controller

    S SLAVEADDRESS WRITE ACK ADDRESS ACK DATA1 ACK DATA2 ACK ... DATAN ACK STOP

Set data address

    S SLAVEADDRESS WRITE ACK ADDRESS ACK STOP

Read data from the Touch controller starting from address set as above

    S SLAVEADDRESS READ ACK DATA1 ACK DATA2 ACK ... DATAN ACK STOP


### FT5336 registers

>> This table was obtained from the FT5306 datasheet.

| Address | Name               | Access | Contents                                 |
|---------|--------------------|--------|------------------------------------------|
|   00    | DEVICE_MODE        |   RW   | Device mode (6:4)                        |
|   01    | GEST_ID            |   R    | Gesture ID (7:0)                         |
|   02    | TD_STATUS          |   R    | # touch points (3:0)                     |
|   03    | TOUCH1_XH          |   R    | Touch 1 flag and X position (higher bits)|
|   04    | TOUCH1_XL          |   R    | Touch 1 X position (low bits)            |
|   05    | TOUCH1_YH          |   R    | Touch 1 ID and Y position (higher bits)  |
|   06    | TOUCH1_YL          |   R    | Touch 1 Y position (low bits)            |
|   07    |  Reserved          |        |                                          |
|   08    |  Reserved          |        |                                          |
|   09    | TOUCH2_XH          |   R    | Touch 2 flag and X position (higher bits)|
|   10    | TOUCH2_XL          |   R    | Touch 2 X position (low bits)            |
|   11    | TOUCH2_YH          |   R    | Touch 2 ID and Y position (higher bits)  |
|   12    | TOUCH2_YL          |   R    | Touch 2 Y position (low bits)            |
|   13    |  Reserved          |        |                                          |
|   14    |  Reserved          |        |                                          |
|   15    | TOUCH3_XH          |   R    | Touch 3 flag and X position (higher bits)|
|   16    | TOUCH3_XL          |   R    | Touch 3 X position (low bits)            |
|   17    | TOUCH3_YH          |   R    | Touch 3 ID and Y position (higher bits)  |
|   18    | TOUCH3_YL          |   R    | Touch 3 Y position (low bits)            |
|   19    |  Reserved          |        |                                          |
|   20    |  Reserved          |        |                                          |
|   21    | TOUCH4_XH          |   R    | Touch 4 flag and X position (higher bits)|
|   22    | TOUCH4_XL          |   R    | Touch 4 X position (low bits)            |
|   23    | TOUCH4_YH          |   R    | Touch 4 ID and Y position (higher bits)  |
|   24    | TOUCH4_YL          |   R    | Touch 4 Y position (low bits)            |
|   25    |  Reserved          |        |                                          |
|   26    |  Reserved          |        |                                          |
|   27    | TOUCH5_XH          |   R    | Touch 5 flag and X position (higher bits)|
|   28    | TOUCH5_XL          |   R    | Touch 5 X position (low bits)            |
|   29    | TOUCH5_YH          |   R    | Touch 5 ID and Y position (higher bits)  |
|   30    | TOUCH5_YL          |   R    | Touch 5 Y position (low bits)            |
|   31    |  Reserved          |        |                                          |
|   32    |  Reserved          |        |                                          |
|  33-127 |  Reserved          |        |                                          |
|  128    | ID_G_THGROUP       |   RW   | Valid touch detect threshold             |
|  129    | ID_G_THPEAK        |   RW   | Valid touching peak detect threshold     |
|  130    | ID_G_THCAL         |   RW   | The threshold when calculating the focus |
|  131    | ID_G_THWATER       |   RW   | The threshold when there is surface water|
|  132    | ID_G_THTEMP        |        | The threshold of temperature compensation|
|  133    |  Reserved          |        |                                          |
|  134    | ID_G_CTRL          |   RW   | Power control mode                       |
|  135    | ID_G_TIME_MONITOR  |   RW   | The timer of entering monitor status     |
|  136    | ID_G_PERIODACTIVE  |   RW   | Period active                            |
|  137    | ID_G_PERIOD_MONITOR|   RW   | The timer of entering idle               |
| 138-159 |  Reserved          |        |                                          |
|  160    | ID_G_AUTO_CLB_MODE |   RW   | Auto calibration mode                    |
|  161    | ID_G_LIB_VERSION_H |   R    | Firmware Library Version Higher bits     |
|  162    | ID_G_LIB_VERSION_L |   R    | Firmware Library Version Lower bits      |
|  163    | ID_G_CIPHER        |   R    | Chip vendor ID                           |
|  164    | ID_G_MODE          |   R    | The interrupt status to host             |
|  165    | ID_G_PMODE         |   R    | Power Consume Mode                       |
|  166    | ID_G_FIRMID        |   R    | Firmware ID                              |
|  167    | ID_G_STATE         |   R    | Running State                            |
|  168    | ID_G_FT5201ID      |   R    | CTPM Vendor ID                           |
|  169    | ID_G_ERR           |   R    | Error code                               |
|  170    | ID_G_CLB           |   R    | Configure TD during calibration          |
|  171    |  Reserved          |        |                                          |
|  172    |  Reserved          |        |                                          |
|  173    |  Reserved          |        |                                          |
|  174    | ID_G_B_AREA_TH     |   RW   | The threshold of big area                |
| 175-253 |  Reserved          |        |                                          |
|  254    | LOG_MSG_CNT        |   R    | The log MSG Count                        |
|  255    | LOG_CUR_CHA        |   R    | Current char of log message              |
References
----------
 
1. [STM32F75xxx and STM32F74xxx advanced Arm ® -based 32-bit MCUs Reference Manual - RM0385](https://www.st.com/resource/en/reference_manual/dm00124865-stm32f75xxx-and-stm32f74xxx-advanced-arm-based-32-bit-mcus-stmicroelectronics.pdf)
2. [STM32F746NG Data sheet](https://www.st.com/resource/en/datasheet/stm32f746ng.pdf)
3. [AN4861 - LCD-TFT display controller (LTDC) on STM32 MCUs](https://www.st.com/resource/en/application_note/dm00287603-lcdtft-display-controller-ltdc-on-stm32-mcus-stmicroelectronics.pdf)
4. [RK043FN48H-CT672B Datasheet](https://mikrocontroller.bplaced.net/wordpress/wp-content/uploads/2018/01/RK043FN48H-CT672B-V1.0.pdf)
5. [OTA5180A Datasaheet](https://www.newhavendisplay.com/resources_dataFiles/datasheets/LCDs/OTA5180A.pdf)
6. [FTA5336GQQ Datasheet](https://www.newhavendisplay.com/resources_dataFiles/datasheets/touchpanel/FT5336.pdf)
7. [A little about graphics subsystem internals on microcontrollers](https://alexkalmuk.medium.com/a-little-about-graphics-subsystem-internals-on-microcontrollers-d952cfd0966a)
8. [Source for FT5336 driver](https://os.mbed.com/teams/ST/code/BSP_DISCO_F746NG/file/c9112f0c67e3/ft5336.h/)
9. [Info about RK043FN66HS-CTG](https://community.st.com/s/question/0D50X0000B5H5uf/display-rk043fn66hsctg-from-rocktech)
10. [Source code for LCD](https://github.com/sumotoy/stm32746g_discovery_lcd)
11. [I2C timing configuration tool for STM32F3xx and STM32F0xx microcontrollers (AN4235)](https://www.st.com/en/embedded-software/stsw-stm32126.html)
12. [STM32CubeMX - STM32Cube initialization code generator ](https://www.st.com/en/development-tools/stm32cubemx.html)       
13. [STM32 I2C Configuration using Registers](https://controllerstech.com/stm32-i2c-configuration-using-registers/)
14. [Newhaven FT5306](https://newhavendisplay.com/content/specs/NHD-4.3-480272EF-ATXL-CTP.pdf)
15. [Newhaven FT5306 registers](https://support.newhavendisplay.com/hc/en-us/article_attachments/4414392094103/FT5x06_registers.pdf)