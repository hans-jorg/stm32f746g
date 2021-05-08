/**
 * @file    lcd.c
 *
 * @date    07/10/2020
 * @author  Hans
 */

#include "stm32f746xx.h"
#include "system_stm32f746.h"
#include "lcd.h"


#define BIT(N)          (1U<<(N))

/**
 * @brief Use GPIO library to initialize and control pins
 *
 * @note  Comment it to use a faster but larger implementation
 */
#define LCD_USEGPIO 1

/**
 *  @brief  Default values
 */
///@{
#define BACKGROUND_COLOR        RGB(0,0,255)


///@}

/*
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
 *  SCL         | LCD_SCL           | PH7                    |
 *  SDA         | LCD_SDA           | PH8                    |
 *  RST         | LCD_RST/NRST      | NRST                   |
 *  INT         | LCD_INT           | PI13                   |
 *  DISP        | LCD_DISP          | PI12                   |
 *  Backlight   | LCD_BL_CTRL       | PK3                    |
 *
 * @note I2CX signals (LCD_SCL and LCD_SDA) are shared with an audio device
 *
 * @note LCD_BL_CTRL, LCD_DISP, LCD_INT, LCD_RST are not controlled by the LCD
 *       interface
 */

/*
 * @brief   Mask for LCD signals
 *
 * @note    LCD_BACKLIGHTCTRL  controls the enable pin of the STLD40DPUR (hite LED power
 *          supply for large display backlight)
 *
 * @note    LCD_DISP controls normal operation (1) or stand-by (0)
 *
 * @note    LCD_INT interrupts the MCU
 *
 */
/** On port K */
///@{
#define LCD_BACKLIGHTCTRL_MASK  BIT(3)
///@}
/** On port I */
///@{
#define LCD_INTERRUPT_MASK      BIT(13)
#define LCD_NORMALSTANDBY_MASK  BIT(12)
///@}

/**
 *  @brief  Characteristics of the LCD interface
 *
 * | Signal  | Description                 |  Min     |  Max     |
 * |---------|-----------------------------|----------|----------|
 * | LCD_CLK | LTDC clock output frequency |  -       |  45 MHz  |
 *
 *  @note LCD_CLK is generated from the R output of the PLLSAI
 *
 *  @note There is a divisor (LLSAIDIVR) of the R output signal that can assume the
 *        values 2, 4, 8 and 16. Is is set in the field PLLSAIDIVR of the RCC_DCKFGR1 register.
 *  @note The input of the PLLSAI is the same of Main PLL. In this project
 *        it is 1 MHz, obtained by dividing the 25 MHz clock input by M=25, set in the
 *        field PLLM of the RCC_PLLCFGR register.
 */

/**
 * @brief   Point to base address of LTDC Layer registers
 *
 * @note    Extra Layer2 at 0 to make possible to use integer 1 and 2 to represent
 *          the layer. An extra benefit is that 0 can be used as an index too.
 */
static LTDC_Layer_TypeDef * const LTDC_Layer[3] = {LTDC_Layer2, LTDC_Layer1, LTDC_Layer2};
/**
 * @brief   Characteristics of the LCD display
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

/**
 * @brief   Struct to store information about display
 */
typedef struct {
    uint32_t    frequency;          /// LCD frequency
    uint32_t    polarity;           /// polarity of control signals (Use LTDC_CGR_xxPOL symbols)
    uint16_t    divider;            /// divider to be used ??
    uint16_t    width;              /// Visible width
    uint16_t    height;             /// Visible height
    uint16_t    hsync;              /// Horizontal synchronization
    uint16_t    vsync;              /// Vertical synchronization (unit is HSYNC Period)
    uint16_t    hfp;                /// Horizontal front porch (unit is LCD_CLK Period)
    uint16_t    hbp;                /// Horizontal back porch  (unit is LCD_CLK Period)
    uint16_t    vfp;                /// Vertical front porch (unit is HSYNC Period)
    uint16_t    vbp;                /// Vertical back porch (unit is HSYNC Period)
    uint16_t    pitch[5];           /// Pitch used to store lines in memory (in bytes)
                                    /// For 1,2,3,4 bytes (Position 0 not used)

} DisplayProperties_t;

/**
  * @brief  RK043FN48H properties
  */

const DisplayProperties_t dispRK043 = {
    .frequency  =   9000000,        // range [5..12] MHz
    .polarity   =   0,              // Use LTDC_CGR_xxPOL symbols)
    .divider    =   5,              //
    .width      =   480,            //
    .height     =   272,            //
    .hsync      =   41,             //
    .vsync      =   10,             //
    .hfp        =   32,             // or 8
    .hbp        =   13,             // or 40
    .vfp        =   2,              // or 4
    .vbp        =   2,              // or 12
    .pitch      = { 0,              // Position not used!!!!
                    512,            // L8 and other 1 byte representation
                    1024,           // RGB565 and other 2 bytes representation
                    1536,           // RGB888 (3 bytes)
                    2048 },         // for ARGB8888 (4 bytes)
};

/**
 * @brief Pointer to current display
 *
 * @note  For now, only one
 */
const DisplayProperties_t *display = &dispRK043;

/**
 * @brief Symbols to make it easier to write complex expressions
 */
///@{
#define LCD_FREQ    (display->frequency)
#define HSW         (display->hsync)
#define HAW         (display->width)
#define HFP         (display->hfp)
#define HBP         (display->hbp)
#define VSH         (display->vsync)
#define VAH         (display->height)
#define VBP         (display->vbp)
#define VFP         (display->vfp)
#define POL         (display->polarity)
///@}

static const int pixelsize[] = {
        4,  // 000: ARGB8888
        3,  // 001: RGB888
        2,  // 010: RGB565
        2,  // 011: ARGB1555
        2,  // 100: ARGB4444
        1,  // 101: L8 (8-bit luminance)
        1,  // 110: AL44 (4-bit alpha, 4-bit luminance)
        1   // 111: AL88 (8-bit alpha, 8-bit luminance)
};

/**
 *  @brief Pin Configuration for LCD
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

#ifdef LCD_USEGPIO

#include "gpio.h"
/* Pin Configuration Fields                                                    */
/* GPIOx: GPIO interface        [GPIOA...GPIOK]                                */
/*     AF: Alternate function   [0:GPIO,2:I2C, ... 14:LCD]                     */
/*     M:  Mode                 [0:Input, 1=Output,2=AF,3=Analog]              */
/*     OT: Output Type          [0:Push-pull, 1=Open-drain ]                   */
/*     S:  Speed                [0:Low, 1:Medium, 2:High, 3:Very high]         */
/*     P:  Pull-down/Pull-up    [0:None, 1:Pull-up, 2:Pull-down]               */
/*     I:  Initial              [0: Low, 1:High]                               */
static const GPIO_PinConfiguration configtable[] = {

/*    GPIOx    Pin     AF  M  O  S  P  I  */
// Control
    { GPIOI,    14,    14, 2, 0, 3, 0, 0 },       // LCD_CLK
    { GPIOI,     9,    14, 2, 0, 3, 0, 0 },       // LCD_VSYNC
    { GPIOI,    10,    14, 2, 0, 3, 0, 0 },       // LCD_HSYNC
    { GPIOK,     7,    14, 2, 0, 3, 0, 0 },       // LCD_DE
// Red
    { GPIOI,    15,    14, 2, 0, 3, 0, 0 },       // LCD_R0
    { GPIOJ,     0,    14, 2, 0, 3, 0, 0 },       // LCD_R1
    { GPIOJ,     1,    14, 2, 0, 3, 0, 0 },       // LCD_R2
    { GPIOJ,     2,    14, 2, 0, 3, 0, 0 },       // LCD_R3
    { GPIOJ,     3,    14, 2, 0, 3, 0, 0 },       // LCD_R4
    { GPIOJ,     4,    14, 2, 0, 3, 0, 0 },       // LCD_R5
    { GPIOJ,     5,    14, 2, 0, 3, 0, 0 },       // LCD_R6
    { GPIOJ,     6,    14, 2, 0, 3, 0, 0 },       // LCD_R7
// Green
    { GPIOJ,     7,    14, 2, 0, 3, 0, 0 },       // LCD_G0
    { GPIOJ,     8,    14, 2, 0, 3, 0, 0 },       // LCD_G1
    { GPIOJ,     9,    14, 2, 0, 3, 0, 0 },       // LCD_G2
    { GPIOJ,    10,    14, 2, 0, 3, 0, 0 },       // LCD_G3
    { GPIOJ,    11,    14, 2, 0, 3, 0, 0 },       // LCD_G4
    { GPIOK,     0,    14, 2, 0, 3, 0, 0 },       // LCD_G5
    { GPIOK,     1,    14, 2, 0, 3, 0, 0 },       // LCD_G6
    { GPIOK,     2,    14, 2, 0, 3, 0, 0 },       // LCD_G7
// Blue
    { GPIOE,     4,    14, 2, 0, 3, 0, 0 },       // LCD_B0
    { GPIOJ,    13,    14, 2, 0, 3, 0, 0 },       // LCD_B1
    { GPIOJ,    14,    14, 2, 0, 3, 0, 0 },       // LCD_B2
    { GPIOJ,    15,    14, 2, 0, 3, 0, 0 },       // LCD_B3
    { GPIOG,    12,    14, 2, 0, 3, 0, 0 },       // LCD_B4
    { GPIOK,     4,    14, 2, 0, 3, 0, 0 },       // LCD_B5
    { GPIOK,     5,    14, 2, 0, 3, 0, 0 },       // LCD_B6
    { GPIOK,     6,    14, 2, 0, 3, 0, 0 },       // LCD_B7

// I2C
    { GPIOH,     7,     4, 0, 2, 3, 0, 0 },       // LCD_SCL / AUDIO SCL (I2C3_SDA)
    { GPIOH,     8,     4, 0, 2, 3, 0, 0 },       // LCD_SDA / AUDIO SDA (I2C3_SCL)
// Others
    { GPIOI,    13,     0, 0, 1, 0, 0, 0 },       // LCD INT = input
    { GPIOI,    12,     0, 1, 0, 3, 0, 0 },       // LCD DISP = output
    { GPIOK,     3,     0, 1, 0, 2, 0 ,0 },       // LCD Backlight Control = output
    {     0,     0,     0, 0, 0, 0, 0 ,0 },       // End of table indicator
};


static void ConfigureLCDPins(void) {

    /* Configure pins from table*/
    GPIO_ConfigureMultiplePins(configtable);

}
#else

static void ConfigureLCDPins(void) {
uint32_t mFIELD, mVALUE;

    /********* Configuring LCD signals *******************/

    /*
     * Port   GPIOE
     */
                // Enabling clock
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOEEN;
    __NOP(); __DSB();
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
                // Enabling clock
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOGEN;
    __NOP(); __DSB();
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
                // Enabling clock
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOIEN;
    __NOP(); __DSB();
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
                // Enabling clock
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOJEN;
    __NOP(); __DSB();
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
    GPIOJ->MODER  = (GPIOJ->MODER&~mFIELD)|mVALUE;

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
                // Enabling clock
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOIEN;
    __NOP(); __DSB();
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
    GPIOK->MODER  = (GPIOK->MODER&~mFIELD)|mVALUE;

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
                // Enabling clock
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOHEN;
    __NOP(); __DSB();
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
     * LCD Control signals using GPIO
     *
     *     LCD_INT      PI13    input   LCD interrupt signal
     *     LCD_DISP     PI12    output
     *     LCD_BL_CTRL  PK3     output
     */
                // Enabling clock
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOIEN;
    __NOP(); __DSB();
                // Configuring mode
    mFIELD =   GPIO_MODER_MODER12_Msk
              |GPIO_MODER_MODER13_Msk;
    mVALUE =   (1<<GPIO_MODER_MODER12_Pos)
              |(0<<GPIO_MODER_MODER13_Pos);
    GPIOI->MODER  = (GPIOI->MODER&~mFIELD)|mVALUE;

                // Enabling clock
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOKEN;
    __NOP(); __DSB();
                // Configuring mode
    mFIELD =   GPIO_MODER_MODER3_Msk;
    mVALUE =   (1<<GPIO_MODER_MODER3_Pos);
    GPIOK->MODER  = (GPIOK->MODER&~mFIELD)|mVALUE;

}
#endif

/**
 * @brief   Turn LCD Backlight On/Off
 *
 * @note    GPIOK Pin 3 controls the backlight
 */

///@{
void  LCD_TurnBacklightOn(void) {

#ifdef LCD_USEGPIO
    GPIO_Init(GPIOK,0,LCD_BACKLIGHTCTRL_MASK);
    GPIO_Set(GPIOK,LCD_BACKLIGHTCTRL_MASK);
#else
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOKEN;
    __NOP(); __DSB();
    GPIOK->MODER  = (GPIOK->MODER&~GPIO_MODER_MODER3_Msk)
                    |(1<<GPIO_MODER_MODER3_Pos);
    GPIOK->BSRR |= LCD_BACKLIGHTCTRL_MASK;
#endif

}

void  LCD_TurnBacklightOff(void) {

#ifdef LCD_USEGPIO
    GPIO_Init(GPIOK,0,LCD_BACKLIGHTCTRL_MASK);
    GPIO_Clear(GPIOK,LCD_BACKLIGHTCTRL_MASK);
#else
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOKEN;
    __NOP(); __DSB();
    GPIOK->MODER  = (GPIOK->MODER&~GPIO_MODER_MODER3_Msk)
                    |(1<<GPIO_MODER_MODER3_Pos);
    GPIOK->BSRR |= (LCD_BACKLIGHTCTRL_MASK<<16);
#endif

}
///@}

/**
 * @brief   Put LCD Operation/Standby
 *
 * @note    GPIOI Pin 12 controls normal operation/standby
 */

///@{
void  LCD_PutDisplayOperation(void) {

#ifdef LCD_USEGPIO
    GPIO_Set(GPIOI,LCD_NORMALSTANDBY_MASK);
#else
    GPIOI->BSRR |= LCD_NORMALSTANDBY_MASK;
#endif

}

void  LCD_PutDisplayStandBy(void) {

#ifdef LCD_USEGPIO
    GPIO_Clear(GPIOI,LCD_NORMALSTANDBY_MASK);
#else
    GPIOI->BSRR |= (LCD_NORMALSTANDBY_MASK<<16);
#endif

}
///@}



/**
 * @brief   Enable/Disable LCD Controler
 *
 * @note    Disable LTDC controller
 */

///@{
void  LCD_EnableController(void) {

    LTDC->GCR |= LTDC_GCR_LTDCEN;

}

void  LCD_DisableController(void) {

    LTDC->GCR &= ~LTDC_GCR_LTDCEN;

}
///@}


/**
 * @brief   Turn LCD Backlight On/Off
 *
 * @note    GPIOK Pin 3 controls the backlight
 */

///@{
void  LCD_On(void) {

    LCD_EnableController();
    LCD_PutDisplayOperation();
    LCD_TurnBacklightOn();

}

void  LCD_Off(void) {

    LCD_DisableController();
    LCD_PutDisplayStandBy();
    LCD_TurnBacklightOff();
}
///@}


/*
 * @brief   Configuration for PLLSAI
 *
 * @note    Assumes PLL Main will use HSE (crystal) and have a 1 MHz input for PLL
 *
 * @note    $$ f_{VCOOUT} = f_{INPUT} * N / M $$
 *          $$ f_{OUTP} = {VCOOUT} / P $$
 *          $$ f_{OUTQ} = {VCOOUT} / Q $$
 *          $$ f_{OUTR} = {VCOOUT} / R $$
 *
 *          $$ f_{INPUT} = "16 MHz (HSI)" or "25 MHz (HSE)" $$
 *          $$ M = {2..63} identical to the one used in Main PLL
 *          $$ N = {50..432 } $$
 *          $$ P = {2, 4, 6, 8 } $$
 *          $$ Q = {2..15 } $$
 *          $$ R = {2..7} $$


 * @note    LCD_CLK should be in range 5-12, with typical value 9 MHz.
 *
 * @note    There is an extra divisor in PLLSAIDIVR[1:0] of RCC_DCKCFGR, that can
 *          have value 2, 4, 8 or 16.
 *
 * @note    So the R output must be 18, 36, 72 or 144 MHz.
 *          But USB, RNG and SDMMC needs 48 MHz. The LCM of 48 and 9 is 144.
 *          But there are a minimal value for the divisor equal to 2.
 *          $$ f_{OUTR} = 72 $$
 *
 * @note    For f_LCDCLK  = 9 MHz the extra divisor PLLSAIRDIV must be 8
 *
 */

/**
 * @brief   Flag to indicate clock for the LCD is set
 */
static int  LCDCLOCK_Initialized = 0;

/**
 * @brief   Set Clock for LCD
 *
 * @note    LCD_CLOCK for the display used must be in the range [5..12]
 */
int LCD_SetClock(void) {
uint32_t pllsaidivr;
int div;
PLLOutputFrequencies_t pllfreq;


    if( ! (RCC->CR&RCC_CR_PLLSAION) ) {
        SystemConfigPLLSAI(&PLLSAIConfiguration_48MHz);
        SystemEnablePLLSAI();
    }

    SystemGetPLLFrequencies(PLL_SAI, &pllfreq);
    div = pllfreq.routfreq/LCD_FREQ;


    switch(div) {
    case 2:  pllsaidivr = 0; break;
    case 4:  pllsaidivr = 1; break;
    case 8:  pllsaidivr = 2; break;
    case 16: pllsaidivr = 3; break;
    default:
        return -2; // ignore wrong divisor
    }

    // Configure divisor for LCD controller
    RCC->DCKCFGR1 = (RCC->DCKCFGR1&~RCC_DCKCFGR1_PLLSAIDIVR)
                    |(pllsaidivr<<RCC_DCKCFGR1_PLLSAIDIVR_Pos);

    // Enable clock for LCD controller
    RCC->APB2ENR |= RCC_APB2ENR_LTDCEN;

    LCDCLOCK_Initialized = 1;
    return 0;
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
        LCD_SetClock();

    /* If PLLSAI not on, return */
    if( (RCC->CR & RCC_CR_PLLSAION) == 0 )
        return;

    /* If PLLSAI not ready, return */
    if( (RCC->CR & RCC_CR_PLLSAIRDY) == 0 )
        return;

    /* Configure pins for LCD usage */
    ConfigureLCDPins();

    // Enable clock for LCD
    RCC->APB2ENR |= RCC_APB2ENR_LTDCEN;

    // Reset LCD

    // configure polarity of control signals
    LTDC->GCR =   (LTDC->GCR&~(LTDC_GCR_DEPOL|LTDC_GCR_HSPOL|LTDC_GCR_VSPOL|LTDC_GCR_PCPOL))
                 |(POL);
    // Configure LCD geometry
    LTDC->SSCR =  ((HSW-1)<<LTDC_SSCR_HSW_Pos)
                 |((VSH-1)<<LTDC_SSCR_VSH_Pos);
    LTDC->BPCR =  ((HSW+HBP-1)<<LTDC_BPCR_AHBP_Pos)
                 |((VSH+VBP-1)<<LTDC_BPCR_AVBP_Pos);
    LTDC->AWCR =  ((HSW+HBP+HAW-1)<<LTDC_AWCR_AAW_Pos)
                 |((VSH+VBP+VAH-1)<<LTDC_AWCR_AAH_Pos);
    LTDC->TWCR  = ((HSW+HBP+HAW+HFP-1)<<LTDC_TWCR_TOTALW_Pos)
                 |((VSH+VBP+VAH+VFP-1)<<LTDC_TWCR_TOTALH_Pos);

    // Set background color
    LTDC->BCCR = BACKGROUND_COLOR;

    /* Enable interrupts */
    //LTDC->IER  |= (LTDC_IER_RRIE|LTDC_IER_TERRIE|LTDC_IER_FUIE|LTDC_IER_LIE);


    LCD_PutDisplayOperation();
    LCD_TurnBacklightOn();

}


/*
 * @brief   LCD Set Background Color
 */

void
LCD_SetBackgroundColor( uint32_t bgcolor ) {

    LTDC->BCCR = bgcolor;
}


//////////////////////////// LAYER routines ////////////////////////////////////////////////////////



/**
 * @brief   Enable/Disable Layer
 *
 * @note
 */
///@{
void  LCD_EnableLayer(int layer) {

    LTDC_Layer[layer]->CR |= LTDC_LxCR_LEN;
    // Reload parameters
    LTDC->SRCR |= LTDC_SRCR_IMR;
}


void  LCD_DisableLayer(int layer) {

    LTDC_Layer[layer]->CR &= ~LTDC_LxCR_LEN;
    // Reload parameters
    LTDC->SRCR |= LTDC_SRCR_IMR;
}

void  LCD_SwapLayers(void) {

    LTDC_Layer[0]->CR ^= LTDC_LxCR_LEN;
    LTDC_Layer[1]->CR ^= LTDC_LxCR_LEN;
    // Reload parameters
    LTDC->SRCR |= LTDC_SRCR_IMR;
}


///@}

/**
 * @brief   Enable/Disable Layer
 *
 * @note
 */
void  LCD_ReloadLayerImmediately(int layer) {

    LTDC->SRCR |= LTDC_SRCR_IMR;
}

/**
 * @brief   Enable/Disable Layer
 *
 * @note
 */
void  LCD_ReloadLayerByVerticalBlanking(int layer) {

    LTDC->SRCR |= LTDC_SRCR_VBR;
}


/*
 * @brief   LCD Set Default Color for layer
 */

void
LCD_SetDefaultColor(int layer,  uint32_t color ) {

    LTDC_Layer[layer]->DCCR = color;
}

/*
 * @brief   LCD Set format for layer
 */
void  LCD_SetFormat(int layer, int format) {

    LTDC_Layer[layer]->PFCR = format;
}


/*
 * @brief   LCD Set Color Key for layer
 */

void
LCD_SetColorKey(int layer,  uint32_t c ) {

    LTDC_Layer[layer]->CKCR = c;
}


/*
 * @brief   LCD Get Frame Buffer Address of a specified layer
 */
void *
LCD_GetFrameBufferAddress(int layer) {

   return (void *) LTDC_Layer[layer]->CFBAR;
}


/*
 * @brief   LCD Get Format used in layer
 */
int   LCD_GetFormat(int layer) {

    return LTDC_Layer[layer]->PFCR;
}

/*
 * @brief   LCD Get Pixel Size in bytes of the layer specified
 */
int   LCD_GetPixelSize(int layer) {

    return pixelsize[LCD_GetFormat(layer)];
}

/*
 * @brief   LCD Get Pixel Size in bytes of the layer specified
 *
 * @note    Uses the pitch information to calculate the demanded
 *          area size
 */
int   LCD_GetMinimalFullFrameBufferSize(int format) {
int ps = pixelsize[format];

    return display->pitch[ps]*display->height;

}

/*
 * @brief   LCD Set Frame Buffer for layer
 */

void
LCD_SetFullSizeFrameBuffer(int layer, void *area, int format) {
LTDC_Layer_TypeDef *const p = LTDC_Layer[layer];
uint32_t ps,w,h,ws,hs,pitch,dw,dh;

    ps        = pixelsize[format];
    h         = display->height;
    w         = display->width;
    p->PFCR   = format;
    pitch     = display->pitch[ps];
    p->CFBAR  = (uint32_t) area;
    p->CFBLR  = (pitch<<LTDC_LxCFBLR_CFBP_Pos) | ((w*ps+3)<<LTDC_LxCFBLR_CFBLL_Pos);
    p->CFBLNR = (h<<LTDC_LxCFBLNR_CFBLNBR_Pos);

    dw        = (LTDC->BPCR&LTDC_BPCR_AHBP_Msk)>>LTDC_BPCR_AHBP_Pos;
    ws        = w+dw;
    dh        = (LTDC->BPCR&LTDC_BPCR_AVBP_Msk)>>LTDC_BPCR_AVBP_Pos;
    hs        = h+dh;
    p->WHPCR  = ((ws)<<LTDC_LxWHPCR_WHSPPOS_Pos)
               |((dw+1)<<LTDC_LxWHPCR_WHSTPOS_Pos);
    p->WVPCR  = ((hs)<<LTDC_LxWVPCR_WVSPPOS_Pos)
               |((dh+1)<<LTDC_LxWVPCR_WVSTPOS_Pos);

    // Enable layer, effective immediately
    LCD_EnableLayer(layer);
    LTDC->SRCR |= LTDC_SRCR_IMR;
}

/*
 * @brief   LCD Set Frame Buffer for layer
 */

void
LCD_SetFrameBuffer(int layer, void *a, int f, int x, int y, int w, int h, int pi ) {
LTDC_Layer_TypeDef *const p = LTDC_Layer[layer];
uint32_t ps,uf,uw,uh,pitch,dw,dh,hmax,wmax;

    hmax      = display->height;
    wmax      = display->width;
    ps        = pixelsize[f];
    pitch     = pi;
    // Avoid extrapolation
    if ( (x+w) > wmax )
        w = (wmax-x)-1;
    if ( (y+h) > hmax )
        h = (hmax-y)-1;

    // Configure dimensions
    uh        = h;
    uw        = w;
    p->PFCR   = f;
    p->CFBAR  = (uint32_t) a;
    p->CFBLR  = (pitch<<LTDC_LxCFBLR_CFBP_Pos) | ((uw*ps+3)<<LTDC_LxCFBLR_CFBLL_Pos);
    p->CFBLNR = (uh<<LTDC_LxCFBLNR_CFBLNBR_Pos);

    // Get initial shift
    dw        = (LTDC->BPCR&LTDC_BPCR_AHBP_Msk)>>LTDC_BPCR_AHBP_Pos;
    dh        = (LTDC->BPCR&LTDC_BPCR_AVBP_Msk)>>LTDC_BPCR_AVBP_Pos;

    // Configure start and stop position
    p->WHPCR  = ((x+uw+dw)<<LTDC_LxWHPCR_WHSPPOS_Pos)
               |((x+dw+1)<<LTDC_LxWHPCR_WHSTPOS_Pos);
    p->WVPCR  = ((y+uh+dh)<<LTDC_LxWVPCR_WVSPPOS_Pos)
               |((y+dh+1)<<LTDC_LxWVPCR_WVSTPOS_Pos);

    // Enable layer, effective immediately
    LCD_EnableLayer(layer);
    LTDC->SRCR |= LTDC_SRCR_IMR;
}

/**
 * @brief Get Framebuffer height
 *
 * @note  Returns the number of lines
 */
int
LCD_GetHeight(int layer) {

    return (LTDC_Layer[layer]->CFBLNR&LTDC_LxCFBLNR_CFBLNBR_Msk)>>LTDC_LxCFBLNR_CFBLNBR_Pos;

}

/**
 * @brief Get Framebuffer width in pixels
 */
int
LCD_GetWidth(int layer) {
LTDC_Layer_TypeDef *p = LTDC_Layer[layer];
int w, format, ps;


    format = p->PFCR;
    ps = pixelsize[format];
    w = (p->CFBLR&LTDC_LxCFBLR_CFBLL_Msk)>>LTDC_LxCFBLR_CFBLL_Pos;
    w -= 3;
    w /= ps;

    return w;

}

/**
 * @brief Get Framebuffer pitch
 *
 * @note This is the distance in bytes
 */
int
LCD_GetPitch(int layer) {

    return (LTDC_Layer[layer]->CFBLR&LTDC_LxCFBLR_CFBP_Msk)>>LTDC_LxCFBLR_CFBP_Pos;

}


/**
 * @brief   Get Line Address
 *
 * @note    It uses the pitch information to calculate the start position
 *          of a line in buffer
 */
void *LCD_GetLineAddress(int layer, int line) {
LTDC_Layer_TypeDef *p = LTDC_Layer[layer];

    uint32_t base = p->CFBAR;
    uint32_t pitch = p->CFBLR>>LTDC_LxCFBLR_CFBP_Pos;

    return (void *) (base + line*pitch);
}

/**
 * @brief   fill1
 *
 * @note    fill a memory area with a 1 byte value
 *
 * @note    n = size in bytes!!!
 *
 */
static void fill1( void *area, int n, unsigned c) {
uint8_t uc;
uint8_t *p;
uint32_t uv;
uint32_t *q;

    p = (uint8_t *) area;
    uc = c&0xFF;
    // align to a word address (last two bits are zero)
    while( (n>0) && ((((uintptr_t)p)&0x3)!=0) ) {
        *p++ = uc;
        n--;
    }
    // after that, can fill 4 bytes at one
    uv = (uc<<24)|(uc<<16)|(uc<<8)|uc;
    q = (uint32_t *) p;
    while( n > 3 ) {
        *q++ = uv;
        n -= 4;
    }
    p = (uint8_t *) q;
    while( n>0 ) {
        *p++ = uc;
        n--;
    }

}

/**
 * @brief   fill2
 *
 * @note    Fill a memory area with a 16-bit value
 *
 * @note    n = size in bytes!!!
 *
 */
static void fill2( void *area, int n, unsigned c) {
uint8_t *p;
uint16_t uc;
uint32_t uv;
uint32_t *q;

    p = (uint8_t *) area;
    uc = c&0xFFFF;
    // align to a even address
    while( ((uintptr_t) p)&3 ) {
        *p++ = uc;
        n--;
        uc = (uc>>8)|(uc<<8);
    }
    // after that, can fill 4 bytes at one
    uv = (uc<<16)|uc;
    q = (uint32_t *) p;
    while( n > 3 ) {
        *q++ = uv;
        n -= 4;
    }
    // fill the remaining bytes
    p = (uint8_t *) q;
    while( n > 0 ) {
        *p++ = uc;
        n--;
        uc = (uc>>8)|(uc<<8);
    }
}


/**
 * @brief   fill3
 *
 * @note    Fill the frame buffer with a 3-byte value
 *
 * @note    n = size in bytes!!!
 *
 * @note    Memory organization
 *            word  | Pixel
 *         ---------|-----------------
 *            +0    | B1 R0 G0 B0
 *            +1    | G2 B2 R1 G1
 *            +2    | R3 G3 B3 R2
 */
static void fill3( void *area, int n, unsigned c) {
uint32_t w1,w2,w3;
uint8_t *p;
uint32_t uc;
uint32_t *q;

    p = (uint8_t *) area;
    uc = c&0xFFFFFF;
    // align to a even address
    while( ((uintptr_t) p)&3 ) {
        *p++ = uc;
        n--;
        uc = ((uc>>8)|(uc<<16))&0xFFFFFF;
    }
    // after that, can fill 4 bytes at one
    q = (uint32_t *) p;
    // uc = 0ABC
    w1 = (uc<<8)|(uc>>16);      // ABCA
    w2 = (uc<<16)|(uc>>8);      // BCAB
    w3 = (uc<<24)|uc;           // CABC
    while( n > 11 ) {
        *q++ = w3;
        *q++ = w2;
        *q++ = w1;
        n -= 12;
    }
    // fill the remaining bytes
    p = (uint8_t *) q;
    while( n > 0 ) {
        *p++ = uc;
        n--;
        uc = ((uc>>8)|(uc<<16))&0xFFFFFF;
    }
}

/*
 * @brief   fill4
 *
 * @note    Fill the frame buffer with a 4-byte value
 *
 * @note    n = size in bytes!!!
 *
 */

static void fill4( void *area, int n, unsigned c) {
uint8_t *p;
uint32_t uc;
uint32_t *q;

    p = (uint8_t *) area;
    uc = c;
    // align to a even address
    while( ((uintptr_t) p)&3 ) {
        *p++ = uc;
        n--;
        uc = (uc>>8)|(uc<<24);
    }
    // after that, can fill 4 bytes at one
    q = (uint32_t *) p;
    while( n > 3 ) {
        *q++ = uc;
        n -= 4;
    }
    // fill the remaining bytes
    p = (uint8_t *) q;
    while( n > 0 ) {
        *p++ = uc;
        n--;
        uc = (uc>>8)|(uc<<24);
    }

}


/*
 * @brief   LCD_FillFrameBuffer
 *
 * @note    Fill the frame buffer with a color using an efficient algorithm
 */
void
LCD_FillFrameBuffer(int layer, unsigned color ) {
LTDC_Layer_TypeDef *p = LTDC_Layer[layer];
int  ps;
char *area,*lineaddr;
int  w,h,pitch;
int i;

    ps     = LCD_GetPixelSize(layer);
    area   = (char *) LCD_GetFrameBufferAddress(layer);
    w      = LCD_GetWidth(layer);
    h      = LCD_GetHeight(layer);
    pitch  = LCD_GetPitch(layer);

    switch(ps) {
    case 1:
        w = LCD_GetPitch(layer);
        fill1(area,w*h,color);
        break;
    case 2:
        for(i=0;i<h;i++) {
            lineaddr = (char *) LCD_GetLineAddress(layer,i);
            fill2(lineaddr,pitch,color);
        }
        break;
    case 3:
        for(i=0;i<h;i++) {
            lineaddr = (char *) LCD_GetLineAddress(layer,i);
            fill3(lineaddr,pitch,color);
        }
        break;
    case 4:
        for(i=0;i<h;i++) {
            lineaddr = (char *) LCD_GetLineAddress(layer,i);
            fill4(lineaddr,pitch,color);
        }
        break;
    default:
        break;
    }
}


//////////////////////////// Drawing routines /////////////////////////////////////////////////////




/*
 * @brief   LCD_DrawHorizontalLine
 *
 * @note    Draw an horizontal line from point (x,y) with size 'size'
 */
void
LCD_DrawHorizontalLine(int layer, int x, int y, int size, unsigned color) {
int  ps,w,i;
char *lineaddr;
char *q;
uint8_t c1,c2,c3,c4;

    ps     = LCD_GetPixelSize(layer);
    w      = LCD_GetWidth(layer);

    if( (x+size) > w )
        size = w-x;

    lineaddr = (char *) LCD_GetLineAddress(layer,y);
    switch(ps) {
    case 1:
        q = lineaddr + x;
        c1 = color&0xFF;
        for(i=0;i<size;i++) {
            *q++ = c1;
        }
        break;
    case 2:
        q = lineaddr + x*2;
        c1 = color&0xFF;
        c2 = (color>>8)&0xFF;
        for(i=0;i<size;i++) {
            *q++ = c1;
            *q++ = c2;
        }
        break;
    case 3:
        q = lineaddr + x*3;
        c1 = color&0xFF;
        c2 = (color>>8)&0xFF;
        c3 = (color>>16)&0xFF;
        for(i=0;i<size;i++) {
            *q++ = c1;
            *q++ = c2;
            *q++ = c3;
        }
        break;
    case 4:
        q = lineaddr + x*4;
        c1 = color&0xFF;
        c2 = (color>>8)&0xFF;
        c3 = (color>>16)&0xFF;
        c4 = (color>>24)&0xFF;
        for(i=0;i<size;i++) {
            *q++ = c1;
            *q++ = c2;
            *q++ = c3;
            *q++ = c4;
        }
        break;
    }
}


/*
 * @brief   LCD_DrawVerticalLine
 *
 * @note    Draw an vertical line from point (x,y) with size 'size'
 */
void
LCD_DrawVerticalLine(int layer, int x, int y, int size, unsigned color) {
int  ps,w,h,i;
char *lineaddr;
char *q;
uint8_t c1,c2,c3,c4;

    ps     = LCD_GetPixelSize(layer);
    w      = LCD_GetWidth(layer);
    h      = LCD_GetHeight(layer);

    if( (y+size) > h )
        size = h-y;

    switch(ps) {
    case 1:
        c1 = color&0xFF;
        for(i=0;i<size;i++) {
            lineaddr = (char *) LCD_GetLineAddress(layer,y);
            q = lineaddr + x;
            *q = c1;
            y++;
        }
        break;
    case 2:
        q = lineaddr + x*2;
        c1 = color&0xFF;
        c2 = (color>>8)&0xFF;
        for(i=0;i<size;i++) {
            lineaddr = (char *) LCD_GetLineAddress(layer,y);
            q = lineaddr + 2*x;
            *q++ = c1;
            *q++ = c2;
            y++;
        }
        break;
    case 3:
        q = lineaddr + x*3;
        c1 = color&0xFF;
        c2 = (color>>8)&0xFF;
        c3 = (color>>16)&0xFF;
        for(i=0;i<size;i++) {
            lineaddr = (char *) LCD_GetLineAddress(layer,y);
            q = lineaddr + 3*x;
            *q++ = c1;
            *q++ = c2;
            *q++ = c3;
            y++;
        }
        break;
    case 4:
        c1 = color&0xFF;
        c2 = (color>>8)&0xFF;
        c3 = (color>>16)&0xFF;
        c4 = (color>>24)&0xFF;
        for(i=0;i<size;i++) {
            lineaddr = (char *) LCD_GetLineAddress(layer,y);
            q = lineaddr + 4*x;
            *q++ = c1;
            *q++ = c2;
            *q++ = c3;
            *q++ = c4;
            y++;
        }
        break;
    }
}

/*
 * @brief   LCD_DrawBox
 *
 * @note    Draw an horizontal line from point (x,y) with size 'size'
 */
void
LCD_DrawBox(int layer, int x, int y, int sizew, int sizeh, unsigned color, unsigned bordercolor) {
int  ps,w,h,i;
char *lineaddr;
char *q;
uint8_t c1,c2,c3,c4;

    ps     = LCD_GetPixelSize(layer);
    w      = LCD_GetWidth(layer);
    h      = LCD_GetHeight(layer);

    if( (x+sizew) > w )
        sizew = w-x;
    if( (y+sizeh) > h )
        sizeh = h-y;

    if( sizew <= 2 || sizeh <= 2 )
        return;

    LCD_DrawHorizontalLine(layer,x,y,sizew,bordercolor);
    LCD_DrawHorizontalLine(layer,x,y+sizeh,sizew,bordercolor);
    LCD_DrawVerticalLine(layer,x,y,sizeh,bordercolor);
    LCD_DrawVerticalLine(layer,x+sizew,y,sizeh,bordercolor);
    sizew -= 1;
    sizeh -= 1;
    x++;
    y++;
    for(i=0;i<sizeh;i++) {
        q = LCD_GetLineAddress(layer,y+i);
        q += ps*x;
        switch(ps) {
        case 1:
            fill1(q,sizew*ps,color);
            break;
        case 2:
            fill2(q,sizew*ps,color);
            break;
        case 3:
            fill3(q,sizew*ps,color);
            break;
        case 4:
            fill4(q,sizew*ps,color);
            break;
        }
    }
}

/*
 * @brief Mark point
 */
static void plot(char *p, int ps, unsigned color ) {

    switch(ps) {
    case 4: *p++ = (color>>24)&0xFF;
    case 3: *p++ = (color>>16)&0xFF;
    case 2: *p++ = (color>>8)&0xFF;
    case 1: *p++ = color&0xFF;
    }
}
/*
 * @brief   LCD_DrawLine
 *
 * @note    Draw an vertical line from point (x,y) with size 'size'
 */

#define ABS(X)  ((X)>0?(X):-(X))

void
LCD_DrawLine(int layer, int x, int y, int dx, int dy, unsigned color) {
enum   { Q0=0, Q1=1, Q2=5, Q3=4, Q4=6, Q5= 7, Q6=3, Q7=2 } octantcode;
int xi,yi;
int x1,x2,y1,y2;
int key;
int eps;
int ps,w,h,pitch;
char *lineaddr;

    ps       = LCD_GetPixelSize(layer);
    w        = LCD_GetWidth(layer);
    h        = LCD_GetHeight(layer);
    pitch    = LCD_GetPitch(layer);
    lineaddr = (char *) LCD_GetLineAddress(layer,y);

    if( (x+dx) > w )
        dx = w-x;
    if( (y+dy) > h )
        dy = h-y;

    // Build oct value setting bits according octant
    key = 0;
    // find quadrant first
    if( dx < 0 ) key |= 4;
    if( dy < 0 ) key |= 2;
    // find octant using rules according each quadrant
    if( ABS(dy) > ABS(dx) ) key |= 1;
    eps = 0;
    x1 = x;
    y1 = y;
    x2 = x+dx;
    y2 = y+dy;
    xi = x1;
    yi = y1;
    lineaddr = (char *) LCD_GetLineAddress(layer,yi);
    switch(key){
    case Q0: // 1st octant
        for(xi=x; xi<=x2;xi++) {
            plot(lineaddr+xi*ps,ps,color);
            eps += dy;
            if( (eps<<1) >= dx ) {
                yi++;
                eps -= dx;
                lineaddr += pitch;
            }
        }
        break;
    case Q1: // 2nd octant
        for(yi=y1; yi<=y2;yi++) {
            plot(lineaddr+xi*ps,ps,color);
            eps += dx;
            if( (eps<<1) >= dy ) {
                xi++;
                eps -= dy;
            }
            lineaddr += pitch;
        }
        break;
    case Q2: // 3rd octant
        for(yi=y1; yi<=y2;yi++) {
            plot(lineaddr+xi*ps,ps,color);
            eps -= dx;
            if( (eps<<1) >= dy ) {
                xi--;
                eps -= dy;
            }
            lineaddr += pitch;
        }
        break;
    case Q3: // 4th octant
        for(xi=x; xi>=x2;xi--) {
            plot(lineaddr+xi*ps,ps,color);
            eps += dy;
            if( (eps<<1) >= -dx ) {
                yi++;
                eps += dx;
                lineaddr += pitch;
            }
        }
        break;
    case Q4: // 5th octant
        for(xi=x; xi>=x2;xi--) {
            plot(lineaddr+xi*ps,ps,color);
            eps -= dy;
            if( (eps<<1) >= -dx ) {
                yi--;
                eps += dx;
                lineaddr -= pitch;
            }
        }
        break;
    case Q5: // 6th octant
        for(yi=y1; yi>=y2;yi--) {
            plot(lineaddr+xi*ps,ps,color);
            eps -= dx;
            if( (eps<<1) >= -dy ) {
                xi--;
                eps += dy;
            }
            lineaddr -= pitch;
        }
        break;
    case Q6: // 7th octant
        for(yi=y1; yi>=y2;yi--) {
            plot(lineaddr+xi*ps,ps,color);
            eps += dx;
            if( (eps<<1) >= -dy ) {
                xi++;
                eps += dy;
            }
            lineaddr -= pitch;
        }
        break;
    case Q7: // 8th octant
        for(xi=x; xi<=x2;xi++) {
            plot(lineaddr+xi*ps,ps,color);
            eps -= dy;
            if( (eps<<1) >= dx ) {
                yi--;
                eps -= dx;
                lineaddr -= pitch;
            }
        }
        break;
    }
}

