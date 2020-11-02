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

extern RGB_t LCD_Frame1[LCD_DW][LCD_DH];
#ifdef LCD_USEFRAME2
extern RGB_t LCD_Frame2[LCD_DW][LCD_DH];
#endif

void LCD_FillFrameBuffer( RGB_t (*frame)[LCD_DH], RGB_t v );


#endif

