#ifndef LED_H
#define LED_H
/**
 * @file    led.h
 *
 * @date    07/10/2020
 * @author  Hans
 */

#include "stm32f746xx.h"
#include "system_stm32f746.h"

#include "gpio.h"

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

static void LED_Init(void) {
    GPIO_Init(LEDGPIO,0,LEDMASK);
}

static inline void LED_Set() {
    GPIO_Set(LEDGPIO,LEDMASK);
}

static inline void LED_Clear() {
    GPIO_Clear(LEDGPIO,LEDMASK);
}

static inline void LED_Toggle() {
    GPIO_Toggle(LEDGPIO,LEDMASK);
}
#endif

