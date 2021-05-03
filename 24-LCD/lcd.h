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


/*
 * @brief   LCD Set Pixel Format
 */
#define LCD_FORMAT_ARGB8888         (0)
#define LCD_FORMAT_RGB888           (1)
#define LCD_FORMAT_RGB565           (2)
#define LCD_FORMAT_ARGB1555         (3)
#define LCD_FORMAT_ARGB4444         (4)
#define LCD_FORMAT_L8               (5)
#define LCD_FORMAT_AL44             (6)
#define LCD_FORMAT_AL88             (7)

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


void  LCD_BacklightOn(void);
void  LCD_BacklightOff(void);
void  LCD_NormalOperation(void);
void  LCD_StandBy(void);
void  LCD_Enable(void);
void  LCD_Disable(void);


void  LCD_Init(void);

void  LCD_SetBackgroundColor( uint32_t bg );
void  LCD_SetDefaultColor(int layer,  uint32_t c );
void  LCD_SetColorKey(int layer,  uint32_t c );

/**
 * @brief Framebuffer management
 *
 *
 * @note  There are two types of framebuffers:
 *
 *        * Full size:    Only the format must be specified
 *        * Partial size: Format, position and size must be specified
 */
void  LCD_SetFullSizeFrameBuffer(int layer, void *area, int format);
void *LCD_GetLineAddress(int layer, int line);
int   LCD_GetHeight(int layer);
int   LCD_GetWidth(int layer);
int   LCD_GetPitch(int layer);

void *LCD_GetFrameBufferAddress(int layer);
void  LCD_SetFormat(int layer, int format);
int   LCD_GetFormat(int layer);
int   LCD_GetPixelSize(int layer);

int   LCD_GetMinimalFullFrameBufferSize(int format);
void  LCD_FillFrameBuffer(int layer, unsigned c );
//void  LCD_SetFrameBuffer(int layer, int format, void *area, int w, int h );
//int   LCD_GetMinimalFrameBufferSize(int format);
#endif

