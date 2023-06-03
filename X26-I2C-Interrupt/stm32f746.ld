/**
 * @file     stm32l476.ld
 * @brief    loader script compatible with CMSIS
 * @version  V1.0
 * @date     05/10/2020
 *
 * @author   Hans
 *
 * @note    Not tested with C++
 */


 /**
 * @note    Memory map for STM32F746NG RAM memory
 *
 *  DTCMRAM     |  64 KB | 0x2000_0000-0x2000_FFFF
 *  SRAM1       | 240 KB | 0x2001_0000-0x2004_BFFF
 *  SRAM2       |  16 KB | 0x2004_C000-0x2004_FFFF
 *  Subtotal    | 320 KB |
 *  ITCMRAM     |  16 KB | 0x0000_0000-0x0000_3FFF
 *  BACKUPSRAM  |   4 KB | 0x4002_4000-0x4002_4XXX
 *  Total       | 340 KB |
 *
 * @note    DTCMRAM+SRAM1+SRAM2 forms a 320 KB contiguous area
 *
 * @note Memory map for STM32F746NG Flash memory
 *
 * ITCMFLASH    | 1 MB    | 0x0020_0000-0x002F_FFFF
 * AXIMFLASH    | 1 MB    | 0x0800_0000-0x080F_FFFF
 *
 * @note    This is the same memory accessed thru different buses
 *
 ******************************************************************************/


MEMORY
{
    /* Choose one of them and rename to FLASH
     *ITCMFLASH (rx)   : ORIGIN = 0x00200000, LENGTH = 1024K
     *AXIMFLASH (rx)   : ORIGIN = 0x08000000, LENGTH = 1024K
    */
    FLASH (rx)         : ORIGIN = 0x00200000, LENGTH = 1024K
    /* Contiguous RAM
     *DTCMRAM (rwx)    : ORIGIN = 0x20000000, LENGTH = 64K
     *SRAM1 (rwx)      : ORIGIN = 0x20010000, LENGTH = 240K
     *SRAM2 (rwx)      : ORIGIN = 0x2004C000, LENGTH = 16K
     */
    SRAM (rwx)         : ORIGIN = 0x20000000, LENGTH = 320K
    /* Extra RAM */
    ITCMRAM (rwx)    : ORIGIN = 0x00000000, LENGTH = 16K
    BACKUPRAM (rwx)  : ORIGIN = 0x40024000, LENGTH = 4K

};


_ram_start   = ORIGIN(SRAM);
_ram_end     = ORIGIN(SRAM) + LENGTH(SRAM)-1;
_flash_start = ORIGIN(FLASH);
_flash_end   = ORIGIN(FLASH) + LENGTH(FLASH)-1;

STACK_SIZE   = 4K;
STACK_BASE   = ORIGIN(SRAM) + LENGTH(SRAM) - STACK_SIZE;
STACK_END    = ORIGIN(SRAM) + LENGTH(SRAM) - 4;
HEAP_SIZE    = 0x400;
_stack_start = STACK_BASE;
_stack_end   = STACK_END; /* Initial value */
_stack_init  = STACK_END;

/*
 * Sections for C
 * .text        : instructions
 * .data        : initialized data Must be stored in flash and moved to RAM
 * .bss         : non initialized data
 * .stack       : just a pointer to end of RAM (Stack grows downward)
 *
 *  isr_vector  : Non standard section to make the vector table appear at the begin of RAM
 *
 * There are additional sectior for C++ (Not tested)
 *
 *
 */

SECTIONS
{
  _text       = ORIGIN(FLASH);
  _text_start = ORIGIN(FLASH);      /* remember start of text (instructions) */
    .text :
    {

     KEEP(*(.isr_vector))           /* Must appear at the beginning */
          .           = ALIGN(4);
          *(.text*)                 /* Instructions follow */
          .           = ALIGN(4);
          *(.rodata*)               /* Constants follow immediatly */
          .           = ALIGN(4);

    } > FLASH                       /* All in flash memory */
  .           = ALIGN(4);
  _text_end   = .;                  /* Remember end of text */
  _etext      = .;


    /*
     * Initialized data must be in RAM but the initial values must be stored in flash
     * and copied to RAM at start of execution
     *
     * The specification > SRAM AT>FLASH tells the linker to put a copy in the flash
     */

    .data :
    {
          .           = ALIGN(4);
          _data       = .;
          _data_start = .;          /* remember start of data area */
          *(.data*)
          .           = ALIGN(4);
          *(vtable)                 /* vtables are used by C++ */
          _data_end   = .;          /* remember end of data area */
          _edata      = .;

    } > SRAM  AT>FLASH              /* linked for RAM but with a copy in flash

    /*
     * Non initialized data is in RAM.
     * Must be zeroed at startup
     */
    .bss :
    {
          .           = ALIGN(4);
        _bss          = .;
        _bss_start    = .;          /* remember start of bss area */
        *(.bss.*)                   /* non initialized data */
        *(COMMON)                   /* maybe fortran (not tested) */
        _bss_end =      .;          /* remember end of area */
        _ebss         = .;
        HEAP_START = .;
    } > SRAM

    /*
     * Stack
     */
    .stack :
    {

    } > SRAM


}

