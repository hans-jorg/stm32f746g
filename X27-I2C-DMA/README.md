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

When using 10-bit address, two bytes are used. The first byte starts with four ones (1111)

    S11111AAMKAAAAAAAAKDDDDDDDDK ... DDDDDDDDKE
          98  76543210 76543210      76543210

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


Information about timing for the SCL and SDA lines is show below.

| Symbol     |   Parameter     | Standard mode | Fast mode | Fast mode plus | SMBus     | Unit  |
|------------|-----------------|---------------|-----------|----------------|-----------|-------|
| tHIGH min  | Duty cycle      |     40        |  24       |     ?          |    ?      |   %   |
| tHIGH max  | Duty cycle      |     53        |  52       |     ?          |    ?      |   %   |
|tHD;DAT min | Data hold time  |      0        |   0       |     0          |    0.3    |  us   |
|tVD;DAT max | Data valid time |      3.45     |   0.9     |     0.45       |           |  us   |
|tSU;DAT min | Data setup time |    250        | 100       |    50          |  250      |  ns   |
|  tr max    | Rise time       |   1000        | 300       |   120          | 1000      |  ns   |
|  tf max    | Fall time       |    300        | 300       |   120          |  300      |  ns   |]




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

* Configure filters
* Enable the peripheral
* Configure clock
* Configure pins
* Configure peripheral

It is possible to use an analog filter, a digital filter or both. This is a requisite for Fast-Mode and Fast-Mode-Plus speeds. The analog filter is enabled by default and can be disable by setting the ANFOFF bit of the CR1 register. The digital filter can be set by setting the 4-bit DNF field of CR1.



To enable the peripheral, set PE bit of the CR1 register to 1.
The I2CCLK can be derived from the HSICLK, SYSCLK, PCLK1 (APB1 CLK) clock signals.



> To use Fast mode Plus, the support for 20 mA output current must be enabled in the system configuration controller (SYSCFG)

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
