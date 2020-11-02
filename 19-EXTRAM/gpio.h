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

/**
 * @brief   Structure to hold information about pin initialization
 */
typedef struct {
    GPIO_TypeDef   *gpio;       /* GPIOA, GPIOB ... GPIOK */
    uint16_t         pin:4;     /* pin of port */
    uint16_t         af:4;      /* Alternate function */
} GPIO_PinConfiguration;

typedef struct {
    GPIO_TypeDef   *gpio;       /* GPIOA, GPIOB ... GPIOK */
    uint8_t         pin:4;      /* pin of port */
    uint8_t         mode:3;     /* Input/Output/Alternate/Analog */
    uint8_t         otype:2;    /* Output type */
    uint8_t         ospeed:2;   /* Low, Medium, High Speed or Very High Speed */
    uint8_t         pupd:2;     /* Pullup, Pulldown or nothing */
    uint8_t         af:4;       /* Alternate function */
    uint16_t        initial;    /* initial value for output*/
} GPIO_PinConfigurationEx;


void GPIO_Init(GPIO_TypeDef *gpio, uint32_t imask, uint32_t omask);

void GPIO_EnableClock(GPIO_TypeDef *gpio);


void GPIO_ConfigurePin( GPIO_TypeDef *gpio, uint32_t pin, uint32_t mode, uint32_t type,
                        uint32_t ospeed, uint32_t pupd, uint32_t af,uint32_t initialdata);
void GPIO_ConfigureMultiplePinsEx(const GPIO_PinConfigurationEx *pconfigtable);

void GPIO_ConfigureAlternateFunction(GPIO_TypeDef *gpio,int pin, int af, int ospeed);
void GPIO_ConfigureAlternateFunctionMultiple(const GPIO_PinConfiguration *pconfigtable);

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

