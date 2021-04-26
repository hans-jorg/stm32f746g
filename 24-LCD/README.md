Using the LCD
=============

Introduction
------------

The board has a 4.3" LCD display with Capacitive Touch Panel (CTP). 

The STM32F646 has a LCD control interface and an accelerator, called ChromeART (DMA2D).





The display
-----------

It is produced by Rocktech under the code RK043FN48H-CT672B. Its resolution is 480x272 and can
 display 16777216 colors (RGB888 interface). The board uses a customized version of the commercial
 display (It uses other cable/connector).
 
The display builtin LCD controller is the OTA5180A and the CTP controller is a FT5336GQQ.

> NOTE: There is a HT043c in the schematics!!!!

> NOTE: This controller is considered obsolete and should not be used for new projects!!!!


The main features of the RK043FN48H are:

| Feature        | Description
|----------------|------------------
| Part No.       | RK043FN02H-CT
| Size           | 4.3"
| Resolution     | 480*272
| AA             | 95.04 x 53.86
| Outline Size   | 105.50 x 67.20 x 4.35
| Interface      | RGB for TFT,I2C for CTP
| Brightness     | 400(after TP)
| Contrast       | 500:1
| View angle     | 70/50/70/70
| Touch Panel    | Optional
| Remarks        | 4.3" TFT with CTP
| Application    | Mobile Devices,POS, GPS,black Box, Security,Evaluation Kits etc


The most importante parameters are:

Parameter         |	Min      |   Typ     |  Max    | Unit
------------------|----------|-----------|---------|----------
DCLK Frequency    |     5    |    9      |    12   | MHz
------------------|----------|-----------|---------|----------
HSYNC Period      |   490    |  531      |   605   | DCLK
HSYNC Display     |          |  480      |         | DCLK
HSYNC Back porch  |     8    |   43      |         | DCLK
HSYNC Front porch |     2    |    8      |         | DCLK
------------------|----------|-----------|---------|----------
VSYNC Period      |   275    |  288      |   335   | DCLK
VSYNC Display     |          | 272       |         | H
VSYNC Back porch  |     1    |  12       |         | H
VSYNC Front porch |     1    |  40       |         | H



LCD interface
--------------

The LCD interface in the MCU has the following features, among others:

* Double layer (Layer 2 over Layer 1)
* Color lookup table
* Support for ARGB8888, RGB888, RGB565, ARGB 1555, ARGB4444, L8, AL44, AL88.
* Color keying]
* Integrated PFC (pixel format converter)



The LCD interface is compound of the following signals

* LCD_R, LCD_G, LCD_B: 3x8 RGB signal
* LCD_CLK: clock signal for LCD display with a prescaler
* LCD_DE: data enable
* LCD_VSYNC: vertical synchonization signal
* LCD_HSYNC: horizontal synchonization signal


The LCD interface uses 

* HCLK: It is generated from the SYSCLK thru a prescaler (1-512 divider)
* APB2: It is generated from the SYSCLK (up to 216 MHz) signal
* LCD_CLK: It is generated from the R output of the PLLSAI PLL clock generator thru a divider

The PLLSAI is configured by the registers:

* PLLLSAICFGR: PLL configuration register
* DCKCFGR1: dedicated clocks configuration register

The PLLSAI generates a VCD clock signal from a VCO input signal, that is the same used by the the
Main PLL.

$$ f_{VCOIN} = f{HSI or HSI} / M where M={2,63} $$
$$ f_{VCOOUT} = f_{VCOIN}*N where  N={50..432} $$
$$ 100 <= f_{VCO} <= 432 MHz $$


The PLLSAI has three outputs from VCOUT clock signal.

$$ f_{PLLSAIP} = f_{VCOOUT}/P where R={ 2,4,6, 8} $$
$$ f_{PLLSAIQ} = f_{VCOOUT}/Q where R={ 2,3,4,5,6,7,8,9,10,11,12,13,14,15 } $$
$$ f_{PLLSAIR} = f_{VCOOUT}/R where R={ 2,3,4,5,6,7} $$

It is also possible to set additional prescalers for these signals in the DCKCFGR1 to generate 
the SAI1, SAI2 and LCD_CLK clock signals. The  LCD_CLK  is generated from the PLLSAIR signal thru 
the divider PLLSAIDIVR.

$$ f_{LCD_CLK} = f_{PLLSAIR}/DIVR where DIVR={2,4,6,8} $$


Board LCD-Display interface
---------------------------

The connection between LCD and display are:
 
 LCD signal   | Board signal      | MCU signal             |
--------------|-------------------|------------------------|
  CLK         | LCD_CLK           | PI14                   |
  LCD_R       | LCD_R0-7          | PI15 PJ0-6             |
  LCD_G       | LCD_G0-7          | PJ7-11 PK0-2           |
  LCD_B       | LCD_B0-7          | PE4 PJ13-15 PG12 PK4-6 |
  HSYNC       | LCD_HSYNC         | PI10                   |
  VSYNC       | LCD_VSYNC         | PI9                    |
  DE          | LCD_DE            | PK7                    |
  INT         | LCD_INT           | PI13                   |
  SCL         | LCD_SCL           | PH7                    |
  SDA         | LCD_SDA           | PH8                    |
  SDA         | LCD_RST/NRST      | NRST                   |


The alternatives for LCD pins are shown below. Those used for LCD connection are marked.


LCD Signal   | ALT     |  Pin
-------------|---------|--------------------
LCD_CLK      |  AF14   | *PI14* PE14 PG7
LCD_R0       |  AF14   | PG13 PH2 *PI15*
LCD_R1       |  AF14   | PA2 PH3 *PJ0*
LCD_R2       |  AF14   | PA1 PC10 PH8 *PJ1*
LCD_R3       |  AF14   | PH9 *PJ2* PB0 (AF9)
LCD_R4       |  AF14   | PA5 PA11 PH10 *PJ3*
LCD_R5       |  AF14   | PA12 PC0 PH11 *PJ4*
LCD_R6       |  AF14   | PA8 PH12 *PJ5* PB1(AF9)
LCD_R7       |  AF14   | PE15 PG6 *PJ6*
LCD_G0       |  AF14   | PE5 *PJ7*
LCD_G1       |  AF14   | PE6 *PJ8*
LCD_G2       |  AF14   | PA6 PH13 *PJ9*
LCD_G3       |  AF14   | PE11 PH14 *PJ10* PG10(AF9)
LCD_G4       |  AF14   | PB10 PH15 *PJ11*
LCD_G5       |  AF14   | PB11 PI0 *PK0*
LCD_G6       |  AF14   | PC7 PI1 *PK1*
LCD_G7       |  AF14   | PD3 PI2 *PK2*
LCD_B0       |  AF14   | *PE4* PG14 PJ12
LCD_B1       |  AF14   | PG12 *PJ13*
LCD_B2       |  AF14   | PD6 PG10 *PJ14*
LCD_B3       |  AF14   | PD10 PG11 *PJ15*
LCD_B4       |  AF14   | PE12 PG12 *PG12(AF9)* PI4 PK3
LCD_B5       |  AF14   | PA3 PI5 *PK4*
LCD_B6       |  AF14   | PB8 PI6 *PK5*
LCD_B7       |  AF14   | PB9 PI7 *PK6*
LCD_VSYNC    |  AF14   | PA4 *PI9* PI13
LCD_HSYNC    |  AF14   | PC6 *PI10* PI12
LCD_DE       |  AF14   | PE13 PF10 *PK7*


The following pins uses GPIO I/O or other modes to control the LCD.

LCD Signal    |        |  Pin   |  Description
--------------|--------|--------|------------------
 LCD_DISP     | GPIO   | I12    |  Enable LCD
 LCD_INT      | GPIO   | I13    |  LCD Interrupt
 LCD_BL_CTRL  | GPIO   | K3     |  Backlight PWM
 

Capacitive Touch Panel (CTP) interface
--------------------------------------


 The LCD display has a Capacitive Touch Display (CTP), controlled by HTC043C.
 
 It uses an I2C interface shared with the Audio WM8994ECS/R  device.
 
| Board Signal    | Signal | Pin  | Description             |  Interface  |
|-----------------|--------|------|-------------------------|-------------|
|  LCD\_SCL       | SCL    | PH7  | Shared with AUDIO\_SCL  |   I2C3_SCL  |
|  LCD\_SDA       | SDA    | PH8  | Shared with AUDIO\_SDA  |   I2C3_SDA  |
|  LCD_INT        | INT    | PI13 | Interrupt               |   GPIOI 13  |


As described in the schematics the I2C interface of the CTP uses the default 
address 01110000 (=0x70). The Audio WM8994ECS/R uses the address 0x00110100 (=0x34), as in 
the Discovery board or 0x00110110 (=0x36) according tho pin CS/ADDR, low or high, respectively.




Implementation
--------------

The HSE clock signal is generated from an external 25 MHz crystal oscillator. To get a 200 MHz clock
signal needed for the SDRAM, it must be divided by 25, multiplied by 400 and then divided by 2, 
since the minimum for Q is 2.

$$ f_{HCLK} = (( f_{HSI}/M )*N / Q ) $$

| Input |  Freq |  VCOIN |  M  |   N   |   Q   | SYSCLK |
|-------|-------|--------|-----|-------|-------|--------|
|  HSE  | 25 MHz|  1 MHz | 25  |  400  |   2   | 200 MHz|
|  HSE  | 25 MHz|  1 MHz | 25  |  432  |   2   | 216 MHz|
|  HSI  | 16 MHz|  2 MHz |  8  |  200  |   2   | 200 MHz|
|  HSI  | 16 MHz|  2 MHz |  8  |  216  |   2   | 216 MHz|

The HCLK can be divided by the AHB PRESC prescaler to generate de HCLK (=SystemCoreClock). It is
specified by the HPRE field in RCC_CFGR.

The input for the other PLL generator is then 1 MHz.


The LCD_CLK frequency has a 12 MHz maximal value. It is generated from PLLSAIR clock signal thru
a divider that can have the value 2, 4, 6 or 8.

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


The combination of parameters N, R, PLLDIVR depends on the other uses of the PLLSAI signals,
SAI1 and SAI2 clock signals, used by DMA, Serial Audio Interface (SAI) 1 and 2. 


 References
 ----------
 
 1. [RK043FN48H-CT672B Datasheet](https://mikrocontroller.bplaced.net/wordpress/wp-content/uploads/2018/01/RK043FN48H-CT672B-V1.0.pdf)
 2. [OTA5180A Datasaheet](https://www.newhavendisplay.com/resources_dataFiles/datasheets/LCDs/OTA5180A.pdf)
 3. [FTA5336GQQ Datasheet](https://www.newhavendisplay.com/resources_dataFiles/datasheets/touchpanel/FT5336.pdf)
 4. [A little about graphics subsystem internals on microcontrollers](https://alexkalmuk.medium.com/a-little-about-graphics-subsystem-internals-on-microcontrollers-d952cfd0966a)
 5. [Source for FT5336 driver](https://os.mbed.com/teams/ST/code/BSP_DISCO_F746NG/file/c9112f0c67e3/ft5336.h/)
 6. [Info about RK043FN66HS-CTG](https://community.st.com/s/question/0D50X0000B5H5uf/display-rk043fn66hsctg-from-rocktech)

 
 
 
 
