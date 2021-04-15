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
#include "conversions.h"


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


static const uint32_t uartconfig =  UART_NOPARITY | UART_8BITS | UART_STOP_2 |
                                    UART_BAUD_9600;



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




/**
 * @brief   WriteValue
 *
 */
void WriteValue(int uart, char *label, unsigned value) {
char s[30];

    IntToString(value,s);
    UART_WriteString(uart,label);
    UART_WriteString(uart,s);
    UART_WriteString(uart,"\n\r");
}

void WriteHexValue(int uart, char *label, unsigned value) {
char s[30];

    IntToHexString(value,s);
    UART_WriteString(uart,label);
    UART_WriteString(uart,s);
    UART_WriteString(uart,"\n\r");
}

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

/**
 * @brief   main
 *
 * @note    Initializes GPIO and blinks LED
 *
 */

int main(void) {
uint32_t v;
Uid u;
uint32_t ramused,flashused;

#if 1
    /* configure clock to 200 MHz */
    SystemSetCoreClock(CLOCKSRC_PLL,1);
#else
    SystemConfigMainPLL(&Clock200MHz);
    SystemSetCoreClock(CLOCKSRC_PLL,1);
#endif
    SysTick_Config(SystemCoreClock/1000);

    LED_Init();

    UART_Init(UART_1,uartconfig);


    UART_WriteString(UART_1,"\n\r\n\r******************************************\n\r");
    UART_WriteString(UART_1,"Information\n\r");

    v = GetModel();
    WriteHexValue(UART_1,"Model:        ",v);

    v = GetFlashSize();
    WriteValue(UART_1,"Flash size:   ",v);

    v = (char *) &_ram_end - (char *) &_ram_start;
    WriteValue(UART_1,"RAM size:     ",v);

    u = GetCPUId();
    WriteHexValue(UART_1,"XY Position:  ",u.xy);
    WriteHexValue(UART_1,"Lot #:        ",u.lot);
    WriteHexValue(UART_1,"Wafer #:      ",u.waf);

    WriteValue(UART_1,"Core Clock Frequency (Hz):   ",SystemCoreClock);
    WriteValue(UART_1,"SYSCLK Clock Frequency (Hz): ",SystemGetSYSCLKFrequency());
    WriteValue(UART_1,"AHB Clock Frequency (Hz):    ",SystemGetAHBFrequency());
    WriteValue(UART_1,"APB1 Clock Frequency (Hz):   ",SystemGetAPB1Frequency());
    WriteValue(UART_1,"APB2 Clock Frequency (Hz):   ",SystemGetAPB2Frequency());

    v = (uint32_t) ((char *) &_flash_start);
    WriteHexValue(UART_1,"Flash start:  ",v);
    v = (uint32_t) ((char *) &_flash_end);
    WriteHexValue(UART_1,"Flash end:    ",v);

    v = (uint32_t) ((char *) &_ram_start);
    WriteHexValue(UART_1,"RAM start:    ",v);
    v = (uint32_t) ((char *) &_ram_end);
    WriteHexValue(UART_1,"RAM end:      ",v);

    ramused = (char *) &_bss_end - (char *) &_data_start;
    WriteValue(UART_1,"RAM used:     ",ramused);
    flashused = (char *) &_text_end - (char *) &_text_start;
    WriteValue(UART_1,"Flash used:   ",flashused);


    v = (uint32_t) ((char *) &_text_start);
    WriteHexValue(UART_1,"Code start:   ",v);
    v = (uint32_t) ((char *) &_text_end);
    WriteHexValue(UART_1,"Code end:     ",v);

    v = (uint32_t) ((char *) &_data_start);
    WriteHexValue(UART_1,"Data start:   ",v);
    v = (uint32_t) ((char *) &_data_end);
    WriteHexValue(UART_1,"Data end:     ",v);

    v = (uint32_t) ((char *) &_bss_start);
    WriteHexValue(UART_1,"BSS start:    ",v);
    v = (uint32_t) ((char *) &_bss_end);
    WriteHexValue(UART_1,"BSS end:      ",v);



    for(;;) {}
}
