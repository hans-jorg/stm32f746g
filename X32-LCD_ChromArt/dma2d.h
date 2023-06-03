#ifndef DMA2D_H
#define DMA2D_H
/**
 * @file    dma2d.h
 *
 * @date    11/04/2021
 * @author  Hans
 */


typedef struct {
    unsigned long   address;                ///< Address of 1st byte of 1st line
    unsigned        x;                      ///< Horizontal position inside the englobing region
    unsigned        y;                      ///< Vertical position inside the englobing region
    unsigned        w;                      ///< Width of region
    unsigned        h;                      ///< Height of region (Number of lines)
    unsigned        pixelformat;            ///< Pixel format used in region
    unsigned        linesize;               ///< Line size in bytes
} DMA2DRegion;

#define DECLARE_REGION(NAME,ADDR,X,Y,W,H,PF,LS)       \
    DMA2DRegion NAME = { (unsigned long ) (ADDR),     \
                         (unsigned)       (X),        \
                         (unsigned)       (Y),        \
                         (unsigned)       (W),        \
                         (unsigned)       (H),        \
                         (unsigned)       (PF),       \
                         (unsigned)       (LS)        \
                         }
/**
 * @brief   Pixel format recognized by the DMA2D
 *
 * @brief   Table 35 in section 9.3.4
 *
 * @brief   A is transparency (alpha value). 0xFF is opaque. 0 is transparent
 *
 * @brief   L is luminance (index to a LUT)
 *
 */
#define DMA2D_ARGB8888                0
#define DMA2D_RGB888                  1
#define DMA2D_RGB565                  2
#define DMA2D_ARGB1555                3
#define DMA2D_ARGB4444                4
#define DMA2D_L8                      5
#define DMA2D_AL44                    6
#define DMA2D_AL88                    7
#define DMA2D_L4                      8
#define DMA2D_A8                      9
#define DMA2D_A4                     10

int DMA2D_Init(void);
int DMA2D_IsReady(void);
int DMA2D_Abort(void);
int DMA2D_Suspend(void);
int DMA2D_Resume(void);
int DMA2D_FillRegion(const DMA2DRegion *r, unsigned c);


#endif
