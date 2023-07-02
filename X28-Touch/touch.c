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
 * @brief  Short Description of Function
 *
 * @param  Description of parameter
 *
 * @return Description of return parameters
 */



int
Touch_Init(void) {

    FTXXXX_Init();

    return 0;
}



/**
 * @brief Return touch info
 *
 * @param touchinfo a pointer to an array of touch info
 * @return the number of touches
 */
int
Touch_ReadTouchInfo(Touch_Info *touchinfo) {
unsigned char packet;


}
