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
    unsigned        pin:4;      /* pin of port */
    unsigned        af:4;       /* Alternate function */
    unsigned        mode:3;     /* Input/Output/Alternate/Analog */
    unsigned        otype:2;    /* Output type */
    unsigned        ospeed:2;   /* Low, Medium, High Speed or Very High Speed */
    unsigned        pupd:2;     /* Pullup, Pulldown or nothing */
    unsigned        initial:1;  /* initial value for output*/
} GPIO_PinConfiguration;

/// Main configuration
void GPIO_Init(GPIO_TypeDef *gpio, uint32_t imask, uint32_t omask);
void GPIO_EnableClock(GPIO_TypeDef *gpio);

/// Pin configuration explicitely
void GPIO_ConfigurePinFull( GPIO_TypeDef *gpio,
                                unsigned pin,
                                unsigned af,
                                unsigned mode,
                                unsigned type,
                                unsigned ospeed,
                                unsigned pupd,
                                unsigned init);

void GPIO_ConfigurePinFunction( GPIO_TypeDef *gpio,
                                unsigned pin,
                                unsigned af);

/// Get pin configuration ( GPIO_TypeDef *gpio, int pin, )
void GPIO_GetPinConfiguration( GPIO_TypeDef *gpio,
                                unsigned pin,
                                GPIO_PinConfiguration *conf);

/// Configure pin based on a PinConfiguration structure
void GPIO_ConfigureSinglePin( const GPIO_PinConfiguration *conf );

/// Configure pins based on a array of PinConfiguration
void GPIO_ConfigureMultiplePins( const GPIO_PinConfiguration *conf );

/// Configure pins specified by a bit mask from a GPIO_PinConfiguration struct
void GPIO_ConfigureMultiplePinsEqual( GPIO_TypeDef *gpio,
                                unsigned pinmask,
                                GPIO_PinConfiguration *conf );


/// Inline functions to access input and to set, clear and toggle output
static inline void GPIO_Set( GPIO_TypeDef *gpio, uint32_t mask ) {
        /* Writing a 1 to lower 16 bits of BSRR set the corresponding bit */
        gpio->BSRR = mask;            // Turn on bits
}

static inline void GPIO_Clear( GPIO_TypeDef *gpio, uint32_t mask ) {
        /* Writing a 1 to upper 16 bits of BSRR clear the correspoding bit */
        gpio->BSRR = (mask<<16);      // Turn off bits
}

static inline void GPIO_Toggle( GPIO_TypeDef *gpio, uint32_t mask ) {
        /* This is a read/modify/write sequence */
       gpio->ODR ^= mask;             // Use XOR to toggle output
}

static inline unsigned GPIO_Read( GPIO_TypeDef *gpio, uint32_t mask ) {
       return gpio->IDR;
}
#endif

