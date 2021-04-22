External SDRAM
==============


Introduction
------------

The board uses a MT48LC4M32B2B5-6A SDRAM integrated circuit to expand its RAM using the Flexible
Memory Controller.

The device is a 128 Mbits (=16 MBytes) SDRAM and it has four banks of 32x1 M cells (4x1Mx32).
Only half of them (64 Mbits=8 MBytes) are used due to fact that only 16 bits of the data bus
 is connected to the MCU. The reasons for this is to use these pins for other application.
 
The STM32F746 has a FMC unit, that can interface to SDRAM and FLASH devices. Using FMC, these
devices appear as occupying a memory region and can be accessed as a normal RAM or ROM.

Usage
-----

There is only one routine.

int SDRAM_Init();

If it returns zero, the SDRAM can be accessed in the 0xC000\_0000-0x007F_FFFF address range.


Configuration
-------------

The following steps must be acomplished to enable the access to the SDRAM

1. Configure HCLK
2. Configure Pins
3. Configure FMC
4. Configure SDRAM Device
5. Configure Refresh


Description
-----------



### Flexible Memory Controller (FMC)

The FMC has two SDRAM banks, Bank 1 and Bank 2 (FMC Banks 5 and 6) and each one can interface to
 one SDRAM device.

The FMC support then, up to two SDRAM, each one with:
* 4 internal banks
* 8/16/32-bit data bus
* 13-bit multiplexed address bus
* Up to 11 columns
* Up to 13 rows

It has individual clock and chip enable for each SDRAM bank (or SDRAM device).



#### MCU interfacing to SDRAM.

| SDRAM      |   FMC         | Description
|------------|---------------|-------------------------------
| SDCLK      |               | SDRAM Clock
| SDCLKE1..0 |               | SDRAM Bank{1,2} Clock Enable
| SDNE1..0   |               | SDRAM Banks{1,2} Chip Enable
| A12..0     |  FMC_A12..0   | Address
| D31..0     |  FMC_D31..0   | Data
| BA1..0     |  FMC_A15..14  | Bank Address
| NRAS       |               | Row Address Strobe
| RCAS       |               | Column Address Strobe
| SDNWE      |               | Write Enable
| NBL3..0    |  FMC_NBL3..0  | Output Byte Mask for Write



#### FMC SDRAM Registers

The FMC SDRAM interface is done using the registers:

* SDCRx: SDRAM Control Register for SDRAM Bank x
* SDTRx: SDRAM Timing Register for SDRAM Bank x
* SDRTR: SDRAM Refresh Timer Register
* SDCMR: SDRAM Command Register
* SDSR:  SDRAM Status Register

> NOTE: Some configuration parameters are shared between the two banks.

> NOTE: SDRAM device must be first reinitialized after reset before issuing any new
access by the NOR Flash/PSRAM/SRAM or NAND Flash controller

##### SDRAM Control Register (SDCRx) fields

| Field   | Name       | Description
|---------|------------|-------------------------
| 14-13   | RPIPE*     | Read pipe
| 12-12   | RBURST*    | Burst read. 
| 11-10   | SDCLK*     | SDCLK = HCLK/n, n=2,3. 0=disable
|  9- 9   | WP         | Write Protect
|  8- 7   | CAS        | CAS Latency: 1,2,3
|  6- 6   | NB         | Number of internal banks (2 or 4)
|  5- 4   | MWID       | Memory Data Bus: 8, 16, 32
|  3- 2   | NR         | Number of Rows: 11, 12, 13
|  1- 0   | NC         | Number of Columns: 8, 9, 10, 11

\* Read-Only in SDCR2

##### SDRAM Timing Register (SDTRx) fields

| Field   | Name       | Description
|---------|------------|-------------------------
| 27-24   | TRCD       | Row to column delay = TRCD+1
| 23-20   | TRP*       | Row precharge delay = TRP+1
| 19-16   | TWR**      | Recovery delay=TWR+1
| 15-12   | TRC        | Row cycle delay=TRC+1
| 11- 8   | TRAS       | Self refresh time=TRAS+1
|  7- 4   | TXSR**     | Exit self-refresh delay=TXSR+1
|  3- 0   | TMRD**     | Load Mode Register to Active=TMRD+1

\*  Don't care in SDTR2
\** Must match the slowest device


##### SDRAM Command Register (SDCMR) fields

| Field   | Name       | Description
|---------|------------|-------------------------
| 21- 9   | MRD        | Mode Register definition
|  8- 5   | NRFS       | Number of Auto-Refresh
|  4- 4   | CTB1       | Command Target is Bank 1
|  3- 3   | CTB2       | Command Target is Bank 2
|  2- 0   | MODE       | Command mode

Command mode can be:

* 000: Normal mode - 
* 001: Clock Configuration Enable - ???
* 010: PALL (All Bank Precharge) Command
* 011: Auto refresh Command
* 100: Load Mode Register
* 101: Self-Refresh Command
* 110: Power-down command
* 111: Reserved

##### SDRAM Refresh Timer Register (SDTR) fields

| Field   | Name       | Description
|---------|------------|-------------------------
| 14-14   | REIE       | RES Interrupt Enable
| 13- 1   | COUNT      | Refresh Timer Count
|  0- 0   | CRE        | Clear Refresh Error Flag


$$ COUNT = "SDRAM Refresh Period"/"Number of rows" + 1 $$

> NOTE: The COUNT must be decremented by 20 to have a safe margin

> NOTE: COUNT must be greater than 40!

> NOTE: It is common to Bank 1 and 2

> NOTE: COUNT value must not be equal to the sum of the following timings: TWR+TRP+TRC+TRCD+4 
memory clock cycles.


##### SDRAM Status Register (SDSR)

| Field   | Name       | Description
|---------|------------|-------------------------
|  5- 5   | BUSY       | Busy status
|  4- 3   | MODES2     | Status Mode for Bank 2
|  2- 1   | MODES1     | Status Mode for Bank 1
|  0- 0   | RE         | Refresh Error Flag


#### FMC Configuration Information

FMC can control the interfacing to diverse types of memory. All memory between 0x6000_0000 and
0xDFFF_FFF (except 0xA000_0000-0xBFFF_FFFF) is reserved for FMC. This area is divided in 256 MByte blocks according 
Figure 2 of RM [3].

| Bank | Type        | Size   | Devices | Address range           | Block
|------|-------------|--------|---------|-------------------------|--------
|  1   | NOR Flash   | 256 MB |    4    | 0x6000_0000-0x6FFF_FFFF |   3
|  2   | Not used    | 256 MB |    -    | 0x7000_0000-0x7FFF_FFFF |   3
|  3   | NAND Flash  | 256 MB |    ?    | 0x8000_0000-0x8FFF_FFFF |   4
|  4   | Not used    | 256 MB |    -    | 0x9000_0000-0x9FFF_FFFF |   4
|  5   | SDRAM       | 256 MB |    1    | 0xC000_0000-0xCFFF_FFFF |   6
|  6   | SDRAM       | 256 MB |    1    | 0xD000_0000-0xDFFF_FFFF |   6

TE: There is an error in Figure  of RM!!! Bank 4 is not used?

It uses Bank 4 and 5 for FMC SDRAM. This board is hardwired to use the Bank 4,
      a.k.a, SDRAM Bank1 due to the use of SDCKE0 and SDNE0
      
      
|  Bank |SDRAM Bank |  Size   |         | Address range
|-------|-----------|---------|--------------------------
|   4   |     1     | 256 MB  | 4x64 MB | 0xC000_0000-0xCFFF_FFFF
|   5   |     2     | 256 MB  | 4x64 MB | 0xD000_0000-0xDFFF_FFFF



#### SDRAM Addressing  

HADDR is a 32 bit register used to access the SDRAM.

HADDR[28] selects wich SDRAM bank is to be accessed.

The other fields of HADDR depend on the bus size as shown below

For the MT48LC4M32B2B5 SDRAM device, the usage of HADDR is

For a 16 bit memory width (Table 55 of RM))

| HADDR   |  Size |  Description            | Signal
|---------|---------------------------------|---------------
|  31-29  |     3 | 110                     | 
|     28  |     1 | Select which SDRAM bank | 
|  27-23  |     6 | Reserved                | 0
|  22-21  |     2 | Bank1..0 (4 banks)      | BA1..0
|  20- 9  |    12 | Row                     | A11..0
|   8- 1  |     8 | Column                  | A7..0 
|      0  |     1 | Bit Mask 0              | BM0

> NOTE: FMC_A0 must be connected to A0 ?????

> NOTE: A10 must be connected (Precharge is not supported)


### SDRAM Connection to MCU


The tables below are for a 16-bit data bus and a 12-bit multiplexed address.

These signals are shared by the two banks (chips)


| Chip signal  | Board signal | MCU Signal  |  Function
|--------------|--------------|-------------|---------
|  CLKE        |  FMC_SDCKE0* | PC3/PH2     |  AF12
|  CS          |  FMC_SDNE0*  | PH3         |  AF12
|  CLK         |  FMC_SDCLK   | PG8         |  AF12
|  RAS         |  FMC_SDNRAS  | PF11        |  AF12
|  CAS         |  FMC_SDNCAS  | PG15        |  AF12
|  WE          |  FMC_SNDWE   | PH5         |  AF12
|  BA0         |  FMC_BA0     | PG4         |  AF12
|  BA1         |  FMC_BA1     | PG5         |  AF12
|  DQM0        |  FMC_NBL0    | PE0         |  AF12
|  DQM1        |  FMC_NBL1    | PE1         |  AF12
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

\*: Bank 1 clock and chip enable

These signals below are not connected (used) in this board.

| Chip signal  | FMC signal   | MCU Signal  |  Function
|--------------|--------------|-------------|---------
|  CLKE        | FMC_SDCKE1*  | PH7         |  AF12
|  CS          | FMC_SDNE1*   | PH6         |  AF12
|  DQM2        | FMC_NBL2     | PI4         |  AF12
|  DQM3        | FMC_NBL3     | PI5         |  AF12
|  CS          | FMC_SDNE1    | P           |  AF12
|  A12         | FMC_A12      | PG2         |  AF12
|  DQ16        | FMC_D16      | PH8         |  AF12
|  DQ17        | FMC_D17      | PH9         |  AF12
|  DQ18        | FMC_D18      | PH19        |  AF12
|  DQ19        | FMC_D19      | PH11        |  AF12
|  DQ20        | FMC_D20      | PH12        |  AF12
|  DQ21        | FMC_D22      | PH13        |  AF12
|  DQ22        | FMC_D23      | PH14        |  AF12
|  DQ23        | FMC_D23      | PH15        |  AF12
|  DQ24        | FMC_D24      | PI0/PG13    |  AF12
|  DQ25        | FMC_D25      | PI1/PG14    |  AF12
|  DQ26        | FMC_D26      | PI2         |  AF12
|  DQ27        | FMC_D27      | PI3         |  AF12
|  DQ28        | FMC_D28      | PI6         |  AF12
|  DQ29        | FMC_D29      | PI7         |  AF12
|  DQ30        | FMC_D30      | PI9         |  AF12
|  DQ31        | FMC_D31      | PI10        |  AF12

\*: Bank 2 clock and chip enable

#### Timing parameters (in clock units)  

| Field   |   Description
|---------|----------------------------------------------------
| TMRD    | Load Mode Register to Active
| TXSR    | Exit self refresh delay
| TRAS    | Self refresh time, i.e., the minimum Self-refresh period
| TRC     | Row cycle delay. i.e. the delay between the Refresh command and the Activate command
| TWR     | Recovery delay, i.e., delay between a Write and a Precharge command
| TRP     | Row precharge delay, i.e., delay between a Precharge command and another command
| TRCD    | Row to column delay


#### Timing information for f_SDCLK = 100 MHz  

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


#### FMC Initialization Sequence

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

### SDRAM


The SDRAM device used is a MT48LC4M32B2B5-6A, manufactured by Micron.

Only 16 bits of data bus (DQ15..0) )are used. So it is then not neccessary to
use full DQM. Only DQM0 and DQM1 need to be used.

It is a PC166/PC100, i.e., can work with a 100/166 MHz clock.


#### SDRAM Information


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

The address pins are used to row and column addressing. Bits A7..0 address the column and A10:0 
address the row. A10 is used to control the precharge.

#### SDRAM Characteristics  

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

#### SDRAM Configuration


| Command                         | Description
|---------------------------------|------------------------------------------------
| COMMAND INHIBIT (NOP)           |
| NO OPERATION (NOP)              | 
| ACTIVE                          | Select bank and activate row
| READ                            | Select bank and column, and start READ burst
| WRITE                           | Select bank and column, and start WRITE burst
| BURST TERMINATE                 |
| Precharge                       | Deactivate row in  bank or banks
| AUTO REFRESH or SELF REFRESH    | Enter self refresh mode
| LOAD MODE REGISTER              |

The MODE register is a 12-bit register and has the following format.

|  Field |  Name       | Description
|--------|-------------|-----------------------------------------------------------
|  12-10 |  000        | Reserved
|   9- 9 |   WB        | Single Location Access (1)/Programmed Burst Length (0)   
|   8- 7 | Op Mode     | Operating Mode: Standard Mode (00), Reserved (all others)
|   6- 4 | CAS Latency | 1, 2, or 3, others reserved
|   3- 3 | Burst Type  | Sequential (0), Interleaved (1) 
|   2- 0 | Burst Length| 2^n, n={0,1,2,3 or Full Page (7) according Burst type



To emmit a command, there is a signaling envolving all SDRAM signals (CS,RAS,CAS,WE, ADDR, DQM, DQ)


| Name (Function)              | CS# | RAS# | CAS# | WE# | DQM | ADDR     |  DQ    |
|------------------------------|-----|------|------|-----|-----|----------|--------|
| COMMAND INHIBIT (NOP)        |  H  |   X  |   X  |   X |  X  |    X     |    X   | 
| NO OPERATION (NOP)           |  L  |   H  |   H  |   H |  X  |    X     |    X   |
| ACTIVE                       |  L  |   L  |   H  |   H |  X  | Bank/row |    X   | 
| READ                         |  L  |   H  |   L  |   H | L/H | Bank/col |    X   |
| WRITE                        |  L  |   H  |   L  |   L | L/H | Bank/col | Valid  |
| BURST TERMINATE              |  L  |   H  |   H  |   L |  X  |    X     | Active |
| PRECHARGE                    |  L  |   L  |   H  |   L |  X  | Code     |    X   |
| AUTO REFRESH or SELF REFRESH |  L  |   L  |   L  |   H |  X  |    X     |    X   |
| LOAD MODE REGISTER           |  L  |   L  |   L  |   L |  X  | Op-code  |    X   |
| Write enable/output enable   |  X  |   X  |   X  |   X |  L  |    X     | Active |
| Write inhibit/output High-Z  |  X  |   X  |   X  |   X |  H  |    X     | High-Z |

#### SDRAM Initialization

The steps described below must be strictly followed to get the SDRAM working.

Power up (VDD and VDDQ)
Clock stable
COMMAND INHIBIT (before 100 us after clock stable)
PRECHARGE all banks
Get to idle state
AUTO REFRESH (two full cycles)
LOAD MODE REGISTER
AUTO REFRESH (two full cycles)

Detailed list of steps

1. Simultaneously apply power to V DD and V DDQ .
2. Assert and hold CKE at a LVTTL logic LOW.
3. Provide stable CLOCK signal.
4. Starting at some point during this 100˩s period, bring CKE HIGH. Continuing at
least through the end of this period, 1 or more COMMAND INHIBIT or NOP com-
mands must be applied.
5. Perform a PRECHARGE ALL command.
6. Wait at least t_RP time; during this time NOPs or DESELECT commands must be
given. 
7. Issue an AUTO REFRESH command.
8. Wait at least t RFC time, during which only NOPs or COMMAND INHIBIT com-
mands are allowed.
9. Issue an AUTO REFRESH command.
10. Wait at least t RFC time, during which only NOPs or COMMAND INHIBIT com-
mands are allowed.
12. The SDRAM is now ready for mode register programming. Outputs are guaranteed High-Z after
 the LMR command is issued. Outputs should be High-Z already before the LMR command is issued.
13. Wait at least t MRD time, during which only NOP or DESELECT commands are allowed.



#### FMC Configuration of the MT48LC4M32B2

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
3. [STM32F75xxx and STM32F74xxx advanced Arm ® -based 32-bit MCUs Reference Manual - RM0385](https://www.st.com/resource/en/reference_manual/dm00124865-stm32f75xxx-and-stm32f74xxx-advanced-arm-based-32-bit-mcus-stmicroelectronics.pdf)



