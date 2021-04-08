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

#include "stm32f746xx.h"
#include "system_stm32f746.h"

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
 * @brief LED Symbols
 *
 * @note    It is at pin 1 of Port I.
 *
 * @note    Not documented. See schematics
 *
 */

///@{
#define LEDPIN              (1)
#define LEDGPIO             GPIOI
#define LEDMASK             BIT(LEDPIN)
///@}

/**
 * @brief   Quick and dirty delay routine
 *
 * @note    It gives approximately 1ms delay at 16 MHz
 *
 * @note    The COUNTERFOR1MS must be adjusted by trial and error
 *
 * @note    Do not use this or similar in production code
 */

#define COUNTERFOR1MS 10000


void ms_delay(volatile int ms) {
   while (ms-- > 0) {
      volatile int x=COUNTERFOR1MS;
      while (x-- > 0)
         __NOP();
   }
}

/**
 * @brief   GPIO Configuration Symbols for LED
 *
 * @note    For many applications, the LEDMASK is enough. This is the case of
 *          the ODR. Each port has 16 pins so only the 16 least significant bits
 *          of a 32 bit register are used.
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
 * @note    The OSSPEED,
 *
 * @note
 */

///@{
// Pin configuration
#define LEDMODE             1
#define LEDOTYPE            0
#define LEDOSPEED           3
#define LEDPUPD             0
///@}


/**************************************************************************************************/
/**
 * @brief   A very crude GPIO HAL
 */
///@{

void GPIO_Init(void) {

    /*
     * Enable clock for GPIOI
     */
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOIEN;
    __DSB();
}

void GPIO_ConfigureOutputPin(int pin) {
uint32_t mask;

    mask = 3U<<(2*pin);
    /*
     * Configure GPIO to drive LED
     */

    // Set LED pin to output
    LEDGPIO->MODER    = (LEDGPIO->MODER&~mask)|(LEDMODE<<(2*pin));
    // Set pin type
    LEDGPIO->OTYPER   = (LEDGPIO->OTYPER&~mask)|(LEDOTYPE<<(2*pin));
    // Set pin SPEED)
    LEDGPIO->OSPEEDR  = (LEDGPIO->OSPEEDR&~mask)|(LEDOSPEED<<(2*pin));
    // Set pullup/pushdown resistors configuration
    LEDGPIO->PUPDR    = (LEDGPIO->PUPDR&~mask)|(LEDPUPD<<(2*pin));
    // Turn off LED
    LEDGPIO->ODR     &=  ~(1<<pin);
}

void GPIO_TogglePin(int pin) {
uint32_t mask = 1U<<pin;

    LEDGPIO->ODR ^= mask;                // Use XOR to toggle output
}

void GPIO_SetPin(int pin) {
uint32_t mask = 1U<<pin;

    LEDGPIO->ODR |= mask;                // Use OR to set output
// There is an alternative using BSRR register
//  LEDGPIO->BSRR = mask;
}

void GPIO_ClearPin(int pin) {
uint32_t mask = 1U<<pin;

    LEDGPIO->ODR &= ~mask;              // Use OR to set output
// There is an alternative using BSRR register
//  LEDGPIO->BSRR = mask<<16;
}
///@}

/**************************************************************************************************/

/**
 * @brief   main
 *
 * @note    Initializes GPIO and blinks LED
 *
 * @note    Really a bad idea to blink LED
 */

int main(void) {

    GPIO_Init();

    GPIO_ConfigureOutputPin(LEDPIN);

    /*
     * Blink LED
     */
    for (;;) {
#if 1
       ms_delay(500);
       GPIO_TogglePin(LEDPIN);
#else
       /* Alternative
        * Writing a 1 to lower 16 bits of BSRR set the corresponding bit
        * Writing a 1 to upper 16 bits of BSRR clear the correspoding bit
       */
        ms_delay(500);
        GPIO_SetPin(LEDPIN);
        ms_delay(500);
        GPIO_ClearPin(LEDPIN);
#endif
    }
}
