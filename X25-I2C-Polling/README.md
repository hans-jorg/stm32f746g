I2C
=====================

Introduction
------------

The board uses the I2C interface to connect to the touch controller and the audio codec.
There is a I2C connection at the Arduino connectors too.

I2C
---

I2C is a multi master synchronous protocol, that uses only two pins for interconnecting.


| Signal    |  Description          |
|-----------|-----------------------|
|   SCL     |  Transmission Clock   |
|   SDA     |  Transmission Data    |


The clock signal is always driven by the master. When there are more than one master attempting to drive it, a collision detection mechanism is used to disable one of them.

Transmission Data (SDA) is driven by the master in write operations, and by the slave, in read operations.

Each slave has an address. Generally, it is a 7-bit address, but a 10-bit address can be used.


| Speed        |  Transfer rate               |
|--------------|------------------------------|
|  Standard    |  up to 100 Kbps              |
|  Fast        |  up to 400 Kbps              |
|  Fast plus   |  up to 1 Mbps                |
|  High Speed  |  up to 3.4 Mbps              |

I2C uses two lines (SCL and SDA), that are pulled up to a high voltage (logical level 1).
In normal condition, the SDA line only changes when the SCL line is low (0). When SDA changes from high to low when SCL is high, it signalizes the start of a package (SOP). When SDA changes from low to high when SCL is high, it signalizes the end o a package (EOP).


    SAAAAAAAMKDDDDDDDDKDDDDDDDDK ... DDDDDDDDKE
     6543210  76543210 76543210      76543210

    S: Start signal (SOP)
    A7-0: I2C Address
    M: Mode (0:Write, 1:Read)
    K: Acknowledge
    D7-0: Data
    E: End signal (EOP)

When using 10-bit address, two bytes are used. The first byte starts with four ones (1110)

    S11110AAMKAAAAAAAAKDDDDDDDDK ... DDDDDDDDKE
          98  76543210 76543210      76543210

The following addresses are reserved.

| Address/Mode   |   Description                                       |
|----------------|-----------------------------------------------------|
| 0000 000 0     | General call address                                |
| 0000 000 1     | Start byte                                          |
| 0000 001 X     | CBUS address                                        |
| 0000 010 X     | Extension for different bus format                  |
| 0000 011 X     | Future use                                          |
| 0000 1XX X     | High speed controller code                          |
| 1111 1XX 1     | Reserved                                            |
| 1111 0XX X     | Higher order 2 bits of a 10-bit address             |



It is possible to use a Repeat Start to have a write and a read operation in the same packet.


### Timing

The main timing parameter is the time interval between received or transmitted data.

| Speed<br/>(Kbps)| Period<br/>(us) | Time interval<br/>(us) |
|-----------------|-----------------|------------------------|
|  100	          |    10    	    |    90.00               |
|  400	          |     2.5   	    |    22.50               |
| 1000            | 	1    	    |     9.00               |
| 3400	          |     0.294	    |     2.65               |

One can see that at higher speeds, there will be a lot of overhead when using interrupts.


Information about timing for the SCL and SDA lines is show below (Table 128 from REFMAN).

| Symbol        |   Parameter     | Standard mode | Fast mode | Fast mode plus | SMBus     | Unit  |
|---------------|-----------------|---------------|-----------|----------------|-----------|-------|
| tHIGH min     | Duty cycle      |     40        |  24       |     ?          |    ?      |   %   |
| tHIGH max     | Duty cycle      |     53        |  52       |     ?          |    ?      |   %   |
| tLOW,SCL min  | Low time        |      4.7      |   1.3     |     0.5        |           |  us   |
| tHIGH,SCL min | High time       |      4.0      |   0.6     |     0.26       |           |  us   |
| tHD;DAT min   | Data hold time  |      0        |   0       |     0          |    0.3    |  us   |
| tVD;DAT max   | Data valid time |      3.45     |   0.9     |     0.45       |           |  us   |
| tSU;DAT min   | Data setup time |    250        | 100       |    50          |  250      |  ns   |
|  tr max       | Rise time       |   1000        | 300       |   120          | 1000      |  ns   |
|  tf max       | Fall time       |    300        | 300       |   120          |  300      |  ns   |




I2C peripherals of the STM32F746 MCU
------------------------------------


The STM32F746NG has four I2C peripherals named I2C1 to I2C4. Their pins connections are show below.

|  I2C   |    SCL           |           SDA      |
|--------|------------------|--------------------|
|  I2C1  |  PB6 *PB8*       |  PB7 *PB9*         |
|  I2C2  |  PB10 PF1 PH4    |  PB11 PF0 PH5      |
|  I2C3  |  PA8 *PH7*       |  PC8 *PH8*         |
|  I2C4  |  PD12 PF14 PH11  |  PD13 PF15 PH12    |

The SCL and SDA pins must be configured as ALTERNATE FUNCTION = 4, Open Drain, High Speed, Pull-Up Active.

> To use Fast mode Plus, the support for 20 mA output current must be enabled in the system configuration controller (SYSCFG)


They can be used as master and slave, transmitter and receiver. They support 7-bin and 10-bin addresses, speed up to 1 Mbps (Fast Mode Plus). So, High Speed I2C is not supported.


| I2C    |  Address range             |
|--------|----------------------------|
| I2C1   |  0x40005400 - 0x400057FF   |
| I2C2   |  0x40005800 - 0x40005BFF   |
| I2C3   |  0x40005C00 - 0x40006FFF   |
| I2C4   |  0x40006000 - 0x400063FF   |

They support also the SMBus protocol, a derivative from I2C. An additional signal (SMBA) is used in the SMBus mode.

They need two clock sources:

* I2CCLK (i2c_ker_ck): It is used to clock the I2C peripheral.
* PCLK (i2c_pclk): It is used to clock the I2C registers.

The following conditions must be met by the I2C clock signal.

t_I2CCLK < (t_LOW-t_filters)/4 and T_I2CCLK < t_HIGH
t_PCLK < 4/3 t_SCL

where t_LOW and t_HIGH are the low and high time of the SCL signal. t_filters can be 0 when disabled, 260 ns for the analog filter and DNF x t_I2CCLK for the digital filter. DNF is a field in the CR1 register.

There are tables on the STM32F74xxx Reference Manual for 8, 16 and 48 MHz. 

> The calculation of the timing parameters can be done by the builtin tool of the STRMCubeIDE.

I2C peripherals of the STM32F746NG Discovery Board
--------------------------------------------------

The STM32F746NG has four I2C peripherals named I2C1 to I2C4. Their pins connections are show below.

|  I2C   |    SCL           |           SDA      |
|--------|------------------|--------------------|
|  I2C1  |  PB6 *PB8*       |  PB7 *PB9*         |
|  I2C2  |  PB10 PF1 PH4    |  PB11 PF0 PH5      |
|  I2C3  |  PA8 *PH7*       |  PC8 *PH8*         |
|  I2C4  |  PD12 PF14 PH11  |  PD13 PF15 PH12    |



Not all I2C peripherals can be used on the Discovery board, because their pins have other uses as show in Annex A. The I2C peripherals that can be used are shown below.

| I2C    | Pins    | Usage                                        |
|--------|---------|----------------------------------------------|
| I2C1   | PB8,PB9 | External I2C on Arduino pins A4 and A5 |                 |
| I2C3   | PH7,PH8 | Touch controller and                 |

The connection of PB8 and PB9 to the Arduino pins is controlled by
solder bridges. The default setup is to use them as analog inputs.


| ARD Pin |   I2C          |   Analog       |
|---------|----------------|----------------|
| A4      |   I2C1SDA (SB4)|  ADC3_IN5 (SB5)|
| A5      |   I2C1SCL (SB1)|  ADC3_IN4 (SB3)|


The I2C3 is used to control the touch controller in the LCD and the Sound Code.

| I2C address      | I2C Address       | Device ID (Write) | Device ID (Read) |
|------------------|-------------------|-------------------|------------------|
| Touch controller | 011 1000 (0x38)   | 01110000 (0x70)   | 01110001 (0x71   | 
| Audio codec      | 001 1010 (0x1A)   | 00110100 (0x34)   | 00110101 (0x35)  |

> There is a confusion between I2C Address (a 7-bit value) and the
> first byte of a I2C packet, that contains the I2C address and the read/write
> bit. The Schematics shows the first byte as the I2C address.
                     



### Configuration

In order to configure an I2C peripheral for operation, the following steps must be followed.

* Disable I2C
* Configure filters
* Configure clock
* Configure timing parameters
* Configure pins
* Configure as slave (if it is the case)
* Enable the peripheral

### Configure filters

It is possible to use an analog filter, a digital filter or both. This is a requisite for Fast-Mode and Fast-Mode-Plus speeds. The analog filter is enabled by default and can be disabled by setting the ANFOFF bit of the CR1 register. The digital filter can be controlled by setting the 4-bit DNF field of CR1. The digital filter suppresses all pulses with width smaller than tI2CCLK

### Configure clock

The peripheral needs two clock signals. One for the peripheral registers and other for the internal working of the peripheral.

The peripheral register clock is set by setting the corresponding bit in the RCC_APB1ENR.

The clock source for the internal working is set by configuring the corresponding fields in the RCC_DCKCFGR2 register.

| field   |   Peripheral |
|---------|--------------|
| 23-22   |    I2C4SEL   |
| 21-20   |    I2C3SEL   |
| 19-18   |    I2C2SEL   |
| 17-16   |    I2C1SEL   |

The options are

| I2CxSEL |  Clock source |
|---------|---------------|
|   00    | APB1CLK/PCLK1 |
|   01    | SYSCLK        |
|   10    | HSI           |
|   11    | Reserved      |


The clock for the I2C is configure in the RCC_APB1ENR registers

| bit     |   Peripheral |
|---------|--------------|
|   24    |    I2C4EN    |
|   23    |    I2C3EN    |
|   22    |    I2C2EN    |
|   21    |    I2C1EN    |

Additionally, the clock can be enabled in low power mode by setting the corresponding bit in RCC_APB1LPENR register

| bit     |   Peripheral   |
|---------|----------------|
|   24    |    I2C4LPEN    |
|   23    |    I2C3LPEN    |
|   22    |    I2C2LPEN    |
|   21    |    I2C1LPEN    |

### Setting timing parameters

The basic unit of time is the period of the clock signal used. The configuration is done by setting the following fields of the I2C_TIMINGR register.

| Field   |  Name        |  Description                              |
|---------|--------------|-------------------------------------------|
| 31-28   | PRESC        | Timing prescaler (tPRESC=(PRESC+1)*tI2CCLK|
| 23-20   | SCLDEL       | Data setup time                           |
| 19-16   | SDADEL       | Data hold time                            |
| 15-8    | SCLH         | SCL high time (=SCLH+1)*tPRESC            |
|  7-0    | SCLL         | SCL low time (=SCLL+1)*tPRESC             |


### Configure Pins

The corresponding SCL and SDA pins must be configures as AF=4, Open Drain, High Speed, Pull-Up Active. To use Fast mode Plus, the support for 20 mA output current must be enabled in the system configuration controller (SYSCFG)

### Configure as slave

When operating as slave, the OARx registers must be set. To use 10-bit addressing, the OA1MODE bit of OAR1 register must be set. Field OA1 must be set to the address and then the bit OA1EN. Since the I2c device can use two adresses the procedure must be repeated for the OAR2 register.


### Enable the peripheral

To enable the peripheral, set PE bit of the CR1 register to 1.

### Summary

The following registers must be configured

* RCC_APB1ENR
* RCC_DCKCFGR2
* I2C_TIMINGR
* I2C_OAR1
* I2C_OAR2
* I2C_CR1
* I2C_CR2

Configuration for I2C3 (Touch and audio controller)
---------------------------------------------------

The Touch and Audio controllers support speed up to 400 Kbps (Fast-Mode).
Using the HSI as clock source. It is an internal RC oscillator with a 16 MHz frequency.
It is not so precise as a crystal base oscillator, but it is factory trimmed to a 1% accuracy.
This is enough for the I2C operation.

According the STM32F746NG datasheet, it has an accuracy of 4% in the -10C to 85C temperature range.

The minimal frequencies for the I2CCLK are show below (Table 74 of Datasheet)

| Mode                | Only Analog filter | Only digital filter (DNF = 1) |
|---------------------|--------------------|-------------------------------|
| Standard mode       |    2 MHz           |            -                  |
| Fast mode           |   10 MHz           |          9 MHz                |
| Fast plus mode      |   22.5 MHz         |         16 MHz                |


For a 16 MHz I2CCLK frequence, it is possible to obtain the parameters from Table 182 of the STM32F746NG Reference Manual.

| Parameter     |  10 KHz |  100 KHz  |  400 KHz  |   1 MHz    |
|---------------|---------|-----------|-----------|------------|
|  PRESC        |    3    |     3     |      1    |       0    |
|  SCLL         |  0xC7   |   0x13    |     0x9   |     0x4    |
|  SCLH         |  0xC3   |   0x0F    |     0x3   |     0x2    |
|  SDADEL       |  0x02   |   0x02    |     0x2   |     0x0    |
|  SCLDEL       |  0x04   |   0x04    |     0x3   |     0x2    |

The resulting timing parameters are

| Parameter     |  10 KHz   |  100 KHz  |   400 KHz  |    1 MHz    |
|---------------|-----------|-----------|------------|-------------|
|  tSCLL        |   50   us |    5.0 us |  1250   ns |   312.5 ns  |
|  tSCLH        |   59   us |    4.0 us |   500   ns |   187.5 ns  |
|  tSCL         | ~100   us |  ~10   us | ~2500  ns  | ~1000   ns  |
|  tSDADEL      |  500   ns |  500   ns |   250   ns |     0   ns  |
|  tSCLDEL      | 1250   ns | 1250   ns |   500   ns |   187.5 ns  |


SCL period tSCL is greater than tSCLL + tSCLH due to SCL internal detection delay. Values provided for tSCL are examples only.

There are other values that can be used, by reducing PRESC and increasing SCLH and SCHL. Just keep the products $$ (PRESC+1)*(SCLH+1) $$ and $$ (PRESC+1)*(SCLH+1) $$ constants. For example, the STM32CubeMX generates the values below.


| Parameter       |  Value       |
|-----------------|--------------|
| I2CCLK          | 16 MHz (HSI) |
| Analog filter   |   On         |
| Digital filter  |   0          |

| 31-28   | PRESC        | Timing prescaler (tPRESC=(PRESC+1)*tI2CCLK|
| 23-20   | SCLDEL       | Data setup time                           |
| 19-16   | SDADEL       | Data hold time                            |
| 15-8    | SCLH         | SCL high time (=SCLH+1)*tPRESC            |
|  7-0    | SCLL         | SCL low time (=SCLL+1)*tPRESC             |


| Parameter     |  100 KHz   |  400 KHz   |    1 MHz   |
|---------------|------------|------------|------------|
|  tr           |   1000 ns  |   300 ns   |   120      |
|  tf           |    300 ns  |   300 ns   |   120      |
|  TIMINGR      | 0x10911E24 | 0x00610611 | 0x00200105 |
|  SCLL         |    0x24    |   0x11     |    0x05    |
|  SCLH         |    0x1E    |   0x06     |    0x01    |
|  SDADEL       |    0x01    |   0x01     |    0x00    |
|  SCLDEL       |    0x09    |   0x06     |    0x02    |
|  PRESC        |    0x10    |   0x00     |    0x00    |





Application Programming Interface (API)
---------------------------------------

There are three programming modes that can be used for the I2C peripherals in the STM32.

* Polling: the peripheral are continuously checked by reading its registers.
* Interrupt: the peripheral generates an interrupt when certain conditions are encountered.
* DMA: the peripheral access memory to get the data to be transmitted or store the data received sequentially into memory.






Annex A
-------

Usage of I2C pins in the STM32F746 Discovery Board


| Pin    | Conflicts                               |
|--------|-----------------------------------------|
| PB6    | QSPI (Quad SPI Flash Memory             |
| PB7    | VCP (Virtual Communication Port)        |
| PB10   | ULPI (USB High Speed OTG)               |
| PF1    | FMC (Flexible Memory Controller (SDRAM) |
| PH4    | ULPI (USB High Speed OTG)               |
| PB11   | ULPI (USB High Speed OTG)               |
| PF0    | FMC (Flexible Memory Controller (SDRAM) |
| PH5    | ULPI (USB High Speed OTG)               |
| PA8    | uSD (micro SD)                          |
| PC8    | uSD (micro SD)                          |
| PD12   | QSPI (Quad SPI Flash Memory             |
| PF14   | LCD  (LCD Panel)                        |
| PH11   | ULPI (USB High Speed OTG)               |
| PD13   | QSPI (Quad SPI Flash Memory)            |
| PF15   | FMC (Flexible Memory Controller (SDRAM) |
| PH12   | DCMI (Camera connector)                 |


References
