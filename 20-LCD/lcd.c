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


#define LCDBIT(N) (1U<<(N))


/**
 * LCD Connection
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
 *
 * LCD_B0       |  AF14   | *PE4* PG14 PJ12
 * LCD_B1       |  AF14   | PG12 *PJ13*
 * LCD_B2       |  AF14   | PD6 PG10 *PJ14*
 * LCD_B3       |  AF14   | PD10 PG11 *PJ15*
 * LCD_B4       |  AF14   | PE12 PG12 *PG12(AF9)* PI4 PK3
 * LCD_B5       |  AF14   | PA3 PI5 *PK4*
 * LCD_B6       |  AF14   | PB8 PI6 *PK5*
 * LCD_B7       |  AF14   | PB9 PI7 *PK6*
 *
 * LCD_VSYNC    |  AF14   | PA4 *PI9* PI13
 * LCD_HSYNC    |  AF14   | PC6 *PI10* PI12
 * LCD_DE       |  AF14   | PE13 PF10 *PK7*
 *
 * The following uses GPIO I/O or other modes
 *
 *  LCD_DISP     | GPIO   | I12              ENABLE LCD
 *  LCD_INT      | ?      | I13              INTERRUPT
 *  LCD_BL_CTRL  | ?      | K3                PWM
 */

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
    { GPIOJ,     6,    14 },       // LCD_R6
// Green
    { GPIOJ,     7,    14 },       // LCD_G0
    { GPIOJ,     8,    14 },       // LCD_G1
    { GPIOJ,     9,    14 },       // LCD_G2
    { GPIOJ,    10,    14 },       // LCD_G3
    { GPIOK,     0,    14 },       // LCD_G4
    { GPIOK,     1,    14 },       // LCD_G5
    { GPIOK,     2,    14 },       // LCD_G6
    { GPIOJ,     7,    14 },       // LCD_G7
// Blue
    { GPIOE,     4,    14 },       // LCD_B7
    { GPIOJ,    13,    14 },       // LCD_B6
    { GPIOJ,    14,    14 },       // LCD_B5
    { GPIOJ,    15,    14 },       // LCD_B4
    { GPIOG,    12,     9 },       // LCD_B3
    { GPIOK,     4,    14 },       // LCD_B3
    { GPIOK,     5,    14 },       // LCD_B2
    { GPIOK,     6,    14 },       // LCD_B0
// I2C
    { GPIOH,     7,     4 },       // LCD_SCL / AUDIO SCL (I2C3_SDA)
    { GPIOH,     8,     4 },       // LCD_SDA / AUDIO SDA (I2C3_SCL)
//
    {     0,     0,     0 }         // End of table indicator
};

/**
 * @brief   LCD_Init
 *
 * @note    Configure LCD unit
 *
 * @note    *PLLSAI must be configured and ON before calling this routine*
 *
 * @note    The R output of PLLSAI generates pixel clock (LCD_CLK)
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

void LCD_Init(void) {

    /* If PLLSAI not on, return */
    if( RCC->CR & RCC_CR_PLLSAION != 0 )
        return;

    /* Configure pins from table*/
    GPIO_ConfigureAlternateFunctionMultiple(configtable);

    /* Configure additional pins */
    GPIO_Init(GPIOI,LCDBIT(12),LCDBIT(12));
    GPIO_Init(GPIOK,0,LCDBIT(3));

    // Enable clock for LCD
    RCC->APB2ENR |= RCC_APB2ENR_LTDCEN;

    // Reset LCD


    // Configure geometry
    LTDC->SSCR =  (HSW-1)<<LTDC_SSCR_HSW_Pos | (VSW-1)<<LTDC_SSCR_VSH_Pos;
    LTDC->BPCR =  (HSW+HBP-1)<<LTDC_BPCR_AHBP_Pos | (VSW+VBP-1)<<LTDC_BPCR_AVBP_Pos;
    LTDC->AWCR =  (HSW+HBP+HAW-1)<<LTDC_AWCR_AAW_Pos | (VSW+VBP+VAH-1)<<LTDC_AWCR_AAW_Pos;
    LTDC->TWCR = 0;
    LTDC->TWCR |=  (HSW+HBP+HAW+HFP-1)<<LTDC_TWCR_TOTALW_Pos | (VSW+VBP+VAH+VFP-1)<<LTDC_TWCR_TOTALW_Pos;
    LTDC->TWCR |=  (HSW+HBP+HAW+HFP-1)<<LTDC_TWCR_TOTALH_Pos | (VSW+VBP+VAH+VFP-1)<<LTDC_TWCR_TOTALH_Pos;


    LTDC->IER |= (LTDC_IER_RRIE|LTDC_IER_TERRIE|LTDC_IER_FUIE|LTDC_IER_LIE);
}


/*
 * @brief   LCD_GetFrameBuffer
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
LCD_FillFrameBuffer( RGB_t (*frame)[LCD_DH], RGB_t v ) {
uint32_t w1,w2,w3;
uint32_t *p = (uint32_t *) frame;

    w1 = (v.B<<24)|(v.R<<16)|(v.G<<8)|v.B;
    w2 = (v.G<<24)|(v.B<<16)|(v.R<<8)|v.G;
    w3 = (v.R<<24)|(v.G<<16)|(v.B<<8)|v.R;

    for(int i=0;i<LCD_DW;i+=3) {
        p[0] = w1;
        p[1] = w2;
        p[2] = w3;
        p+=3;
    }
}

