#ifndef GPIO_H
#define GPIO_H
/**
 * @file    gpio.h
 *
 * @date    07/10/2020
 * @author  Hans
 */

#include "stm32f746xx.h"
#include "system_stm32f746.h"

void GPIO_Init(GPIO_TypeDef *gpio, uint32_t imask, uint32_t omask);

static inline void GPIO_Set(GPIO_TypeDef *gpio, uint32_t mask) {
        /* Writing a 1 to lower 16 bits of BSRR set the corresponding bit */
        gpio->BSRR = mask;            // Turn on bits
}

static inline void GPIO_Clear(GPIO_TypeDef *gpio, uint32_t mask) {
        /* Writing a 1 to upper 16 bits of BSRR clear the correspoding bit */
        gpio->BSRR = (mask<<16);      // Turn off bits
}

static inline void GPIO_Toggle(GPIO_TypeDef *gpio, uint32_t mask) {
        /* This is a read/modify/write sequence */
       gpio->ODR ^= mask;             // Use XOR to toggle output
}

static inline uint32_t GPIO_Read(GPIO_TypeDef *gpio, uint32_t mask) {
       return gpio->IDR;
}
#endif

