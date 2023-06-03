/**
 * @file     stm32l476-disco.ld
 * @brief    loader script compatible with CMSIS
 * @version  V1.1
 * @date     06/30/2021
 *
 * @author   Hans
 *
 * @note    Not tested with C++
 */


 /**
 * @note    Memory map for STM32F746NG Flash memory. The same memory can be 
 *          accessed thru two different buses.
 *
 * ITCMFLASH    | 1 MB    | 0x0020_0000-0x002F_FFFF
 * AXIMFLASH    | 1 MB    | 0x0800_0000-0x080F_FFFF
 *
 *
 * @note    Memory map for STM32F746NG RAM memory
 *
 *  DTCMRAM     |  64 KB | 0x2000_0000-0x2000_FFFF     |  o-wait states
 *  SRAM1       | 240 KB | 0x2001_0000-0x2004_BFFF     |
 *  SRAM2       |  16 KB | 0x2004_C000-0x2004_FFFF
 *  Subtotal    | 320 KB |
 *  ITCMRAM     |  16 KB | 0x0000_0000-0x0000_3FFF
 *  BACKUPSRAM  |   4 KB | 0x4002_4000-0x4002_4XXX
 *  Total       | 340 KB |
 *
 * @note    DTCMRAM+SRAM1+SRAM2 forms a 320 KB contiguous area
 *
 * @note    External memories of STM32F746G Discovery Board
 *
 * SDRAM        | 8 MB   | 0xC000_0000-0xC07F_FFFF
 * QSPIFLASH    | 16 MB  | 0x9000_0000-0x90FF_FFFF (QSPI)
 *
 ******************************************************************************/


MEMORY
{
    /* Choose one of them and rename to FLASH
     *ITCMFLASH (rx)   : ORIGIN = 0x00200000, LENGTH = 1024K
     *AXIMFLASH (rx)   : ORIGIN = 0x08000000, LENGTH = 1024K
     */
    FLASH (rx)         : ORIGIN = 0x00200000, LENGTH = 1024K
    /* System RAM (Contiguous)
     *DTCMRAM (rwx)    : ORIGIN = 0x20000000, LENGTH = 64K
     *SRAM1 (rwx)      : ORIGIN = 0x20010000, LENGTH = 240K
     *SRAM2 (rwx)      : ORIGIN = 0x2004C000, LENGTH = 16K
     */
    DTCMRAM (rwx)      : ORIGIN = 0x20000000, LENGTH = 64K
    SRAM (rwx)         : ORIGIN = 0x20010000, LENGTH = 256K
    /* Extra RAM */
    ITCMRAM (rwx)      : ORIGIN = 0x00000000, LENGTH = 16K
    BACKUPRAM (rwx)    : ORIGIN = 0x40024000, LENGTH = 4K
    /* External memories on the discovery board */
    SDRAM(rwx)         : ORIGIN = 0xC0000000, LENGTH = 8M
    QSPIFLASH(rx)      : ORIGIN = 0x90000000, LENGTH = 16M
    

};

/* definition of memory areas */
_ram_start             =     ORIGIN(SRAM);
_ram_end               =     ORIGIN(SRAM) + LENGTH(SRAM)-1;
_flash_start           =     ORIGIN(FLASH);
_flash_end             =     ORIGIN(FLASH) + LENGTH(FLASH)-1;
_sdram_start           =     ORIGIN(SDRAM);
_sdram_end             =     ORIGIN(SDRAM) + LENGTH(SDRAM)-1;
_qspiflash_start       =     ORIGIN(QSPIFLASH);
_qspiflash_end         =     ORIGIN(QSPIFLASH) + LENGTH(QSPIFLASH)-1;
/* Stack definition. Actually not neccessary    */
STACK_SIZE             =     4K;
STACK_BASE             =     ORIGIN(SRAM) + LENGTH(SRAM) - STACK_SIZE;
STACK_END              =     ORIGIN(SRAM) + LENGTH(SRAM) - 4;
_stack_start           =     STACK_BASE;
_stack_end             =     STACK_END; /* Initial value */
_stack_init            =     STACK_END;
/* Heap definition. Actually not used (yet?)     */
HEAP_SIZE              =     0x400;


/*
 * Standard Sections for C
 * .text        : Instructions (code area)
 * .data        : Initialized data Must be stored in flash and moved to RAM
 * .bss         : Non initialized data
 * .stack       : Just a pointer to end of RAM (The stack grows downward)
 *
 * Non standard sections for C
 * .isr_vector  : non standard section to make the vector table appear
 *                at the begin of RAM
 *
 * .sdram       : area in external SDRAM (only non initialized data)
 *
 * There are additional sectors for C++ (Not tested)
 */

SECTIONS
{
  _text       = ORIGIN(FLASH);
  _text_start = ORIGIN(FLASH);     /* Remember start of text (code area) */
  .text :   
  {   
    KEEP(*(.isr_vector))           /* Must appear at the beginning */
      .            = ALIGN(4);   
      *(.text*)                    /* Instructions follow */
      .            = ALIGN(4);   
      *(.rodata*)                  /* Constants follow immediatly */
      .            = ALIGN(4);   
   
  } > FLASH                        /* All in flash memory */
  .                    = ALIGN(4);   
  _text_end            = .;        /* Remember end of text */
  _etext               = .;        /* Alias */


  /*
    * Initialized data must be in RAM but the initial values must be stored in flash
    * and copied to RAM at start of execution
    *
    * The specification > SRAM AT>FLASH tells the linker to put a copy in the flash
    */

  .data :
  {
    .            = ALIGN(4);
    _data        = .;
    _data_start  = .;            /* remember start of data area */
    *(.data*)
    .            = ALIGN(4);
    *(vtable)                    /* vtables are used by C++ */
    .            = ALIGN(4);
    _data_end    = .;            /* remember end of data area */
    _edata       = .;            /* alias for data_end  */
  } > SRAM  AT>FLASH             /* linked for RAM but with a copy in flash

  /*
    * Non initialized data is in RAM.
    * Must be zeroed at startup
    */
  .bss :
  {
    .            = ALIGN(4);
    _bss         = .;
    _bss_start   = .;            /* remember start of bss area */
    *(.bss.*)                    /* non initialized data */
    *(COMMON)                    /* maybe fortran (not tested) */
    .            = ALIGN(4);
    _bss_end     = .;            /* remember end of area */
    _ebss        = .;            /* alias for bss_end */
    HEAP_START   = .;
  } > SRAM

  /*
    * only non initialized data in external sdram (EXTRAM)
    */
  .sdram (NOLOAD) :
  {
    .              = ALIGN(4);
    _sdram         = .;
    _sdrambss_start   = .;          /* remember start of sdram area */
    *(.sdram*)                      /* non initialized data */
    .              = ALIGN(4);
    _sdrambss_end     = .;          /* remember end of area */
    _esdram        = .;          /* alias for _sdram_end */
  } > SDRAM

  .dtcmram (NOLOAD) :
  {
    .              = ALIGN(4);
    _dtcm_start    = .;
    *(.dtcm.*)
    .              = ALIGN(4);
    _dtcm_end      = .;
    _dtcmram       = .;
  } > DTCMRAM

  .itcmram (NOLOAD) :
  {
    .              = ALIGN(4);
    _itcm_start    = .;
    *(.itcm.*)
    .              = ALIGN(4);
    _itcm_end      = .;
    _itcmram       = .;
  } > ITCMRAM

  /*
    * Stack
    */
  .stack :
  {

  } > SRAM


}

