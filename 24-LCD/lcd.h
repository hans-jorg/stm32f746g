#ifndef LCD_H
#define LCD_H
/**
 * @file    lcd.h
 *
 * @date    07/10/2020
 * @author  Hans
 */

#include "stm32f746xx.h"
#include "system_stm32f746.h"


void LCD_Init(void);

/**
 * @brief   Active area
 */
///@{
#define LCD_DW          480
#define LCD_DH          272
///@}

typedef struct {
    uint8_t     B;
    uint8_t     G;
    uint8_t     R;
} RGB_t;


void LCD_FillFrameBuffer( RGB_t *frame, RGB_t v );

#endif

