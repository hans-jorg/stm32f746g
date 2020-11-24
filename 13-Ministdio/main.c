/**
 * @file     main.c
 * @brief    Blink LEDs using Systick and CMSIS (Heavy use of macros)
 * @version  V1.0
 * @date     06/10/2020
 *
 * @note     The blinking frequency depends on core frequency
 * @note     Direct access to registers
 * @note     No library used
 *
 *
 ******************************************************************************/

#include "stm32f746xx.h"
#include "system_stm32f746.h"
#include "led.h"
#include "uart.h"
#include "ministdio.h"


/**
 * @brief   Systick routine
 *
 * @note    It is called every 1ms
 *
 */

static volatile uint32_t tick_ms = 0;
static volatile uint32_t delay_ms = 0;

#define INTERVAL 500
void SysTick_Handler(void) {

    if( tick_ms >= INTERVAL ) {
       LED_Toggle();
       tick_ms = 0;
    } else {
       tick_ms++;
    }

    if( delay_ms > 0 ) delay_ms--;

}

void Delay(uint32_t delay) {

    delay_ms = delay;
    while( delay_ms ) {}

}

static PLL_Configuration Clock200MHz = {
    .source = CLOCKSRC_HSE,
    .M = HSE_OSCILLATOR_FREQ/1000000,       // f_INT = 1 MHz
    .N = 400,                               // f_VCO = 400 MHz
    .P = 2,                                 // f_OUT = 200 MHz
    .Q = 2,                                 // not used
    .R = 2                                  // not used
};

/**
 * @brief   Communication parameters
 */
static const uint32_t uartconfig =  UART_NOPARITY | UART_8BITS | UART_STOP_2 |
                                    UART_BAUD_9600;

/**
 * @brief   Linker information
 */
///@{
extern unsigned long _text_start;
extern unsigned long _text_end;
extern unsigned long _data_start;
extern unsigned long _data_end;
extern unsigned long _bss_start;
extern unsigned long _bss_end;
extern unsigned long _stack_start;
extern unsigned long _ram_start;
extern unsigned long _ram_end;
extern unsigned long _flash_start;
extern unsigned long _flash_end;
///@}

/*
 * @brief Get CPU Info
 */
///@{
uint32_t GetFlashSize(void) {
uint32_t size;

    size = *(uint16_t *) 0x1FF0F442;    // size in KB
    size *= 1024;

    return size;
}

typedef struct {
    uint32_t    xy;
    uint32_t    lot;
    uint32_t    waf;
} Uid;

Uid GetCPUId(void) {
Uid u;

    u.xy  = *(uint32_t *) 0x1FF0F420;    // Factory programmed
    u.lot = *(uint32_t *) 0x1FF0F424;    // Factory programmed
    u.waf = *(uint32_t *) 0x1FF0F428;    // Factory programmed

    return u;
}

uint32_t GetModel(void) {
uint32_t v;

    v = *(uint32_t *) 0xE0042000;       // DBGMCU_IDCODE
    return v;
}
///@}

/**
 * @brief   Interface to ministdio
 *
 * @note    All input/output go thru getchar/putchar functions
 */

int getchar(void) { return UART_ReadChar(UART_1); }
void putchar(char c) { UART_WriteChar(UART_1,c); }

/**
 * @brief   main
 *
 * @note    Initializes GPIO and blinks LED
 *
 *
 */

int main(void) {
uint32_t v;
Uid u;
uint32_t ramused,flashused;

    /* configure clock to 200 MHz */
    SystemSetCoreClock(CLOCKSRC_PLL,1);

    SysTick_Config(SystemCoreClock/1000);

    LED_Init();

    UART_Init(UART_1,uartconfig);


    printf("\n\r\n\r******************************************\n\r");
    printf("Information\n\r");

    v = GetModel();
    printf("Model:       %X\n",v);

    v = GetFlashSize();
    printf("Flash size:   %d\n",v);

    v = (char *) &_ram_end - (char *) &_ram_start;
    printf("RAM size:     %d\n",v);

    u = GetCPUId();
    printf("XY Position:  %x\n",u.xy);
    printf("Lot #:        %x\n",u.lot);
    printf("Wafer #:      %x\n",u.waf);

    printf("Core Clock Frequency (Hz):   %d\n",SystemCoreClock);
    printf("SYSCLK Clock Frequency (Hz): %d\n",SystemGetSYSCLKFrequency());
    printf("AHB Clock Frequency (Hz):    %d\n",SystemGetAHBFrequency());
    printf("APB1 Clock Frequency (Hz):   %d\n",SystemGetAPB1Frequency());
    printf("APB2 Clock Frequency (Hz):   %d\n",SystemGetAPB2Frequency());

    v = (uint32_t) ((char *) &_flash_start);
    printf("Flash start:  %x\n",v);
    v = (uint32_t) ((char *) &_flash_end);
    printf("Flash end:    %x\n",v);

    v = (uint32_t) ((char *) &_ram_start);
    printf("RAM start:    %x\n",v);
    v = (uint32_t) ((char *) &_ram_end);
    printf("RAM end:      %x\n",v);

    ramused = (char *) &_bss_end - (char *) &_data_start;
    printf("RAM used:     %d\n",ramused);
    flashused = (char *) &_text_end - (char *) &_text_start;
    printf("Flash used:   %d\n",flashused);


    v = (uint32_t) ((char *) &_text_start);
    printf("Code start:   %x\n",v);
    v = (uint32_t) ((char *) &_text_end);
    printf("Code end:     %x\n",v);

    v = (uint32_t) ((char *) &_data_start);
    printf("Data start:   %x\n",v);
    v = (uint32_t) ((char *) &_data_end);
    printf("Data end:     %x\n",v);

    v = (uint32_t) ((char *) &_bss_start);
    printf("BSS start:    %x\n",v);
    v = (uint32_t) ((char *) &_bss_end);
    printf("BSS end:      %x\n",v);



    for(;;) {}
}
