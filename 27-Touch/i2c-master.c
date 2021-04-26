/**
 * @file    i2c-master.c
 *
 * @brief   I2C implementarion of master interface
 *
 * @note    Simple implementation. Configured to use 16 MHz HSI as clock source
 *
 * @author  Hans
 */

#include "stm32f746xx.h"
#include "system_stm32f746.h"
#include "i2c-master.h"
#include "gpio.h"

/**
 *  @brief  Data structure to store information about I2C Configuration
 */

typedef struct {
    I2C_TypeDef             *i2c;
    GPIO_PinConfiguration   sclpin;
    GPIO_PinConfiguration   sdapin;
} I2C_Configuration_t;

typedef uint32_t I2C_Timing_t[4];
                                      //                      TIMING
                                      //    None        Analog      DNF=1       DNF=2
static I2C_Timing_t   timing_normal   = {0x00503D5A, 0x00503D58, 0x00503C59, 0x00503B58 };
static I2C_Timing_t   timing_fast     = {0x00300718, 0x00300617, 0x00300617, 0x00300912 };
static I2C_Timing_t   timing_fastplus = {0x00200205, 0x00200105, 0x00200004, 0x00200003 };

/**
 * |  I2C   |    SCL             |           SDA--------------\
 * |--------|--------------------|----------------------------|
 * |  I2C1  |  PB6 *PB8*         |  PB7 *PB9*                 \
 * |  I2C2  |  PB10 PF1 PH4      |  PB11 PF0 PH5              \
 * |  I2C3  |  PA8 *PH7*         |  PC9 *PH8*                 \
 * |  I2C4  |  PD12 PF14 PH11    |  PD13 PF15 PH12            \
 *
 * I2C3 at PH7 and PH* used for LCD and AUDIO I2C
 * I2C1 at PB8 and PB9 used for EXT I2C (Arduino connectors)
 * Other I2C has pin usage conflicts
 *
 */
static I2C_Configuration_t i2c_configuration[] = {
    //          SCL            SDA
    //      GPIO  Pin AF   GPIO  Pin AF
    { I2C1, {GPIOB, 8, 4}, {GPIOB, 9, 4} },
    { I2C2, {    0,10, 4}, {GPIOB,11, 4} },    // Disabled
    { I2C3, {GPIOH, 7, 4}, {GPIOH, 8, 4} },
    { I2C4, {    0,12, 4}, {GPIOD,13, 4} },    // Disabled
    { 0,    {    0, 0, 0}, {    0, 0, 0} }
};

/**
 *  @brief  I2CMaster_ClockEnable
 *
 *  @note   Using HSI as I2CCLK clock source
 */
#define I2CLKSRC (2)

static void
I2CMaster_ClockEnable( I2C_TypeDef *i2c ) {

    // Enable Peripheral Clock 1

    // Enable I2C clocks
    if ( i2c == I2C1 ) {
        RCC->APB1ENR |= RCC_APB1ENR_I2C1EN_Msk;
        RCC->DCKCFGR2 =  (RCC->DCKCFGR2&~(3<<RCC_DCKCFGR2_I2C1SEL_Pos))
                        |(I2CLKSRC<<RCC_DCKCFGR2_I2C1SEL_Pos);
    } else if ( i2c == I2C2 ) {
        RCC->APB1ENR |= RCC_APB1ENR_I2C2EN_Msk;
        RCC->DCKCFGR2 =  (RCC->DCKCFGR2&~(3<<RCC_DCKCFGR2_I2C2SEL_Pos))
                        |(I2CLKSRC<<RCC_DCKCFGR2_I2C2SEL_Pos);
    } else if ( i2c == I2C3 ) {
        RCC->APB1ENR |= RCC_APB1ENR_I2C3EN_Msk;
        RCC->DCKCFGR2 =  (RCC->DCKCFGR2&~(3<<RCC_DCKCFGR2_I2C3SEL_Pos))
                        |(I2CLKSRC<<RCC_DCKCFGR2_I2C3SEL_Pos);
    } else if ( i2c == I2C4 ) {
        RCC->APB1ENR |= RCC_APB1ENR_I2C4EN_Msk;
        RCC->DCKCFGR2 =  (RCC->DCKCFGR2&~(3<<RCC_DCKCFGR2_I2C4SEL_Pos))
                        |(I2CLKSRC<<RCC_DCKCFGR2_I2C4SEL_Pos);
    }

}

/**
 *  @brief  I2CMaster_Init
 *
 */
int
I2CMaster_Init( I2C_TypeDef *i2c, uint32_t conf) {
I2C_Configuration_t *p;


    p = i2c_configuration;
    while( p->i2c && (i2c!=p->i2c) ) p++;

    // Not found!!
    if( ! p->i2c )
        return -1;
    // Pins not configurable
    if( (p->sclpin.gpio == 0) || (p->sdapin.gpio == 0) )
        return -2;


    // For now, analog and digital filters are exclusive
    if( (conf&I2C_CONF_FILTER_ANALOG) && (conf&I2C_CONF_FILTER_DIGITAL_MASK) )
        return -3;

    uint32_t dnf = (conf&I2C_CONF_FILTER_DIGITAL_MASK)>>I2C_CONF_FILTER_DIGITAL_Pos;
    if( dnf > 2 )
        return -4;
    //
    // Configure pins when possible

    GPIO_ConfigureSinglePin(&(p->sclpin));
    GPIO_ConfigureSinglePin(&(p->sdapin));

    // Turn Peripheral Clock for the I2C interface
    I2CMaster_ClockEnable(i2c);

    // Turn off device (Three times, see Note in RM Section 30.7.1 */
    i2c->CR1 &= ~I2C_CR1_PE;
    i2c->CR1 &= ~I2C_CR1_PE;
    i2c->CR1 &= ~I2C_CR1_PE;


    // Configure filters. Reverse logic: 1 means off
    int index = 0;
    if( conf&I2C_CONF_FILTER_ANALOG ) {
        i2c->CR1 &= ~I2C_CR1_ANFOFF;
        index = 1;
    } else {
        i2c->CR1 |= I2C_CR1_ANFOFF;
        if( dnf ) {
            i2c->CR1 = (i2c->CR1&~(I2C_CR1_DNF_Msk))|(dnf<<I2C_CR1_DNF_Pos);
            index = dnf+1;      // Only digital
        }
    }

    uint32_t t = 0;
    // Configure timing
    switch( conf&I2C_CONF_MODE_MASK ) {
    case I2C_CONF_MODE_NORMAL:
        t = timing_normal[index];
        break;
    case I2C_CONF_MODE_FAST:
        t = timing_fast[index];
        break;
    case I2C_CONF_MODE_FASTPLUS:
        t = timing_fastplus[index];
        break;
    }
    i2c->TIMINGR = t;

    // Turn on device */
    i2c->CR1 |= I2C_CR1_PE_Msk;

    return 0;
}

