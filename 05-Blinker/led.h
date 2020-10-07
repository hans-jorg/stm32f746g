/**
 * @file    led.h
 *
 * @date    07/10/2020
 * @author  Hans
 */

#include "stm32f746xx.h"
#include "system_stm32f746.h"

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
#define LEDMASK             (1U<<(LEDPIN))
///@}

void LED_Init(void);

static inline void LED_Set() {
        /* Writing a 1 to lower 16 bits of BSRR set the corresponding bit */
        LEDGPIO->BSRR = LEDMASK;            // Turn on LED
}

static inline void LED_Clear() {
        /* Writing a 1 to upper 16 bits of BSRR clear the correspoding bit */
        LEDGPIO->BSRR = (LEDMASK<<16);      // Turn off LED
}

static inline void LED_Toggle() {
        /* This is a read/modify/write sequence */
       LEDGPIO->ODR ^= LEDMASK;             // Use XOR to toggle output
}

