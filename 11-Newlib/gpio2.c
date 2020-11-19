/**
 * @file    gpio.c
 *
 * @date    07/10/2020
 * @author  Hans
 */

#include "stm32f746xx.h"
#include "system_stm32f746.h"
#include "gpio.h"

/**
 * @defgroup defines-1
 *
 * @brief   Pin configuration
 */

/**
 * @addtogroup defines-1
 * @{
 */

#define INPUTMODE           0
#define OUTPUTMODE          1
#define OUTPUTTYPE          0
#define OUTPUTSPEED         3
#define OUTPUTPUPDR         0
/**@} */


#define PUPDDEFAULT     (0)
#define OTYPEDEFAULT    (0)

/**
 * @defgroup    macros-1
 * @brief Macros for bit and bitmask definition
 *
 * @note                    Least Significant Bit (LSB) is 0
 *
 * BIT(N)                   Creates a bit mask with only the bit N set
 * SHIFTLEFT(V,N)           Shifts the value V so its LSB is at position N
 */

/**
 * @addtogroup macros-1
 * @{
 */

#define BIT(N)                          (1UL<<(N))
#define SHIFTLEFT(V,N)                  ((V)<<(N))
/** @} */

/**
 * @brief   GPIO Init
 *
 * @param   gpio Pointer to a GPIO register area. Can be GPIOA..GPIOK
 * @param   imask The pins corresponding to a bit set are configured as input
 * @param   omask The pins corresponding to a bit set are configured as output
 *
 * @note    When configured as input and output, a pin is configured as input (safer)
 *
 * @note    Many registers like MODER,OSPEER and PUPDR use a 2-bit field
 *          to configure pin.So the configuration of pin 6 is done in field in
 *          bits 13-12 of these registers. All bits of the field must be zeroed
 *          before it is OR'ed with the mask. This is done by AND'ing the register
 *          with a mask, which is all 1 except for the bits in the specified field.
 *          The easy way to do it is complementing (exchangig 0 and 1) a mask with
 *          1s in the desired field and 0 everywhere else.
 *
 * @note    The MODE register is the most important. The LED pin must be configured
 *          for output. The field must be set to 1. The mask for the field is
 *          GPIO_MODE_M and the mask for the desired value is GPIO_MODE_V.
 *
 */

static GPIO_PinConfiguration defaultinput = {
    .gpio   = 0,    // not used
    .pin    = 0,    // not used
    .mode   = 1,    // input
    .otype  = 0,    //
    .ospeed = 0,    //
    .pupd   = 0,    // pull-up or pull-down
    .initial= 0
};

static GPIO_PinConfiguration defaultoutput = {
    .gpio   = 0,    // not used
    .pin    = 0,    // not used
    .mode   = 2,    // output
    .otype  = 0,    //
    .ospeed = 0,    //
    .pupd   = 0,    // pull-up or pull-down
    .initial= 0
};


void
GPIO_Init(GPIO_TypeDef *gpio, uint32_t imask, uint32_t omask) {
uint32_t m;
uint32_t f;
int pos,pos2;
uint32_t moder, otyper, ospeedr, pupdr, odr;

    /* Enable clock for gpio unit */
    GPIO_EnableClock(gpio);

    GPIO_ConfigureMultiplePinsEqual( gpio, imask, &defaultinput );
    GPIO_ConfigureMultiplePinsEqual( gpio, omask, &defaultoutput );

}

/**
 * @brief   GPIO_EnableClock
 */

void
GPIO_EnableClock(GPIO_TypeDef *gpio) {
uint32_t m;

    /* Enable clock for GPIO */
    if( gpio == GPIOA ) m=RCC_AHB1ENR_GPIOAEN;
    else if ( gpio == GPIOB ) m=RCC_AHB1ENR_GPIOBEN;
    else if ( gpio == GPIOC ) m=RCC_AHB1ENR_GPIOCEN;
    else if ( gpio == GPIOD ) m=RCC_AHB1ENR_GPIODEN;
    else if ( gpio == GPIOE ) m=RCC_AHB1ENR_GPIOEEN;
    else if ( gpio == GPIOF ) m=RCC_AHB1ENR_GPIOFEN;
    else if ( gpio == GPIOG ) m=RCC_AHB1ENR_GPIOGEN;
    else if ( gpio == GPIOH ) m=RCC_AHB1ENR_GPIOHEN;
    else if ( gpio == GPIOI ) m=RCC_AHB1ENR_GPIOIEN;
    else if ( gpio == GPIOJ ) m=RCC_AHB1ENR_GPIOJEN;
    else if ( gpio == GPIOK ) m=RCC_AHB1ENR_GPIOKEN;
    else    m = 0;
    RCC->AHB1ENR |= m;

}


/**
 * @brief   Configure Pin
 */
void GPIO_ConfigureSinglePin(const GPIO_PinConfiguration *conf) {
GPIO_TypeDef *gpio;
int pos2,pos4;
int pos;

    gpio = conf->gpio;

    GPIO_EnableClock(gpio);

    pos = conf->pin;
    pos2 = pos*2;
    pos4 = pos*4;

    if ( conf->af != 0 ) {
        /* Configure pin to use alternate function */
        if( pos < 8 ) {     // Use AFRL
            gpio->AFR[0] = (gpio->AFR[0]&~(0xF<<pos4))|(conf->af<<pos4);
        } else {            // Use AFRH
            pos4 -= 32;
            gpio->AFR[1] = (gpio->AFR[1]&~(0xF<<pos4))|(conf->af<<pos4);
        }
        // Set remain configurations to default
        gpio->MODER   = (gpio->MODER&~(3<<pos2));
        gpio->OSPEEDR = (gpio->OSPEEDR&~(3<<pos2));
        gpio->PUPDR   = (gpio->PUPDR&~(3<<pos2));
        gpio->OTYPER  = (gpio->OTYPER&~(BIT(pos)));
        gpio->ODR     = (gpio->ODR&~BIT(pos));
    } else {
        /* Configure pin to use GPIO function */
        if( pos < 8 ) {     // Use AFRL
            gpio->AFR[0] = (gpio->AFR[0]&~(0xF<<pos4));
        } else {            // Use AFRH
            gpio->AFR[1] = (gpio->AFR[1]&~(0xF<<(pos4-32)));
        }
        gpio->MODER   = (gpio->MODER&~(3<<pos2))|(conf->mode<<pos2);
        gpio->OSPEEDR = (gpio->OSPEEDR&~(3<<pos2))|(conf->ospeed<<pos2);
        gpio->PUPDR   = (gpio->PUPDR&~(3<<pos2))|(conf->pupd<<pos2);
        gpio->OTYPER  = (gpio->OTYPER&~(BIT(pos)))|(conf->otype<<pos);
        gpio->ODR     = (gpio->ODR&~BIT(pos))|(conf->initial<<pos);
    }
}

/**
 * @brief   GPIO Configure all pins in an array
 */
void GPIO_ConfigureMultiplePins(const GPIO_PinConfiguration *pconfig) {

    while( pconfig->gpio ) {
        GPIO_ConfigureSinglePin(pconfig);
        pconfig++;
    }
}


/**
 * @brief   GPIO_ConfigurePinSimple
 */
void
GPIO_ConfigurePinSimple(GPIO_TypeDef *gpio, unsigned pin, unsigned af) {
unsigned pos2,pos4;

    GPIO_EnableClock(gpio);

    pos2 = pin*2;
    pos4 = pin*4;

    /* Configure pin to use alternate function */
    if ( af != 0 ) {
        gpio->MODER = (gpio->MODER&~(3<<pos2))|(2<<pos2);
    }

    /* Configure pin which alternate function */
    if( pin < 8 ) { // Use AFRL
        gpio->AFR[0] = (gpio->AFR[0]&~(0xF<<pos4))|(af<<pos4);
    } else {            // Use AFRH
        gpio->AFR[1] = (gpio->AFR[1]&~(0xF<<(pos4-32)))|(af<<(4*pin-32));
    }

    /*
    gpio->OSPEEDR = (gpio->OSPEEDR&~(3<<pos2))|(ospeed<<pos2);
    gpio->PUPDR   = (gpio->PUPDR&~(3<<pos2))|(PUPDDEFAULT<<pos2);
    gpio->OTYPER  = (gpio->OTYPER&~BIT(pin))|(OTYPEDEFAULT<<(pin));
    */
}

/**
 * @brief   GPIO_ConfigureAlternateFunction
 */
void GPIO_ConfigurePinFull( GPIO_TypeDef *gpio,
                                unsigned pin,
                                unsigned af,
                                unsigned mode,
                                unsigned otype,
                                unsigned ospeed,
                                unsigned pupd,
                                unsigned init) {
unsigned pos2,pos4;

    GPIO_EnableClock(gpio);

    pos2 = pin*2;
    pos4 = pin*4;

    switch(mode) {
    case 0:         /* INPUT */
        gpio->MODER   = (gpio->MODER&~(3<<pos2))   | (0*pos2);
        gpio->OTYPER  = (gpio->OTYPER&~(1<<(pin))) | (otype<<(pin));
        gpio->OSPEEDR = (gpio->OSPEEDR&~(3<<pos2)) | (ospeed<<pos2);
        gpio->PUPDR   = (gpio->PUPDR&~(3<<pos2))   | (pupd<<pos2);
        break;
    case 1:         /* OUTPUT */
        gpio->MODER   = (gpio->MODER&~(3<<pos2))   | (1<<pos2);
        gpio->OTYPER  = (gpio->OTYPER&~(1<<(pin))) | (otype<<(pin));
        gpio->OSPEEDR = (gpio->OSPEEDR&~(3<<pos2)) | (ospeed<<pos2);
        gpio->PUPDR   = (gpio->PUPDR&~(3<<pos2))   | (pupd<<pos2);
        gpio->ODR     = (gpio->ODR&~(1<<(pin)))    | (init<<pos2);
        break;
    case 2:         /*& Alternate function */
         if( pin < 8 ) {    // Use AFRL
            gpio->AFR[0] = (gpio->AFR[0]&~(0xF<<pos4))    | (af<<pos4);
        } else {            // Use AFRH
            gpio->AFR[1] = (gpio->AFR[1]&~(0xF<<(4*pin-32))) | (af<<(4*pin-32));
        }
        gpio->MODER   = (gpio->MODER&~(3<<pos2))   | (2<<pos2);
        gpio->OTYPER  = (gpio->OTYPER&~(1<<(pin))) | (otype<<(pin));
        gpio->OSPEEDR = (gpio->OSPEEDR&~(3<<pos2)) | (ospeed<<pos2);
        gpio->PUPDR   = (gpio->PUPDR&~(3<<pos2))   | (pupd<<pos2);
        break;
    case 3:         /* Analog */
        gpio->MODER   = (gpio->MODER&~(3<<pos2))   | (3<<pos2);
        gpio->OTYPER  = (gpio->OTYPER&~(1<<(pin))) | (otype<<(pin));
        gpio->OSPEEDR = (gpio->OSPEEDR&~(3<<pos2)) | (ospeed<<pos2);
        gpio->PUPDR   = (gpio->PUPDR&~(3<<pos2))   | (pupd<<pos2);
        break;
    }
}


/**
 * @brief   Configure all pins specified by a bit mask
 *          with the configuration in a GPIO_PinConfiguration struct
 */
void GPIO_ConfigureMultiplePinsEqual( GPIO_TypeDef *gpio,
                                unsigned pinmask,
                                GPIO_PinConfiguration *conf ) {
int pin;
unsigned m;

    /* Enable clock for gpio unit */
    GPIO_EnableClock(gpio);

    conf->gpio = gpio;
    for(pin=0;pin<16;pin++) {
        m =  BIT(pin);               /* mask for bit for pin            */

        if( pinmask&m ) {
            conf->pin = pin;
            GPIO_ConfigureSinglePin( conf );
        }
    }
}

/**
 * @brief   Get pin configuration ( GPIO_TypeDef *gpio, int pin, )
 */
void GPIO_GetPinConfiguration( GPIO_TypeDef *gpio,
                                unsigned pin,
                                GPIO_PinConfiguration *conf) {
unsigned pos2,pos4;

    conf->gpio = gpio;
    conf->pin  = pin;

    pos2 = 2*pin;
    pos4 = 4*pin;

    if( pin < 8 ) {
        conf->af = (gpio->AFR[0]>>pos4)&0xF;
    } else {
        conf->af = (gpio->AFR[1]>>(pos4-32))&0xF;
    }
    conf->mode   = (gpio->MODER>>pos2)&0x3;
    conf->otype  = (gpio->OTYPER>>pin)&0x1;
    conf->ospeed = (gpio->OSPEEDR>>pos2)&0x3;
    conf->pupd   = (gpio->PUPDR>>pos2)&0x3;
    conf->initial= (gpio->ODR>>pin)&0x1;

}

