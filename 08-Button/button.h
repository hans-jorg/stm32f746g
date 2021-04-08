#ifndef BUTTON_H
#define BUTTON_H
/**
 * @file    Button.h
 *
 * @date    23/11/2020
 * @author  Hans
 */

#include "stm32f746xx.h"
#include "system_stm32f746.h"

#include "gpio.h"

/**
 * @brief   Button symbols
 *
 * @note    It is at pin 11 of Port I.
 *
 * @note    Not documented. See schematics
 *
 */

///@{
#define BUTTONPIN              (11)
#define BUTTONGPIO             GPIOI
#define BUTTONMASK             (1U<<(BUTTONPIN))
///@}


static inline void Button_Init(void) {
    GPIO_Init(BUTTONGPIO,BUTTONMASK,0);
}

static inline uint32_t Button_Read(void) {
    return GPIO_Read(BUTTONGPIO)&BUTTONMASK;
}

 #endif // BUTTON_H
