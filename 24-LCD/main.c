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

#define DO_NOT_STOP

/*
 * @brief   prints a message and waits for ENTER to continue
 */

void messagewithconfirm(char *s) {
int c;
    if( ! verbose )
        return;

#ifdef DO_NOT_STOP
    fputs("Now ",stdout);
    fputs(s,stdout);
    ms_delay(10);
    putchar('\n');
#else
    fputs("Press ENTER to ");
    fputs(s,stdout);
    while( (c=getchar())!='\n' ) {}
#endif
}


/*
 * @brief   prints a message
 */

int message(char *s) {
int c;
    if( verbose )
        puts(s);
}

void printlayerinfo( int layer ) {
LTDC_Layer_TypeDef *p;

    if( layer == 1 )       p = LTDC_Layer1;
    else if ( layer == 2 ) p = LTDC_Layer2;
    else return;

    printf("Info about layer at address %p\n",p);
    printf("CR       = %08X\n",p->CR);
    printf("WHPCR    = %08X\n",p->WHPCR);
    printf("WVPCR    = %08X\n",p->WVPCR);
    printf("CKCR     = %08X\n",p->CKCR);
    printf("PFCR     = %08X\n",p->PFCR);
    printf("CACR     = %08X\n",p->CACR);
    printf("DCCR     = %08X\n",p->DCCR);
    printf("BFCR     = %08X\n",p->BFCR);
    printf("CFBAR    = %08X\n",p->CFBAR);
    printf("CFBLR    = %08X\n",p->CFBLR);
    printf("CFBLNR   = %08X\n",p->CFBLNR);
    printf("CLUTWR   = %08X\n",p->CLUTWR);

}

void printlayercontents(int layer) {
LTDC_Layer_TypeDef *p;
int  ps;
char *area,*lineaddr;
int  w,h,pitch;
int i,j;
char *q;

    if( layer == 1 )       p = LTDC_Layer1;
    else if ( layer == 2 ) p = LTDC_Layer2;
    else return;

    ps     = LCD_GetPixelSize(layer);
    area   = (char *) LCD_GetFrameBufferAddress(layer);
    w      = LCD_GetWidth(layer);
    h      = LCD_GetHeight(layer);
    pitch  = LCD_GetPitch(layer);

    for(i=0;i<h;i+=100) {
        lineaddr = (char *) LCD_GetLineAddress(layer,i);
        printf("%p:",lineaddr);
        for(j=0;j<8;j++)
            printf("%02X",lineaddr[j]&0xFF);
        putchar(' ');
        for(j=pitch-8;j<pitch;j++)
            printf("%02X",lineaddr[j]&0xFF);

        putchar('\n');

    }

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
void *fbarea1;
void *fbarea2;
int format = LCD_FORMAT_RGB888;

    message("Initializing LED");
    LED_Init();

    message("Setting clock to operating frequency");
    SystemConfigMainPLL(&MainPLLConfiguration_200MHz);
    SystemSetCoreClock(CLOCKSRC_PLL,1);
    printf("Frequency is now %d Hz\n",SystemCoreClock);

    messagewithconfirm("turn OFF backlight without LCD initialization");
    LCD_TurnBacklightOff();

    messagewithconfirm("initialize LCD");
    LCD_Init();

    messagewithconfirm("turn OFF backlight");
    LCD_TurnBacklightOff();

    message("Initializing SDRAM");
    SDRAM_Init();

    message("Writing 0x12345678 to SDRAM");
    *((uint32_t *) 0xc0000000) = 0x12345678;

    message("Reading from SDRAM");
    uint32_t r = *((uint32_t *) 0xc0000000);
    printf("Read 0x%0x\n",r);


    message("Initializing buddy allocator");
    Buddy_Init((char*) SDRAM_ADDRESS,SDRAM_SIZE,4096);

    messagewithconfirm("enable controller");
    LCD_EnableController();

    messagewithconfirm("turn ON backlight");
    LCD_TurnBacklightOn();


    messagewithconfirm("enter in standby");
    LCD_PutDisplayStandBy();

    messagewithconfirm("enter normal operation");
    LCD_PutDisplayOperation();

    messagewithconfirm("get the frame buffer size");
    fbsize = LCD_GetMinimalFullFrameBufferSize(format);
    printf("Minimal size is %d\n",fbsize);

    messagewithconfirm("allocate area for frame buffer 1");
    fbarea1 = Buddy_Alloc(fbsize);
    printf("Allocated at address %p\n",fbarea1);

    messagewithconfirm("set background color");
    LCD_SetBackgroundColor(RGB(255,0,255));

    messagewithconfirm("set the frame buffer of layer 1");
    LCD_SetFullSizeFrameBuffer(1, fbarea1, format);
    LCD_FillFrameBuffer(1,RGB(0,255,255));
    printlayerinfo(1);

    messagewithconfirm("enable it");
    LCD_EnableLayer(1);
    printlayerinfo(1);

#define H2    48
#define W2    32
#define PS2   3
#define P2    (((((W2)*PS2+3)+63)/64)*64)
    messagewithconfirm("allocate area for frame buffer 2");
    fbarea2 = Buddy_Alloc(P2*H2);
    printf("Allocated %d bytes at address %p\n",P2*H2,fbarea2);

    messagewithconfirm("set the frame buffer of layer 2");
    int w2 = W2;
    int h2 = H2;
    int p2 = P2;
    int x = 96;
    int y = 64;
    LCD_SetFrameBuffer(2, fbarea2, LCD_FORMAT_RGB888, x, y, w2, h2, p2 );
    LCD_FillFrameBuffer(2,RGB(255,255,0));
    printlayerinfo(2);
    printlayercontents(2);


    messagewithconfirm("come back to layer 1");
    LCD_DisableLayer(2);
    LCD_EnableLayer(1);

    /*
     * Show some screens
     */


    for (;;) {
        messagewithconfirm("make layer 1 all GRAY");
        LCD_FillFrameBuffer(1,RGB(127,127,127));
        LCD_ReloadLayerByVerticalBlanking(1);
        printlayerinfo(1);
        printlayercontents(1);

        messagewithconfirm("make layer 1 all WHITE");
        LCD_FillFrameBuffer(1,RGB(255,255,255));
        LCD_ReloadLayerByVerticalBlanking(1);
        printlayerinfo(1);
        printlayercontents(1);

        messagewithconfirm("make layer 1 all BLACK");
        LCD_FillFrameBuffer(1,RGB(0,0,0));
        LCD_ReloadLayerByVerticalBlanking(1);
        printlayercontents(1);

        messagewithconfirm("make layer 1 all RED");
        LCD_FillFrameBuffer(1,RGB(255,0,0));
        LCD_ReloadLayerByVerticalBlanking(1);
        printlayercontents(1);

        messagewithconfirm("make layer 1 all GREEN");
        LCD_FillFrameBuffer(1,RGB(0,255,0));
        LCD_ReloadLayerByVerticalBlanking(1);

        messagewithconfirm("make layer 1 all BLUE");
        LCD_FillFrameBuffer(1,RGB(0,0,255));
        LCD_ReloadLayerByVerticalBlanking(1);

        messagewithconfirm("make layer 1 all YELLOW");
        LCD_FillFrameBuffer(1,RGB(255,255,0));
        LCD_ReloadLayerByVerticalBlanking(1);

        messagewithconfirm("make layer 1 all MAGENTA");
        LCD_FillFrameBuffer(1,RGB(255,0,255));
        LCD_ReloadLayerByVerticalBlanking(1);

        messagewithconfirm("make layer 1 all CYAN");
        LCD_FillFrameBuffer(1,RGB(0,255,255));
        LCD_ReloadLayerByVerticalBlanking(1);

        messagewithconfirm("draw horizontal layer in BLACK");
        LCD_DrawHorizontalLine(1,30,60,30,RGB(0,0,0));
        LCD_ReloadLayerByVerticalBlanking(1);

        messagewithconfirm("draw vertical layer in BLACK");
        LCD_DrawVerticalLine(1,30,60,60,RGB(0,0,0));
        LCD_ReloadLayerByVerticalBlanking(1);
        printlayerinfo(2);

        messagewithconfirm("swap layers");
        LCD_SwapLayers();
        LCD_ReloadLayerByVerticalBlanking(1);

        messagewithconfirm("make layer 2 transparent");
        LCD_SetLayerOpacity(2,0);
        LCD_ReloadLayerByVerticalBlanking(1);

        messagewithconfirm("make layer 2 opaque");
        LCD_SetLayerOpacity(2,255);
        LCD_ReloadLayerByVerticalBlanking(1);

        messagewithconfirm("swap layers again");
        LCD_SwapLayers();
        LCD_ReloadLayerByVerticalBlanking(1);

        messagewithconfirm("draw a box in RED");
        LCD_DrawBox(1,120,80,40,60,RGB(255,0,0),RGB(0,0,0));
        LCD_ReloadLayerByVerticalBlanking(1);

        messagewithconfirm("draw a inclined line in RED");
        LCD_DrawLine(1,120,80,-40,-60,RGB(0,0,0));
        LCD_ReloadLayerByVerticalBlanking(1);

    }
}
