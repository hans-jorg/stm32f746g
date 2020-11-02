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
 *          to configure pin.So the configuration of pin 6 is done in field in bits 13-12
 *          of these registers. All bits of the field must be zeroed before it is
 *          OR'ed with the mask. This is done by AND'ing the register with a mask, which
 *          is all 1 except for the bits in the specified field. The easy way to do it is
 *          complementing (exchangig 0 and 1) a mask with 1s in the desired field and 0
 *          everywhere else.
 *
 * @note    The MODE register is the most important. The LED pin must be configured
 *          for output. The field must be set to 1. The mask for the field is GPIO_MODE_M
 *          and the mask for the desired value is GPIO_MODE_V.
 *
 */

void
GPIO_Init(GPIO_TypeDef *gpio, uint32_t imask, uint32_t omask) {
uint32_t m;
uint32_t f;
int i,i2;


    /* Enable clock for gpio unit */
    GPIO_EnableClock(gpio);

    /*
     * Configure GPIO according parameters imask e omask
     */

    for(i=0;i<16;i++) {
        m =  1<<i;               /* mask for bit for pin            */
        i2 = 2*i;
        f =  3<<i2;           /* mask for 2 bit field for pin    */

        if( imask&m ) {         /* pin is input */
            gpio->MODER    = (gpio->MODER&~f)|(INPUTMODE<<i2);
        } else if( omask&m ) {  /* pin is output */
            gpio->MODER    = (gpio->MODER&~f)|(OUTPUTMODE<<i2);
            // Set pin type
            gpio->OTYPER   = (gpio->OTYPER&~f)|(OUTPUTTYPE<<i2);
            // Set pin SPEED)
            gpio->OSPEEDR  = (gpio->OSPEEDR&~f)|(OUTPUTSPEED<<i2);
            // Set pullup/pushdown resistors configuration
            gpio->PUPDR    = (gpio->PUPDR&~f)|(OUTPUTPUPDR<<i2);
            // Set pin to 0
            gpio->ODR     &=  ~m;
        }
    }

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
 * @brief   GPIO_ConfigureAlternateFunction
 */
void
GPIO_ConfigureAlternateFunction(GPIO_TypeDef *gpio,int pin, int af, int ospeed) {

    GPIO_EnableClock(gpio);

    if( pin < 8 ) { // Use AFRL
        gpio->AFR[0] = (gpio->AFR[0]&~(0xF<<(4*pin)))|(af<<(4*pin));
    } else {            // Use AFRH
        gpio->AFR[1] = (gpio->AFR[1]&~(0xF<<(4*pin-32)))|(af<<(4*pin-32));
    }
    /* Configure pin to use alternate function */
    gpio->MODER   = (gpio->MODER&~(3>>(2*pin)))|(2<<(2*pin));
    gpio->OSPEEDR = (gpio->OSPEEDR&~(3>>(2*pin)))|(ospeed<<(2*pin));
    gpio->PUPDR   = (gpio->PUPDR&~(3>>(2*pin)))|(0<<(2*pin));
    gpio->OTYPER  = (gpio->OTYPER&~(1>>(pin)))|(0<<(pin));

}

void GPIO_ConfigureAlternateFunctionMultiple(const GPIO_PinConfiguration *pconfig) {

    while( pconfig->gpio ) {
        GPIO_ConfigureAlternateFunction(pconfig->gpio,pconfig->pin,pconfig->af,3);
        pconfig++;
    }
}

void GPIO_ConfigurePinEx( GPIO_TypeDef *gpio, uint32_t pin, uint32_t mode, uint32_t otype,
                        uint32_t ospeed, uint32_t pupd, uint32_t af,uint32_t initial) {


    GPIO_EnableClock(gpio);

    switch(mode) {
    case 0:         /* INPUT */
        gpio->MODER = (gpio->MODER&~(3>>(2*pin)))     | (0*(2*pin));
        gpio->OTYPER  = (gpio->OTYPER&~(1>>(pin)))    | (otype<<(pin));
        gpio->OSPEEDR = (gpio->OSPEEDR&~(3>>(2*pin))) | (ospeed<<(2*pin));
        gpio->PUPDR   = (gpio->PUPDR&~(3>>(2*pin)))   | (pupd<<(2*pin));
        break;
    case 1:         /* OUTPUT */
        gpio->MODER   = (gpio->MODER&~(3>>(2*pin)))   | (2*(2*pin));
        gpio->OTYPER  = (gpio->OTYPER&~(1>>(pin)))    | (otype<<(pin));
        gpio->OSPEEDR = (gpio->OSPEEDR&~(3>>(2*pin))) | (ospeed<<(2*pin));
        gpio->PUPDR   = (gpio->PUPDR&~(3>>(2*pin)))   | (pupd<<(2*pin));
        break;
    case 2:         /*& Alternate function */
         if( pin < 8 ) { // Use AFRL
            gpio->AFR[0] = (gpio->AFR[0]&~(0xF<<(4*pin)))    | (af<<(4*pin));
        } else {            // Use AFRH
            gpio->AFR[1] = (gpio->AFR[1]&~(0xF<<(4*pin-32))) | (af<<(4*pin-32));
        }
        gpio->MODER   = (gpio->MODER&~(3>>(2*pin)))   | (2*(2*pin));
        gpio->OTYPER  = (gpio->OTYPER&~(1>>(pin)))    | (otype<<(pin));
        gpio->OSPEEDR = (gpio->OSPEEDR&~(3>>(2*pin))) | (ospeed<<(2*pin));
        gpio->PUPDR   = (gpio->PUPDR&~(3>>(2*pin)))   | (pupd<<(2*pin));
        break;
    case 3:         /* Analog */
        gpio->MODER   = (gpio->MODER&~(3>>(2*pin)))   | (2*(2*pin));
        gpio->OTYPER  = (gpio->OTYPER&~(1>>(pin)))    | (otype<<(pin));
        gpio->OSPEEDR = (gpio->OSPEEDR&~(3>>(2*pin))) | (ospeed<<(2*pin));
        gpio->PUPDR   = (gpio->PUPDR&~(3>>(2*pin)))   | (pupd<<(2*pin));
        break;
    }
}

void GPIO_ConfigureMultiplePinsEx(const GPIO_PinConfigurationEx *p) {

    while( p->gpio ) {
        GPIO_ConfigurePinEx(p->gpio,p->pin,p->mode,p->otype,
                            p->ospeed,p->pupd,p->af,p->initial);
        p++;
    }
}

