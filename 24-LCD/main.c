/**
 * @file     main.c
 * @brief    Blink LEDs using counting delays and CMSIS (Heavy use of macros)
 * @version  V1.0
 * @date     06/10/2020
 *
 * @note     The blinking frequency depends on core frequency
 * @note     Direct access to registers
 * @note     No library used
 *
 *
 ******************************************************************************/

#include <stdint.h>
#include <stdio.h>
#include "stm32f746xx.h"
#include "system_stm32f746.h"
#include "led.h"
#include "sdram.h"
#include "buddy.h"
#include "lcd.h"



#define OPERATING_FREQUENCY (200000000)

/**
 * @brief   Quick and dirty delay routine
 *
 * @note    It gives approximately 1ms delay at 16 MHz
 *
 * @note    The COUNTERFOR1MS must be adjusted by trial and error
 *
 * @note    Do not use this or similar in production code
 */

#define COUNTERFOR1MS 300000


void ms_delay(volatile int ms) {
   while (ms-- > 0) {
      volatile int x=COUNTERFOR1MS;
      while (x-- > 0)
         __NOP();
   }
}

/**
 * @brief   flag
 */
int verbose = 1;

/*
 * @brief   prints a message and waits for ENTER to continue
 */

void messagewithconfirm(char *s) {
int c;
    if( ! verbose )
        return;
    fputs(s,stdout);
    while( (c=getchar())!='\n' ) {}
}


/*
 * @brief   prints a message
 */

int message(char *s) {
int c;
    if( verbose )
        puts(s);
}


/**
 * @brief   main
 *
 * @note    Initializes GPIO and blinks LED
 *
 * @note    Really a bad idea to blink LED
 */

int main(void) {
int fbsize;
void *fbarea;
int format = LCD_FORMAT_RGB888;

    message("Initializing LED");
    LED_Init();

    message("Setting clock to operating frequency");
    SystemConfigMainPLL(&MainPLLConfiguration_200MHz);
    SystemSetCoreClock(CLOCKSRC_PLL,1);
    printf("Frequency is now %d Hz\n",SystemCoreClock);

    messagewithconfirm("Press ENTER to turn OFF backlight without LCD initialization");
    LCD_TurnBacklightOff();

    messagewithconfirm("Press ENTER to initialize LCD");
    LCD_Init();

    messagewithconfirm("Press ENTER to turn OFF backlight");
    LCD_TurnBacklightOff();

    message("Initializing SDRAM");
    SDRAM_Init();

    message("Writing 0x12345678 to SDRAM");
    *((uint32_t *) 0xc0000000) = 0x12345678;

    message("Reading from SDRAM");
    uint32_t r = *((uint32_t *) 0xc0000000);
    printf("Read %0x\n",r);


    message("Initializing buddy allocator");
    Buddy_Init((char*) SDRAM_ADDRESS,SDRAM_SIZE,8192);

    messagewithconfirm("Press ENTER to enable controller");
    LCD_EnableController();

    messagewithconfirm("Press ENTER to turn ON backlight");
    LCD_TurnBacklightOn();

    messagewithconfirm("Press ENTER to enter normal operation");
    LCD_PutDisplayOperation();

    messagewithconfirm("Press ENTER to get the frame buffer size");
    fbsize = LCD_GetMinimalFullFrameBufferSize(format);
    printf("Minimal size is %d\n",fbsize);

    messagewithconfirm("Press ENTER to allocate area");
    fbarea = Buddy_Alloc(fbsize);
    printf("Allocated at address %p\n",fbarea);

    messagewithconfirm("Press ENTER to set background color");
    LCD_SetBackgroundColor(RGB(255,0,255));

    messagewithconfirm("Press ENTER to set the frame buffer to layer 1");
    LCD_SetFullSizeFrameBuffer(1, fbarea, format);

    /*
     * Blink LCD
     */


    for (;;) {
        messagewithconfirm("Press ENTER to make layer 1 all RED");
        LCD_FillFrameBuffer(1,RGB(255,0,0));

        messagewithconfirm("Press ENTER to make layer 1 all GREEN");
        LCD_FillFrameBuffer(1,RGB(0,255,0));

        messagewithconfirm("Press ENTER to make layer 1 all BLUE");
        LCD_FillFrameBuffer(1,RGB(0,0,255));

        messagewithconfirm("Press ENTER to make layer 1 all YELLOW?");
        LCD_FillFrameBuffer(1,RGB(255,255,0));

        messagewithconfirm("Press ENTER to make layer 1 all YELLOW?");
        LCD_FillFrameBuffer(1,RGB(255,255,0));

        messagewithconfirm("Press ENTER to make layer 1 all MAGENTA?");
        LCD_FillFrameBuffer(1,RGB(255,0,255));

        messagewithconfirm("Press ENTER to make layer 1 all MAGENTA?");
        LCD_FillFrameBuffer(1,RGB(0,255,255));

    }
}
