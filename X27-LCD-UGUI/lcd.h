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

/**
 * @brief   Create a RGB color in an unsigned int
 *
 * @note    The order is reversed!!!
 *
 * @note    The 8 low order bits are 0
 */
#define RGB(R,G,B)     ( (((uint32_t) (R))<<16)         \
                        |(((uint32_t) (G))<<8)          \
                        |(((uint32_t) (B))<<0) )
#define RGB565(R,G,B)  ( (((uint16_t)((R)&0x1F))<<8)    \
                        |(((uint16_t)((G)&0x3F))<<3)    \
                        |(((uint16_t)((B)&0x1F))>>3) )
#define RGB555(R,G,B)  ( (((uint16_t)((R)&0x1F))<<7)    \
                        |(((uint16_t)((G)&0x1F))<<3)    \
                        |(((uint16_t)((B)&0x1F))>>3) )
#define RGBA(R,G,B)    ( (((uint32_t) (A))<<24)         \
                        |(((uint32_t) (R))<<16)         \
                        |(((uint32_t) (G))<<8)          \
                        |(((uint32_t) (B))<<0) )

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


void  LCD_Init(void);

void  LCD_TurnBacklightOn(void);
void  LCD_TurnBacklightOff(void);
void  LCD_PutDisplayOperation(void);
void  LCD_PutDisplayStandBy(void);
void  LCD_EnableController(void);
void  LCD_DisableController(void);

void  LCD_SetBackgroundColor( uint32_t bg );
void  LCD_SetColorKey(int layer,  uint32_t c );

/**
 * @brief Layer management
 *
 *
 * @note  There are two types of framebuffers:
 *
 *        * Full size:    Only the format must be specified
 *        * Partial size: Format, position and size must be specified
 */
void  LCD_SetFullSizeFrameBuffer(int layer, void *area, int format);
void  LCD_SetFrameBuffer(int layer, void *a, int f, int w, int h, int p, int hp, int vp );
void  LCD_SetDefaultColor(int layer, uint32_t c );
void  LCD_SetDefaultColor(int layer, uint32_t c );
void  LCD_FillFrameBuffer(int layer, unsigned c );
void  LCD_EnableLayer(int layer);
void  LCD_DisableLayer(int layer);
void  LCD_SwapLayers(void);
void  LCD_ReloadLayerImmediately(int layer);
void  LCD_ReloadLayerByVerticalBlanking(int layer);
void  LCD_SetLayerPosition(int layer, int hp, int vp);
void  LCD_SetLayerOpacity(int layer, int opacity);

void *LCD_GetLineAddress(int layer, int line);
int   LCD_GetHeight(int layer);
int   LCD_GetWidth(int layer);
int   LCD_GetPitch(int layer);
void *LCD_GetFrameBufferAddress(int layer);
void  LCD_SetFormat(int layer, int format);
int   LCD_GetFormat(int layer);
int   LCD_GetPixelSize(int layer);

int   LCD_GetMinimalFullFrameBufferSize(int format);

void LCD_DrawHorizontalLine(int layer, int x, int y, int size, unsigned color);
void LCD_DrawVerticalLine(int layer, int x, int y, int size, unsigned color);
void LCD_DrawBox(int layer, int x, int y, int sw, int sh, unsigned color, unsigned bordercolor);
void LCD_DrawLine(int layer, int x, int y, int sw, int sh, unsigned color);
#endif

