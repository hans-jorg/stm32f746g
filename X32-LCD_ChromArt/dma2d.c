/**
 * @file    dma2d.c
 *
 * @date    11/04/2021
 * @author  Hans
 *
 * @brief   DMA2D (also called Chrome-Art Accelerator) is a specialized DMS unit than can:
 *          1.  Fill a part or the whole of an image with a specific color
 *          2.  Copy part or the whole of an image into a specific part of another image
 *          3   Identical to the former but doing a pixel format conversion
 *          4   Blend a part of an image into a destination image doing a pixel format conversion
 *          5   Blend two images and copy into a destination image doing a pixel format conversion
 *
 * @brief   It can use a LUT (Look-Up Table)
 *
 * @brief   Pixel Format Conversion accepts inputs in ARGB8888, RGB888, RGB565, ARGB1555, ARGB4444,
 *          L8, AL44, AL88, L4, A8 and A4 format and converts to outputs in ARGB8888, RGB888,
 *          RGB565, ARGB1555 and ARGB4444 format
 */


#include "stm32f746xx.h"
#include "system_stm32f746.h"

#include "dma2d.h"

/**
 * @brief   structure to hold parameters as used by DMA2D unit
  *
 * @note
 */

typedef struct {
    unsigned        area;               ///< Address of first byte of 1st line
    unsigned        w;                  ///< Width
    unsigned        h;                  ///< Height
    unsigned        offset;             ///< Offset in bytes to start of next line
    unsigned        pixelformat;        ///< Pixel format
} Params;


/**
 * @brief Size in bits and in bytes of a pixel
 */
///@{
static unsigned char pixelsizebits[] = {
/*      0       1        2          3          4      5      6      7    8    9   10 */
/* ARGB8888  RGB888   RGB565   ARGB1555   ARGB4444   L8   AL44   AL88   L4   A8   A4 */
/*    I/O     1/O......I/O        I/O        I/O      I      I      I    I    I    I */
       32,     24,      16,        16,        16,     8,     8,    16,   4,   8,   4
};
static unsigned char pixelsize[] = {
/*      0       1        2          3          4      5      6      7    8    9   10 */
/* ARGB8888  RGB888   RGB565   ARGB1555   ARGB4444   L8   AL44   AL88   L4   A8   A4 */
/*    I/O     1/O......I/O        I/O        I/O      I      I      I    I    I    I */
        4,      3,       2,         2,         2,     1,     1,     2,   1,   1,   1
};
///@}


/**
 * @brief   <Function to ....>
 *
 * @note    <bla bla bla>
 */
static int
calcParamsFromRegion(const DMA2DRegion *r, Params *p) {
unsigned ps = pixelsize[r->pixelformat];

    p->pixelformat = r->pixelformat;
    p->area = (unsigned) (r->address) + r->x*ps;
    p->w    = r->w*ps;
    p->h    = r->h;
    p->offset= r->linesize - p->w;

    return 0;
}


/**
 * @brief   DMA2D_Init
 *
 * @note    Initializes de DMA2D (ChromeArt Accelerator) unit
 */
int DMA2D_Init(void) {

    /* Enable clock for DMA2D unit */
    RCC->AHB1ENR |= RCC_AHB1ENR_DMA2DEN;

    return 0;
}


/**
 * @brief   DMA2D_IsReady
 *
 * @note    Test if ongoing operation is done and unit is ready to accept new ones
 */
int DMA2D_IsReady(void) {

    return !(DMA2D->CR & DMA2D_CR_START);
}


/**
 * @brief   DMA2D_Abort
 *
 * @note    Abort on going operation
 */
int DMA2D_Abort(void) {

    DMA2D->CR |= DMA2D_CR_SUSP;

    DMA2D->CR |= DMA2D_CR_ABORT;

    return 1;
}


/**
 * @brief   DMA2D_Suspend
 *
 * @note    Suspend the going operation
 */
int DMA2D_Suspend(void) {

    DMA2D->CR |= DMA2D_CR_SUSP;

    return 1;
}


/**
 * @brief   DMA2D_Resume
 *
 * @note    Abort on going operation
 */
int DMA2D_Resume(void) {

    DMA2D->CR &= ~DMA2D_CR_SUSP;

    return 1;
}


/**
 * @brief   DMA2D_FillRegion
 *
 * @note    Fill specified region with color c
 */
int DMA2D_FillRegion( const DMA2DRegion *r, unsigned c ) {
Params p;

    /* Wait until previus operation is done and unit is ready to accept a new one */
    while( !DMA2D_IsReady() ) {}

    /* Set register to memory mode*/
    DMA2D->CR = DMA2D_CR_MODE_0;

    /* Set color source */
    DMA2D->OCOLR = c;

    /* Calculate parameters for configuring DMA2D */
    calcParamsFromRegion(r,&p);

    /* Set color format */
    DMA2D->OPFCCR = p.pixelformat;

    /* Destination address */
    DMA2D->OMAR = p.area;

    /* Set pixel per line and number of lines */
    DMA2D->NLR = (p.w<<DMA2D_NLR_PL_Pos)|(p.h<<DMA2D_NLR_NL_Pos);

    /* Offset to next start of line */
    DMA2D->OOR = p.offset;

    /* Start operation */
    DMA2D->CR |= DMA2D_CR_START;


    return 0;
}

