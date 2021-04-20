External SDRAM
==============


Introduction
------------

The board uses a MT48LC4M32B2B5-6A SDRAM integrated circuit to expand its RAM using the Flexible
Memory Controller.

The device is a 128 Mbits (=64 MBytes) SDRAM and it has four banks of 32x1 M cells.
Only half of them (64 Mbits=8 MBytes) are used due to fact that only 16 bits of the data bus
 is connected to the MCU. The reasons for this is to use these pins for other application.
 
The STM32F746 has a FMC unit, that can interface to SDRAM and FLASH devices. Using FMC, these
devices appear as occupying a memory region and can be accessed as a normal RAM or ROM.

Usage
-----

There is only one routine.

int SDRAM_Init();

If it returns zero, the SDRAM can be accessed in the 0x0000_0000-0x0000_0000 address range.



Description
-----------

The SDRAM device used is a MT48LC4M32B2B5-6A, manufactured by Micron.

Only 16 bits of data bus (DQ15..0) )are used. So it is then not neccessary to
use full DQM. Only DQM0 and DQM1 need to be used.

It is a PC166/PC100, i.e., can work with a 100/166 MHz clock


### RAM Device Information


It has the following interface

| Signal    | Type |     Description
|-----------|------|-----------------------------
| DQ31..0   |  I/O | 32-bit data bus
| A11..0    |  I   | 12 bit address input
| BA1..0    |  I   | 2 bit bank selector
| DQM3..0   |  I   | 4 bit mask input to enable byte write operation
| CAS#      |  I   | Column Address Selector
| RAS#      |  I   | Row Address Selector
| WE#       |  I   | Write operation
| CS#       |  I   | Chip select
| CKE       |  I   | Clock enable
| CLK       |  I   | Clock


### SDRAM characteristics

| Parameter           |           Value
|---------------------|-----------------------------
|  Configuration      | 4 banks of 1 Mega 32 bits words
|  Capacity           | 128 Mbit = 16 MByte (Only 8 are used )
|  Refresh count      | 4096
|  Row addressing     | A11:0    (12 bits = 4K)
|  Column addresssing | A7:0     (8 bits = 256)
|  Bank addressing    | BA1:0    (2 bits = 4 )
|                     |
| *Addressing *       |
| Total addressing    | 2+8+12 = 22 bits = 4194304 addresses = 4 Mwords
|                     | 32 bit words  = 128 Mbit
|                     |  4 bytes      =  16 MBytes
|                     |
| Burst lenghts       | 1,2,4,8
| CAS latency         | 1,2,3
| Autorefresh         | 64 ms x 4096
| Cycle time          | 167 MHz (-6A) ???)
| Self refresh        |
| Auto precharge      |
|                     |
| *Timing*            |
| Clock Frequency     | < 167 MHz
| Cycle               | 3-3-3
| t_RCD               | 18 ns
| t_RP                | 18 ns
| CL                  | 18 ns
|                     |
| Minimum frequency   |
|     CL=3            |  166 MHz => 6 ns
|     CL=2            |  100 MHz => 10 ns
|     CL=1            |   50 MHz => 20 ns
|


### FMC Configuration Information

It uses Bank 4 and 5 for FMC SDRAM. This board is hardwired to use the Bank 4,
      a.k.a, SDRAM Bank1 due to the use of SDCKE0 and SDNE0
      
      
|  Bank |   Size     | Address range
|-------|------------|-----------------------
|   4   |  4x64 MB   | 0xC00_0000-0xCFFF_FFFF
|   5   |  4x64 MB   | 0xD00_0000-0xDFFF_FFFF

Example  

For a 16 bit memory width (Table 55 of RM))

* Bit 28 specifies which FMC bank to be accessed.
* Bits 22:21 which bank for 16-bit memory
* Bits 20:9 row address
* Bits 8:1  column address
* Bit0 must be connected to memory A0 (BM0)


### SDRAM Connection to MCU


| Chip signal  | Board signal | MCU Signal  |  Function
|--------------|--------------|-------------|---------
|  DQ0         |  FMC_D0      | PD14        |  AF12
|  DQ1         |  FMC_D1      | PD15        |  AF12
|  DQ2         |  FMC_D2      | PD0         |  AF12
|  DQ3         |  FMC_D3      | PD1         |  AF12
|  DQ4         |  FMC_D4      | PE7         |  AF12
|  DQ5         |  FMC_D5      | PE8         |  AF12
|  DQ6         |  FMC_D6      | PE9         |  AF12
|  DQ7         |  FMC_D7      | PE10        |  AF12
|  DQ8         |  FMC_D8      | PE11        |  AF12
|  DQ9         |  FMC_D9      | PE12        |  AF12
|  DQ10        |  FMC_D10     | PE13        |  AF12
|  DQ11        |  FMC_D11     | PE14        |  AF12
|  DQ12        |  FMC_D12     | PE15        |  AF12
|  DQ13        |  FMC_D13     | PD8         |  AF12
|  DQ14        |  FMC_D14     | PD9         |  AF12
|  DQ15        |  FMC_D15     | PD10        |  AF12
|  A0          |  FMC_A0      | PF0         |  AF12
|  A1          |  FMC_A1      | PF1         |  AF12
|  A2          |  FMC_A2      | PF2         |  AF12
|  A3          |  FMC_A3      | PF3         |  AF12
|  A4          |  FMC_A4      | PF4         |  AF12
|  A5          |  FMC_A5      | PF5         |  AF12
|  A6          |  FMC_A6      | PF12        |  AF12
|  A7          |  FMC_A7      | PF13        |  AF12
|  A8          |  FMC_A8      | PF14        |  AF12
|  A9          |  FMC_A9      | PF15        |  AF12
|  A10         |  FMC_A10     | PG0         |  AF12
|  A11         |  FMC_A11     | PG1         |  AF12
|  BA0         |  FMC_BA0     | PG4         |  AF12
|  BA1         |  FMC_BA1     | PG5         |  AF12
|  RAS         |  FMC_SDNRAS  | PF11        |  AF12
|  CAS         |  FMC_SDNCAS  | PG15        |  AF12
|  WE          |  FMC_SNDWE   | PH5         |  AF12
|  CLK         |  FMC_SDCLK   | PG8         |  AF12
|  CLKE        |  FMC_SDCKE0  | PC3         |  AF12
|  CS          |  FMC_SDNE0   | PH3         |  AF12
|  DQM0        |  FMC_NBL0    | PE0         |  AF12
|  DQM1        |  FMC_NBL1    | PE1         |  AF12


Timing parameters (in clock units)  

| Field   |   Description
|---------|----------------------------------------------------
| TMRD    | Load Mode Register to Active
| TXSR    | Exit self refresh delay
| TRAS    | Self refresh time, i.e., the minimum Self-refresh period
| TRC     | Row cycle delay. i.e. the delay between the Refresh command and the Activate command
| TWR     | Recovery delay, i.e., delay between a Write and a Precharge command
| TRP     | Row precharge delay, i.e., delay between a Precharge command and another command
| TRCD    | Row to column delay


Timing information for f_SDCLK = 100 MHz  

|Parameter| Encoding | Recommended | Description
|---------|----------|-------------|--------------------------------------------------------
| TMRD    | 2        | 2 t_CK      | LOAD MODE REGISTER command to ACTIVE or REFRESH command
| TXSR    | 6.       | 67 ns       | Exit Self-Refresh to Active Delay
| TRAS    | 5        | 60 ns       | Self refresh time ?=? Auto refresh time
| TRC     |          |             |
| TWR     | 0        |  3 ns       | Data-in to PRECHARGE command
| TRP     | 1        | 18 ns       | PRECHARGE command period
| TRCD    |          |             | 
 
OBS: TXSR=5 is used in the example  


#### Initialization Sequence

The initialization sequence is managed by software. If the two banks are used, the
initialization sequence must be generated simultaneously to Bank 1and Bank 2 by setting
the Target Bank bits CTB1 and CTB2 in the FMC_SDCMR register:

  1. Program the memory device features into the FMC_SDCRx register. The SDRAM
     clock frequency, RBURST and RPIPE must be programmed in the FMC_SDCR1
     register.
  2. Program the memory device timing into the FMC_SDTRx register. The TRP and TRC
     timings must be programmed in the FMC_SDTR1 register.
  3. Set MODE bits to ‘001’ and configure the Target Bank bits (CTB1 and/or CTB2) in the
     FMC_SDCMR register to start delivering the clock to the memory (SDCKE is driven
     high).
  4. Wait during the prescribed delay period. Typical delay is around 100 μs (refer to the
     SDRAM datasheet for the required delay after power-up).
  5. Set MODE bits to ‘010’ and configure the Target Bank bits (CTB1 and/or CTB2) in the
     FMC_SDCMR register to issue a “Precharge All” command.
  6. Set MODE bits to ‘011’, and configure the Target Bank bits (CTB1 and/or CTB2) as well
     as the number of consecutive Auto-refresh commands (NRFS) in the FMC_SDCMR
     register. Refer to the SDRAM datasheet for the number of Auto-refresh commands that
     should be issued. Typical number is 8.
  7. Configure the MRD field according to the SDRAM device, set the MODE bits to '100',
     and configure the Target Bank bits (CTB1 and/or CTB2) in the FMC_SDCMR register
     to issue a "Load Mode Register" command in order to program the SDRAM device.
     In particular:
     a) the CAS latency must be selected following configured value in FMC_SDCR1/2
        registers
     b) the Burst Length (BL) of 1 must be selected by configuring the M[2:0] bits to 000 in
        the mode register. Refer to SDRAM device datasheet.
        If the Mode Register is not the same for both SDRAM banks, this step has to be
        repeated twice, once for each bank, and the Target Bank bits set accordingly.
  8. Program the refresh rate in the FMC_SDRTR register. The refresh rate corresponds to
     the delay between refresh cycles. Its value must be adapted to SDRAM devices.
  9. For mobile SDRAM devices, to program the extended mode register it should be done
     once the SDRAM device is initialized: First, a dummy read access should be performed
     while BA1=1 and BA=0 (refer to SDRAM address mapping section for BA[1:0] address



### Configuration of the MT48LC4M32B2

Mode register   

| Field   |   Description
|---------|-------------------
|  2-0    | Burst length
|    3    | Burst type
|  6-4    | CAS Latency
|  8-7    | Operation mode
|    9    | Write burst mode
|12-10    | Reserved


Burst length encoding   

|   M2-0 |  Burst length
| -------|--------
|   000  |     1
|   001  |     2
|   010  |     4
|   011  |     8
|   100  |     -
|   101  |     -
|   110  |     -
|   111  |     -


Burst type  

|   M3   | Burst type
|--------|---------------
|    0   | Sequential
|    1   | Interleaved


CAS Latency  

|   M6-4 | CAS Latency
| -------|----------------
|   000  |     -
|   001  |     1
|   010  |     2
|   011  |     3
|   100  |     -
|   101  |     -
|   110  |     -
|   111  |     -


Operation mode   

|  M8-7  |  Operation mode
|--------|----------------------
|    00  |  Standard
| others |  Reserved


Write Burst Mode   

|   M9   |  Write burst mode
|--------|----------------------
|   0    | Programmed Burst Mode
|   1    | Single Location Access


Configuration used   

    Burst length     =  000 (1)
    Burst type       =    0 (Sequential)
    CAS Latency      =  010 (2)
    Operation mode   =   00 (Standard Operation)
    Write Burst Mode =    1 (Single Location Access)

#### Refresh Count

All rows must be refreshed every 64 ms. This can be done distributed along this time
or as a burst with an interval of 60 ns.

Refresh count depends on the SD_CLK signal  

    64 ms/4096 rows = 15.625 us
    15.625 us *100 MHz = 1562
    Subtract a safety margin (20)
    Counter = 1542
    Refresh rate = (1582+1)*100 MHz
    
    OBS: 20 or 20% ??  

OBS:   

* It must be different from TWR+TRP+TRC+TRCD+4 memory cycles
* It must be greater than 41


References
----------


1. [https://www.micron.com/products/dram/sdram/part-catalog/mt48lc4m32b2b5-6a-it](https://www.micron.com/products/dram/sdram/part-catalog/mt48lc4m32b2b5-6a-it)
2. [](https://media-www.micron.com/-/media/client/global/documents/products/data-sheet/dram/128mb_x32_sdram.pdf?rev=a4b9962d86784413b3cfa348b78a1360)



