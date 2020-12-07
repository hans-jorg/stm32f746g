/**
 * @file    lcd.c
 *
 * @date    07/10/2020
 * @author  Hans
 */

#include "stm32f746xx.h"
#include "system_stm32f746.h"
#include "gpio.h"
#include "lcd.h"


#define BIT(N) (1U<<(N))

/**
 * @brief   Characteristics of the LCD
 *
 * @note    Device Caracteristics from Rocktech RK043FN48H-CT672B Datasheet
 *
 *     Item               |  Min  |  Typ  |  Max  |  Units
 *   ---------------------|-------|-------|-------|--------------------
 *   DCLK Frequency       |    5  |    9  |   12  |  MHz
 *   DCLK Period          |   83  |  119  |  200  |  ns
 *   HSYNC Period         |  490  |  531  |  605  | DCLK Period
 *   HSYNC Display        |       |  480  |       | DCLK Period
 *   HSYNC Back porch     |    8  |   43  |       | DCLK Period
 *   HSYNC Front porch    |    2  |    1  |       | DCLK Period
 *   HSYNC pulse width    |    1  |       |       | DCLK Period
 *   VSYNC Period         |  275  |  288  |  335  | HSYNC Period
 *   VSYNC Display Period |       |  272  |       | HSYNC Period
 *   VSYNC Back porch     |    2  |   12  |       | HSYNC Period
 *   VSYNC Front porch    |    1  |    4  |       | HSYNC Period
 *   VSYNC pulse width    |    1  |   10  |       | HSYNC Period
 */


/* Units are DCLK = LCD_CLK Periods */
#define HSW             2
#define HBP            40
#define HFP             8
#define HAW           480

/* Units are HSYNC Periods */
#define VSW             2
#define VBP            12
#define VFP             4
#define VAH           272

#define  RK043FN48H_WIDTH    ((uint16_t)480)          /* LCD PIXEL WIDTH            */
#define  RK043FN48H_HEIGHT   ((uint16_t)272)          /* LCD PIXEL HEIGHT           */

/**
  * @brief  RK043FN48H Timing
  */
#define  RK043FN48H_HSYNC            ((uint16_t)41)   /* Horizontal synchronization */
#define  RK043FN48H_HBP              ((uint16_t)13)   /* Horizontal back porch      */
#define  RK043FN48H_HFP              ((uint16_t)32)   /* Horizontal front porch     */
#define  RK043FN48H_VSYNC            ((uint16_t)10)   /* Vertical synchronization   */
#define  RK043FN48H_VBP              ((uint16_t)2)    /* Vertical back porch        */
#define  RK043FN48H_VFP              ((uint16_t)2)    /* Vertical front porch       */

#define  RK043FN48H_FREQUENCY_DIVIDER    5            /* LCD Frequency divider      */



/**
 * @brief   LCD Connection
 *
 * LCD signal   | Board signal      | MCU signal             |
 *--------------|-------------------|------------------------|
 *  CLK         | LCD_CLK           | PI14                   |
 *  LCD_R       | LCD_R0-7          | PI15 PJ0-6             |
 *  LCD_G       | LCD_G0-7          | PJ7-11 PK0-2           |
 *  LCD_B       | LCD_B0-7          | PE4 PJ13-15 PG12 PK4-6 |
 *  HSYNC       | LCD_HSYNC         | PI10                   |
 *  VSYNC       | LCD_VSYNC         | PI9                    |
 *  DE          | LCD_DE            | PK7                    |
 *  INT         | LCD_INT           | PI13                   |
 *  SCL         | LCD_SCL           | PH7                    |
 *  SDA         | LCD_SDA           | PH8                    |
 *  SDA         | LCD_RST/NRST      | NRST                   |
 *
 *
 * Alternate Functions.(Table 12 of datasheet)
 *
 * LCD Signal   | ALT     |  Pin
 * -------------|---------|--------------------
 * LCD_CLK      |  AF14   | *PI14* PE14 PG7
 *
 * LCD_R0       |  AF14   | PG13 PH2 *PI15*
 * LCD_R1       |  AF14   | PA2 PH3 *PJ0*
 * LCD_R2       |  AF14   | PA1 PC10 PH8 *PJ1*
 * LCD_R3       |  AF14   | PH9 *PJ2* PB0 (AF9)
 * LCD_R4       |  AF14   | PA5 PA11 PH10 *PJ3*
 * LCD_R5       |  AF14   | PA12 PC0 PH11 *PJ4*
 * LCD_R6       |  AF14   | PA8 PH12 *PJ5* PB1(AF9)
 * LCD_R7       |  AF14   | PE15 PG6 *PJ6*
 *
 * LCD_G0       |  AF14   | PE5 *PJ7*
 * LCD_G1       |  AF14   | PE6 *PJ8*
 * LCD_G2       |  AF14   | PA6 PH13 *PJ9*
 * LCD_G3       |  AF14   | PE11 PH14 *PJ10* PG10(AF9)
 * LCD_G4       |  AF14   | PB10 PH15 *PJ11*
 * LCD_G5       |  AF14   | PB11 PI0 *PK0*
 * LCD_G6       |  AF14   | PC7 PI1 *PK1*
 * LCD_G7       |  AF14   | PD3 PI2 *PK2*
 * -            |   -     | ---
 * LCD_B0       |  AF14   | *PE4* PG14 PJ12
 * LCD_B1       |  AF14   | PG12 *PJ13*
 * LCD_B2       |  AF14   | PD6 PG10 *PJ14*
 * LCD_B3       |  AF14   | PD10 PG11 *PJ15*
 * LCD_B4       |  AF14   | PE12 PG12 *PG12(AF9)* PI4 PK3
 * LCD_B5       |  AF14   | PA3 PI5 *PK4*
 * LCD_B6       |  AF14   | PB8 PI6 *PK5*
 * LCD_B7       |  AF14   | PB9 PI7 *PK6*
 * -            |   -     | ---
 * LCD_VSYNC    |  AF14   | PA4 *PI9* PI13
 * LCD_HSYNC    |  AF14   | PC6 *PI10* PI12
 * LCD_DE       |  AF14   | PE13 PF10 *PK7*
 *
 * The following uses GPIO I/O or other modes
 *
 * LCD Signal    |        |  Pin   |  Description
 * --------------|--------|--------|------------------
 *  LCD_DISP     | GPIO   | I12    |  Enable LCD
 *  LCD_INT      | GPIO   | I13    |  LCD Interrupt
 *  LCD_BL_CTRL  | GPIO   | K3     |  Backlight PWM
 */

#ifdef FAST_INITIALIZATION

static void ConfigurePins(void) {
uint32_t mFIELD, mVALUE;

    /********* Configuring LCD signals *******************/

    /*
     * Port   GPIOE
     */
                // Configuring Mode
    mFIELD =    GPIO_MODER_MODER4_Msk;
    mVALUE =   (2<<GPIO_MODER_MODER4_Pos);
    GPIOE->MODER  = (GPIOE->MODER&~mFIELD)|mVALUE;

                // Configuring Alternate Function (Pins 7-0)
    mFIELD =    GPIO_AFRL_AFRL4_Msk;
    mVALUE =   (14<<GPIO_AFRL_AFRL4_Pos);
    GPIOE->AFR[0]  = (GPIOE->AFR[0]&~mFIELD)|mVALUE;

    /*
     * Port   GPIOG
     */
                // Configuring Mode
    mFIELD =    GPIO_MODER_MODER12_Msk;
    mVALUE =   (2<<GPIO_MODER_MODER12_Pos);
    GPIOG->MODER  = (GPIOG->MODER&~mFIELD)|mVALUE;

                // Configuring Alternate Function (Pins 15-8)
    mFIELD =    GPIO_AFRL_AFRL4_Msk;
    mVALUE =   (14<<GPIO_AFRL_AFRL4_Pos);  /////////
    GPIOG->AFR[1]  = (GPIOG->AFR[1]&~mFIELD)|mVALUE;

    /*
     * Port   GPIOI
     */
                // Configuring Mode
    mFIELD =   GPIO_MODER_MODER9_Msk
              |GPIO_MODER_MODER10_Msk
              |GPIO_MODER_MODER12_Msk
              |GPIO_MODER_MODER13_Msk
              |GPIO_MODER_MODER14_Msk
              |GPIO_MODER_MODER15_Msk;
    mVALUE =   (2<<GPIO_MODER_MODER9_Pos)
              |(2<<GPIO_MODER_MODER10_Pos)
              |(2<<GPIO_MODER_MODER12_Pos)
              |(2<<GPIO_MODER_MODER13_Pos)
              |(2<<GPIO_MODER_MODER14_Pos)
              |(2<<GPIO_MODER_MODER15_Pos);
    GPIOI->MODER  = (GPIOI->MODER&~mFIELD)|mVALUE;

                // Configuring Alternate Function (Pins 15-8)
    mFIELD =   GPIO_AFRL_AFRL1_Msk
              |GPIO_AFRL_AFRL2_Msk
              |GPIO_AFRL_AFRL4_Msk
              |GPIO_AFRL_AFRL5_Msk
              |GPIO_AFRL_AFRL6_Msk
              |GPIO_AFRL_AFRL7_Msk;
    mVALUE =   (14<<GPIO_AFRL_AFRL1_Pos)
              |(14<<GPIO_AFRL_AFRL2_Pos)
              |(14<<GPIO_AFRL_AFRL4_Pos)
              |(14<<GPIO_AFRL_AFRL5_Pos)
              |(14<<GPIO_AFRL_AFRL6_Pos)
              |(14<<GPIO_AFRL_AFRL7_Pos);
    GPIOI->AFR[1]  = (GPIOI->AFR[1]&~mFIELD)|mVALUE;

    /*
     * Port   GPIOJ
     */
                // Configuring Mode
    mFIELD =   GPIO_MODER_MODER0_Msk
              |GPIO_MODER_MODER1_Msk
              |GPIO_MODER_MODER2_Msk
              |GPIO_MODER_MODER3_Msk
              |GPIO_MODER_MODER4_Msk
              |GPIO_MODER_MODER5_Msk
              |GPIO_MODER_MODER6_Msk
              |GPIO_MODER_MODER7_Msk;

    mVALUE =   (2<<GPIO_MODER_MODER0_Pos)
              |(2<<GPIO_MODER_MODER1_Pos)
              |(2<<GPIO_MODER_MODER2_Pos)
              |(2<<GPIO_MODER_MODER3_Pos)
              |(2<<GPIO_MODER_MODER4_Pos)
              |(2<<GPIO_MODER_MODER5_Pos)
              |(2<<GPIO_MODER_MODER6_Pos)
              |(2<<GPIO_MODER_MODER7_Pos)
              |(2<<GPIO_MODER_MODER8_Pos)
              |(2<<GPIO_MODER_MODER9_Pos)
              |(2<<GPIO_MODER_MODER10_Pos)
              |(2<<GPIO_MODER_MODER11_Pos)
              |(2<<GPIO_MODER_MODER13_Pos)
              |(2<<GPIO_MODER_MODER14_Pos)
              |(2<<GPIO_MODER_MODER15_Pos);
    GPIOI->MODER  = (GPIOI->MODER&~mFIELD)|mVALUE;

                // Configuring Alternate Function (Pins 7-0)
    mFIELD =   GPIO_AFRL_AFRL0_Msk
              |GPIO_AFRL_AFRL1_Msk
              |GPIO_AFRL_AFRL2_Msk
              |GPIO_AFRL_AFRL3_Msk
              |GPIO_AFRL_AFRL4_Msk
              |GPIO_AFRL_AFRL5_Msk
              |GPIO_AFRL_AFRL6_Msk
              |GPIO_AFRL_AFRL7_Msk;
    mVALUE =   (14<<GPIO_AFRL_AFRL0_Pos)
              |(14<<GPIO_AFRL_AFRL1_Pos)
              |(14<<GPIO_AFRL_AFRL2_Pos)
              |(14<<GPIO_AFRL_AFRL3_Pos)
              |(14<<GPIO_AFRL_AFRL4_Pos)
              |(14<<GPIO_AFRL_AFRL5_Pos)
              |(14<<GPIO_AFRL_AFRL6_Pos)
              |(14<<GPIO_AFRL_AFRL7_Pos);
    GPIOJ->AFR[0]  = (GPIOJ->AFR[0]&~mFIELD)|mVALUE;

                // Configuring Alternate Function (Pins 15-8)
    mFIELD =   GPIO_AFRL_AFRL0_Msk
              |GPIO_AFRL_AFRL1_Msk
              |GPIO_AFRL_AFRL2_Msk
              |GPIO_AFRL_AFRL3_Msk
              |GPIO_AFRL_AFRL5_Msk
              |GPIO_AFRL_AFRL6_Msk
              |GPIO_AFRL_AFRL7_Msk;
    mVALUE =   (14<<GPIO_AFRL_AFRL0_Pos)
              |(14<<GPIO_AFRL_AFRL1_Pos)
              |(14<<GPIO_AFRL_AFRL2_Pos)
              |(14<<GPIO_AFRL_AFRL3_Pos)
              |(14<<GPIO_AFRL_AFRL5_Pos)
              |(14<<GPIO_AFRL_AFRL6_Pos)
              |(14<<GPIO_AFRL_AFRL7_Pos);
    GPIOJ->AFR[1]  = (GPIOJ->AFR[1]&~mFIELD)|mVALUE;

    /*
     * Port   GPIOK
     */
                // Configuring Mode
    mFIELD =   GPIO_MODER_MODER0_Msk
              |GPIO_MODER_MODER1_Msk
              |GPIO_MODER_MODER2_Msk
              |GPIO_MODER_MODER4_Msk
              |GPIO_MODER_MODER5_Msk
              |GPIO_MODER_MODER6_Msk
              |GPIO_MODER_MODER7_Msk;
    mVALUE =   (2<<GPIO_MODER_MODER0_Pos)
              |(2<<GPIO_MODER_MODER1_Pos)
              |(2<<GPIO_MODER_MODER2_Pos)
              |(2<<GPIO_MODER_MODER4_Pos)
              |(2<<GPIO_MODER_MODER5_Pos)
              |(2<<GPIO_MODER_MODER6_Pos)
              |(2<<GPIO_MODER_MODER7_Pos);
    GPIOI->MODER  = (GPIOI->MODER&~mFIELD)|mVALUE;

                // Configuring Alternate Function (Pins 7-0)
    mFIELD =   GPIO_AFRL_AFRL0_Msk
              |GPIO_AFRL_AFRL1_Msk
              |GPIO_AFRL_AFRL2_Msk
              |GPIO_AFRL_AFRL4_Msk
              |GPIO_AFRL_AFRL5_Msk
              |GPIO_AFRL_AFRL6_Msk
              |GPIO_AFRL_AFRL7_Msk;
    mVALUE =   (14<<GPIO_AFRL_AFRL0_Pos)
              |(14<<GPIO_AFRL_AFRL1_Pos)
              |(14<<GPIO_AFRL_AFRL2_Pos)
              |(14<<GPIO_AFRL_AFRL4_Pos)
              |(14<<GPIO_AFRL_AFRL5_Pos)
              |(14<<GPIO_AFRL_AFRL6_Pos)
              |(14<<GPIO_AFRL_AFRL7_Pos);
    GPIOK->AFR[0]  = (GPIOK->AFR[0]&~mFIELD)|mVALUE;

    /*
     * I2C signals
     */
                // Configuring Mode
    mFIELD =   GPIO_MODER_MODER7_Msk
              |GPIO_MODER_MODER8_Msk;
    mVALUE =   (2<<GPIO_MODER_MODER7_Pos)
              |(2<<GPIO_MODER_MODER8_Pos);
    GPIOH->MODER  = (GPIOH->MODER&~mFIELD)|mVALUE;

                // Configuring Alternate Function (Pins 7-0)
    mFIELD =   GPIO_AFRL_AFRL7_Msk;
    mVALUE =   (4<<GPIO_AFRL_AFRL7_Pos);
    GPIOH->AFR[0]  = (GPIOH->AFR[0]&~mFIELD)|mVALUE;
                // Configuring Alternate Function (Pins 15-8)
    mFIELD =   GPIO_AFRL_AFRL0_Msk;
    mVALUE =   (4<<GPIO_AFRL_AFRL0_Pos);
    GPIOH->AFR[1]  = (GPIOH->AFR[1]&~mFIELD)|mVALUE;
    /*
     * LCD Control signals
     *
     *     LCD_INT      PI13    input   LCD interrupt signal
     *     LCD_DISP     PI12    output
     *     LCD_BL_CTRL  PK3     output
     */
    mFIELD =   GPIO_MODER_MODER12_Msk
              |GPIO_MODER_MODER13_Msk;
    mVALUE =   (1<<GPIO_MODER_MODER12_Pos)
              |(0<<GPIO_MODER_MODER13_Pos);
    GPIOI->MODER  = (GPIOI->MODER&~mFIELD)|mVALUE;

    mFIELD =   GPIO_MODER_MODER3_Msk;
    mVALUE =   (1<<GPIO_MODER_MODER3_Pos);
    GPIOK->MODER  = (GPIOK->MODER&~mFIELD)|mVALUE;

}
#else

static const GPIO_PinConfiguration configtable[] = {
    { GPIOI,    14,    14 },       // LCD_CLK
    { GPIOI,     9,    14 },       // LCD_VSYNC
    { GPIOI,    10,    14 },       // LCD_HSYNC
    { GPIOK,     7,    14 },       // LCD_DE
// Red
    { GPIOI,    15,    14 },       // LCD_R0
    { GPIOJ,     0,    14 },       // LCD_R1
    { GPIOJ,     1,    14 },       // LCD_R2
    { GPIOJ,     2,    14 },       // LCD_R3
    { GPIOJ,     3,    14 },       // LCD_R4
    { GPIOJ,     4,    14 },       // LCD_R5
    { GPIOJ,     5,    14 },       // LCD_R6
    { GPIOJ,     6,    14 },       // LCD_R7
// Green
    { GPIOJ,     7,    14 },       // LCD_G0
    { GPIOJ,     8,    14 },       // LCD_G1
    { GPIOJ,     9,    14 },       // LCD_G2
    { GPIOJ,    10,    14 },       // LCD_G3
    { GPIOJ,    11,    14 },       // LCD_G4
    { GPIOK,     0,    14 },       // LCD_G5
    { GPIOK,     1,    14 },       // LCD_G6
    { GPIOK,     2,    14 },       // LCD_G7
// Blue
    { GPIOE,     4,    14 },       // LCD_B0
    { GPIOJ,    13,    14 },       // LCD_B1
    { GPIOJ,    14,    14 },       // LCD_B2
    { GPIOJ,    15,    14 },       // LCD_B3
    { GPIOG,    12,    14 },       // LCD_B4
    { GPIOK,     4,    14 },       // LCD_B5
    { GPIOK,     5,    14 },       // LCD_B6
    { GPIOK,     6,    14 },       // LCD_B7

// I2C
    { GPIOH,     7,     4 },       // LCD_SCL / AUDIO SCL (I2C3_SDA)
    { GPIOH,     8,     4 },       // LCD_SDA / AUDIO SDA (I2C3_SCL)
// Others
    { GPIOI,    13,     0, 0, 1, 0, 0 },// LCD INT = input
    { GPIOI,    12,     0, 1, 0, 0, 0 },// LCD DISP = output
    { GPIOK,     3,     0, 1, 0, 0, 0 },// LCD Backlight Control = output
    {     0,     0,     0, 0, 0, 0, 0 },// End of table indicator
};

#endif
/**
 *
 */
static int  LCDCLOCK_Initialized = 0;

/**
 * @brief   Set Clock for LCD
 *
 * @param   div must be 2, 4, 8 or 16
 *
 * @note    This should be set only when PLLSAI is disabled
 */
void LCD_SetClock(uint32_t div) {
uint32_t pllsaidivr;

    if( RCC->CR&RCC_CR_PLLSAION )   // Do nothing when PLLSAI is enabled
        return;

    pllsaidivr = div;
    switch(div) {
    case 2:  pllsaidivr = 0; break;
    case 4:  pllsaidivr = 1; break;
    case 8:  pllsaidivr = 2; break;
    case 16: pllsaidivr = 3; break;
    default:
        return; // ignore wrong divisor
    }

    RCC->DCKCFGR1 = (RCC->DCKCFGR1&~RCC_DCKCFGR1_PLLSAIDIVR)
                    |(pllsaidivr<<RCC_DCKCFGR1_PLLSAIDIVR_Pos);

    LCDCLOCK_Initialized = 1;
}


/**
 * @brief   LCD Low Level Initializationb
 *
 * @note    Configure LCD unit
 *
 * @note    *PLLSAI must be configured and ON before calling this routine*
 *
 * @note    The R output of PLLSAI generates pixel clock (LCD_CLK)
 */

void LCD_Init(void) {

    /* If LCD Clock not initialized */
    if( !LCDCLOCK_Initialized )
        return;

    /* If PLLSAI not on, return */
    if( (RCC->CR & RCC_CR_PLLSAION) == 0 )
        return;

    /* If PLLSAI not ready, return */
    if( (RCC->CR & RCC_CR_PLLSAIRDY) == 0 )
        return;

#ifdef FAST_INITIALIZATION
    ConfigurePins();
#else
    /* Configure pins from table*/
    GPIO_ConfigureMultiplePins(configtable);
#endif

    /* Configure additional pins */
    GPIO_Init(GPIOI,BIT(12),BIT(12));
    GPIO_Init(GPIOK,0,BIT(3));

    // Enable clock for LCD
    RCC->APB2ENR |= RCC_APB2ENR_LTDCEN;

    // Reset LCD


    // Configure LCD geometry
    LTDC->SSCR =  (HSW-1)<<LTDC_SSCR_HSW_Pos
                 |(VSW-1)<<LTDC_SSCR_VSH_Pos;
    LTDC->BPCR =  (HSW+HBP-1)<<LTDC_BPCR_AHBP_Pos
                 |(VSW+VBP-1)<<LTDC_BPCR_AVBP_Pos;
    LTDC->AWCR =  (HSW+HBP+HAW-1)<<LTDC_AWCR_AAW_Pos
                 |(VSW+VBP+VAH-1)<<LTDC_AWCR_AAW_Pos;
    LTDC->TWCR  = 0;
    LTDC->TWCR |= (HSW+HBP+HAW+HFP-1)<<LTDC_TWCR_TOTALW_Pos
                 |(VSW+VBP+VAH+VFP-1)<<LTDC_TWCR_TOTALW_Pos;
    LTDC->TWCR |= (HSW+HBP+HAW+HFP-1)<<LTDC_TWCR_TOTALH_Pos
                 |(VSW+VBP+VAH+VFP-1)<<LTDC_TWCR_TOTALH_Pos;


    LTDC->IER  |= (LTDC_IER_RRIE|LTDC_IER_TERRIE|LTDC_IER_FUIE|LTDC_IER_LIE);
}


/*
 * @brief   LCD_FillFrameBuffer
 *
 * @note    Fill the frame buffer using only word access
 *
 * @note    Memory organization
 *            word  | Pixel
 *         ---------|-----------------
 *            +0    | B1 R0 G0 B0
 *            +1    | G2 B2 R1 G1
 *            +2    | R3 G3 B3 R2
 */
void
LCD_FillFrameBuffer( RGB_t *frame, RGB_t v ) {
uint32_t w1,w2,w3;
uint32_t *p = (uint32_t *) frame;

    w1 = (v.B<<24)|(v.R<<16)|(v.G<<8)|v.B;
    w2 = (v.G<<24)|(v.B<<16)|(v.R<<8)|v.G;
    w3 = (v.R<<24)|(v.G<<16)|(v.B<<8)|v.R;

    for(int i=0;i<(LCD_DW*LCD_DH+3)/4;i+=3) {
        p[0] = w1;
        p[1] = w2;
        p[2] = w3;
        p+=3;
    }
}

