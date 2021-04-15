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
 * @brief   Pin configuration
 */
///@{
#define INPUTMODE           0
#define OUTPUTMODE          1
#define OUTPUTTYPE          0
#define OUTPUTSPEED         3       /* High Speed */
#define OUTPUTPUPDR         0

///@}
/**
 * @brief Macros for bit and SHIFTLEFTs manipulation
 *
 * @note                    Least Significant Bit (LSB) is 0
 *
 * BIT(N)                   Creates a bit mask with only the bit N set
 * SHIFTLEFT(V,N)           Shifts the value V so its LSB is at position N
 */

///@{
#define BIT(N)                          (1UL<<(N))
#define SHIFTLEFT(V,N)                  ((V)<<(N))
///@}

/**
 * @brief   GPIO Configuration

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

void GPIO_Init(GPIO_TypeDef *gpio, uint32_t imask, uint32_t omask) {
uint32_t m;
uint32_t f;
int i,i2;

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
    __DSB();

    /*
     * Configure GPIO according parameters imask e omask
     */

    for(i=0;i<16;i++) {
        m =  1<<i;               /* mask for bit for pin            */
        i2 = 2*i;
        f =  3<<i2;             /* mask for 2 bit field for pin    */

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

