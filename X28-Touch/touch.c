/**
 * @file touch.c
 *
 * @brief Touch Screen interface using a FT5336
 *
 * @note  The FT5336 in the STM32F746G Discovery Board uses an I2C interface
 *        with slave address 0x38. So the first byte must be 0x70 for a write
 *        operation or 0x71 for a read operation
 *
 *
 * @author your name (you@domain.com)
 * @version 0.1
 * @date 2021-11-16
 *
 * @copyright Copyright (c) 2021
 *
 */

#include "stm32f746xx.h"
#include "system_stm32f746.h"
#include "touch.h"
#include "i2c-master.h"
#include "ftxxxx.h"


/**
 * @brief  Touch Initialization
 *
 * @return Initialize Touch Controller
 */

int
Touch_Init(void) {
int rc;

    rc = FTXXXX_Init();

    return rc;
}

/**
 * @brief  Touch Detected
 *
 * @return Signalize that a touch is beeing detected
 */

int
Touch_Detected(void) {
int rc;

    rc = FTXXXX_ReadInterruptPinStatus() || FTXXXX_GetStatus();

    return rc;
}


/**
 * @brief Return touch info
 *
 * @param touchinfo a pointer to an array of touch info
 *
 * @return the number of touches
 */
int
Touch_ReadInfo(Touch_Info *touchinfo, int nmax) {
FTXXXX_Info buffer;
int n = 0;


    if( Touch_Detected() ) {
        n = FTXXXX_ReadTouchInfo(&buffer);
        if( n > 0 ) {
            if( n > nmax ) n = nmax;
            for(int i=0;i<n;i++) {
                touchinfo[i].id    = buffer.gesture;
                touchinfo[i].x     = buffer.points[i].x;
                touchinfo[i].y     = buffer.points[i].y;
                touchinfo[i].weight= buffer.points[i].w;
            }
        }
    }
    return n;
}
