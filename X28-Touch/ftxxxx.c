/**
 * @file ftxxxx.c
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
#include "i2c-master.h"
#include "ftxxxx.h"


/**
 * @brief Touch info from FT5336 (see FT5x06_register.pdf)
 *
 * @note  The device manages up to 10(?) simultaneous touch point. The number of
 *        points is returned by accessing register STATUS (0x03).
 *
 * @note  For each point, it returns:
 *
 *        | Byte | Field   | Size | Description                      |
 *        |------|---------|------|_---------------------------------|
 *        |   0  | Event   |   2  | Type of event                    |
 *        |   0  | Not used|   2  | Not used                         |
 *        |  0-1 | X       |  12  | X coordinate of the touch point  |
 *        |   ?  | ID      |   2  | Id                               |
 *        |   2  | Not used|   4  | Not used                         |
 *        |  2-3 | Y       |  12  | Y coordinate of the touch point  |
 *        |   4  | Weight  |   8  | Touch weight                     |
 *        |   5  | Not used|   4  | Not used                         |
 *        |   5  | Area    |   4  | Area                             |
 *
 * The event flag is encoded as:
 *        * 00: Put down
 *        * 01: Put up
 *        * 10: Contact
 *        * 11: Reserved
 *
 * The touch info is accessed thru registers:
 *
 *          | Register | Touch  |
 *          |----------|--------|
 *          | 03H-08H  |   #1   |
 *          | 09H-0EH  |   #2   |
 *          | 0FH-14H  |   #3   |
 *          | 15H-1AH  |   #4   |
 *          | 1BH-20H  |   #5   |
 *          | 1BH-20H  |   #6   |
 *          | 1BH-20H  |   #7   |
 *          | 1BH-20H  |   #8   |
 *          | 1BH-20H  |   #9   |
 *          | 1BH-20H  |  #10   |
 *
 * @note  Sample code to read touch data:
 *
 *       i2c_start();
 *       i2c_tx(0x70);              //Slave Address (Write)
 *       i2c_tx(0x00);              //Start reading address
 *       i2c_stop();
 *       i2c_start();
 *       i2c_tx(0x71);              //Slave Address (Read)
 *       for(i=0x00;i<0x1F;i++)
 *          {touchdata_buffer[i] = i2c_rx(1);}
 *       i2c_stop();
 *
 * Sample code to overwrite default register values:
 *       i2c_start();
 *       i2c_tx(0x70);
 *       i2c_tx(0xA4);
 *       i2c_tx(0x01);
 *       i2c_stop()
 *
 *  @note Register map (From NHD-4.3-480272EF-ATXL#-CTP)
 *        Similar but not equal to FT5x06.pdf) because it has weight and area
 *        info. It is possible that the two high order bit of the YH field
 *        contains additional information. It is also possible that a 11
 *
 *
 *  |  #  | -  | Symbol            | Bits| Description                        |
 *  |-----|----|-------------------|-----|------------------------------------|
 *  | 01h | RO | Touch_ID          | 7:0 |                                    |
 *  | 02h | RO | Touch Points      | 7:0 | # of touch points                  |
 *  | 03h | RO | TOUCH1_Event_Flag | 7:6 |                                    |
 *  | 03h | RO | TOUCH1_XH         | 3:0 | Upper 4 bits of X touch coordinate |
 *  | 04h | RO | TOUCH1_XL         | 7:0 | Lower 8 bits of X touch coordinate |
 *  | 05h | RO | TOUCH1_YH         | 3:0 | Upper 4 bits of Y touch coordinate |
 *  | 06h | RO | TOUCH1_YL         | 7:0 | Lower 8 bits of Y touch coordinate |
 *  | 07h | RO | TOUCH1_Weight     | 7:0 | Touch Weight                       |
 *  | 08h | RO | TOUCH1_Misc       | 3:0 | Touch Area                         |
 *  | 09h | RO | TOUCH2_Event_Flag | 7:6 |                                    |
 *  | 09h | RO | TOUCH1_XH         | 3:0 | Upper 4 bits of X touch coordinate |
 *  | 0Ah | RO | TOUCH2_XL         | 7:0 | Lower 8 bits of X touch coordinate |
 *  | 0Bh | RO | TOUCH2_YH         | 3:0 | Upper 4 bits of Y touch coordinate |
 *  | 0Ch | RO | TOUCH2_YL         | 7:0 | Lower 8 bits of Y touch coordinate |
 *  | 0Dh | RO | TOUCH2_Weight     | 7:0 | Touch Weight                       |
 *  | 0Eh | RO | TOUCH2_Misc       | 3:0 | Touch Area                         |
 *  | 0Fh | RO | TOUCH3_Event_Flag | 7:6 |                                    |
 *  | 0Fh | RO | TOUCH3_XH         | 3:0 | Upper 4 bits of X touch coordinate |
 *  | 10h | RO | TOUCH3_XL         | 7:0 | Lower 8 bits of X touch coordinate |
 *  | 11h | RO | TOUCH3_YH         | 3:0 | Upper 4 bits of Y touch coordinate |
 *  | 12h | RO | TOUCH3_YL         | 7:0 | Lower 8 bits of Y touch coordinate |
 *  | 13h | RO | TOUCH3_Weight     | 7:0 | Touch Weight                       |
 *  | 14h | RO | TOUCH3_Misc       | 3:0 | Touch Area                         |
 *  | 15h | RO | TOUCH4_Event_Flag | 7:6 |                                    |
 *  | 15h | RO | TOUCH4_XH         | 3:0 | Upper 4 bits of X touch coordinate |
 *  | 16h | RO | TOUCH4_XL         | 7:0 | Lower 8 bits of X touch coordinate |
 *  | 17h | RO | TOUCH4_YH         | 3:0 | Upper 4 bits of Y touch coordinate |
 *  | 18h | RO | TOUCH4_YL         | 7:0 | Lower 8 bits of Y touch coordinate |
 *  | 1Ah | RO | TOUCH4_Misc       | 3:0 | Touch Area                         |
 *  | 1Bh | RO | TOUCH5_Event_Flag | 7:6 |                                    |
 *  | 1Bh | RO | TOUCH5_XH         | 3:0 | Upper 4 bits of X touch coordinate |
 *  | 1Ch | RO | TOUCH5_XL         | 7:0 | Lower 8 bits of X touch coordinate |
 *  | 1Dh | RO | TOUCH5_YH         | 3:0 | Upper 4 bits of Y touch coordinate |
 *  | 1Eh | RO | TOUCH5_YL         | 7:0 | Lower 8 bits of Y touch coordinate |
 *  | 1Fh | RO | TOUCH5_Weight     | 7:0 | Touch Weight                       |
 *  | 20h | RO | TOUCH5_Misc       | 3:0 | Touch Area                         |
 *  | 21h | RO | TOUCH6_Event_Flag | 7:6 |                                    |
 *  | 21h | RO | TOUCH6_XH         | 3:0 | Upper 4 bits of X touch coordinate |
 *  | 22h | RO | TOUCH6_XL         | 7:0 | Lower 8 bits of X touch coordinate |
 *  | 23h | RO | TOUCH6_YH         | 3:0 | Upper 4 bits of Y touch coordinate |
 *  | 24h | RO | TOUCH6_YL         | 7:0 | Lower 8 bits of Y touch coordinate |
 *  | 25h | RO | TOUCH6_Weight     | 7:0 | Touch Weight                       |
 *  | 26h | RO | TOUCH6_Misc       | 3:0 | Touch Area                         |
 *  | 27h | RO | TOUCH7_Event_Flag | 7:6 |                                    |
 *  | 27h | RO | TOUCH7_XH         | 3:0 | Upper 4 bits of X touch coordinate |
 *  | 28h | RO | TOUCH7_XL         | 7:0 | Lower 8 bits of X touch coordinate |
 *  | 29h | RO | TOUCH7_YH         | 3:0 | Upper 4 bits of Y touch coordinate |
 *  | 2Ah | RO | TOUCH7_YL         | 7:0 | Lower 8 bits of Y touch coordinate |
 *  | 2Bh | RO | TOUCH7_Weight     | 7:0 | Touch Weight                       |
 *  | 2Ch | RO | TOUCH7_Misc       | 3:0 | Touch Area                         |
 *  | 2Dh | RO | TOUCH8_Event_Flag | 7:6 |                                    |
 *  | 2Dh | RO | TOUCH8_XH         | 3:0 | Upper 4 bits of X touch coordinate |
 *  | 2Eh | RO | TOUCH8_XL         | 7:0 | Lower 8 bits of X touch coordinate |
 *  | 2Fh | RO | TOUCH8_YH         | 3:0 | Upper 4 bits of Y touch coordinate |
 *  | 30h | RO | TOUCH8_YL         | 7:0 | Lower 8 bits of Y touch coordinate |
 *  | 31h | RO | TOUCH8_Weight     | 7:0 | Touch Weight                       |
 *  | 32h | RO | TOUCH8_Misc       | 3:0 | Touch Area                         |
 *  | 33h | RO | TOUCH9_Event_Flag | 7:6 |                                    |
 *  | 33h | RO | TOUCH9_XH         | 3:0 | Upper 4 bits of X touch coordinate |
 *  | 34h | RO | TOUCH9_XL         | 7:0 | Lower 8 bits of X touch coordinate |
 *  | 35h | RO | TOUCH9_YH         | 3:0 | Upper 4 bits of Y touch coordinate |
 *  | 36h | RO | TOUCH9_YL         | 7:0 | Lower 8 bits of Y touch coordinate |
 *  | 37h | RO | TOUCH9_Weight     | 7:0 | Touch Weight                       |
 *  | 38h | RO | TOUCH9_Misc       | 3:0 | Touch Area                         |
 *  | 39h | RO | TOUCH10_Event_Flag| 7:6 |                                    |
 *  | 39h | RO | TOUCH10_XH        | 3:0 | Upper 4 bits of X touch coordinate |
 *  | 3Ah | RO | TOUCH10_XL        | 7:0 | Lower 8 bits of X touch coordinate |
 *  | 3Bh | RO | TOUCH10_YH        | 3:0 | Upper 4 bits of Y touch coordinate |
 *  | 3Ch | RO | TOUCH10_YL        | 7:0 | Lower 8 bits of Y touch coordinate |
 *  | 3Dh | RO | TOUCH10_Weight    | 7:0 | Touch Weight                       |
 *  | 3Eh | RO | TOUCH10_Misc      | 3:0 | Touch Area                         |
 *  | 80h | RW | ID_G_MC_THGROUP   | 7:0 | Threshold (default=4Bh)            |
 *  | 81h | RW | ID_G_MC_THPEAK    | 7:0 | Peak threshold (default=46h)       |
 *  | 85h | RW | ID_G_THDIFF       | 7:0 | Filtering threshold (default=A0h)  |
 *  | 86h | RW | ID_G_CTRL         | 1:0 | Enable to enter monitor mode       |
 *  | 88h | RW | ID_G_PERIODACTIVE | 3:0 | Period of Active Status            |
 *  | 89h | RW | ID_G_PERIODMONITOR| 7:0 | Time to idle (ms)                  |
 *  | A1h | RO | ID_G_LIB_VERSION_H| 7:0 | Library version (High byte)        |
 *  | A2h | RO | ID_G_LIB_VERSION_L| 7:0 | Library version (Low byte)         |
 *  | A3h | RO | ID_G_CHIPER_HIGH  | 7:0 | Chip vendor ID (=54h ??)           |
 *  | A4h | RW | ID_G_MODE         | 0:0 | Trigger/Polling mode               |
 *  | A5h | RW | ID_G_PMODE        | 1:0 | Active/Monitor/Sleep mode          |
 *  | A6h | RO | ID_G_FIRMID       | 7:0 | Firmware ID # (default=2??)        |
 *  | A8h | RO | ID_G_VENODRID     | 7:0 | CTPM Vendor chip ID (default=79h??)|
 *  | C0h | RW | ID_G_GLOVE_MODE_EN| 0:0 | Glove mode disable/enable          |
 *  | C1h | RW | ID_G_COVER_MODE_EN| 0:0 | Cover mode disable/enable          |
 *
 *
 *  | Gesture ID field    |   Description                            |
 *  |---------------------|------------------------------------------|
 *  |    10h              |  Swipe Up                                |
 *  |    18h              |  Swipe Down                              |
 *  |    1Ch              |  Swipe Left                              |
 *  |    14h              |  Swipe Right                             |
 *  |    48h              |  Zoom Out                                |
 *  |    49h              |  Zoom In                                 |
 *  |    00h              |  No gesture                              |
 *
 *
 *  | Event flag field    |  Description                             |
 *  |---------------------|------------------------------------------|
 *  |        0            |  Put Down                                |
 *  |        1            |  Put Up                                  |
 *  |        2            |  Contact                                 |
 *  |        3            |  Reserved                                |
 *
 * > X and Y field have 12 bits but maximal value is 1FFh, that
 * >       corresponds to 8 bits
 *
 *  @note Packet format
 *
 *  Write
 *          SAAAAAAAW*RRRRRRRR*DDDDDDDD*DDDDDDDD*.....DDDDDDDD*P
 *  Read
 *          SAAAAAAAW*RRRRRRRR*P
 *          SAAAAAAAR*DDDDDDDD*DDDDDDDD*DDDDDDDD*.....DDDDDDDD*P
 *
 *    S=Start
 *    P=Stop
 *    R=Read
 *    W=Write
 *    AAAAAAA = Slave address
 *    RRRRRRRR = First address of register to be read/written
 *    DDDDDDDD = Data
 */


/**
 * Addresses of first register of touch info
 */

static uint16_t touchaddr[] = {
    TOUCH_ADDR_TOUCH1_XH,
    TOUCH_ADDR_TOUCH2_XH,
    TOUCH_ADDR_TOUCH3_XH,
    TOUCH_ADDR_TOUCH4_XH,
    TOUCH_ADDR_TOUCH5_XH,
};

static uint16_t touchmax = sizeof(touchaddr)/sizeof(uint16_t);

#define I2C_INTERFACE   I2C3
#define I2C_ADDRESS     0x70
#define XMAX            511
#define YMAX            511


static uint16_t width,height;

/**
 * @brief  Write data to Touch register
 *
 * @param  reg:  register to be written
 *         data: data to be written
 *
 * @return 0: no errors
 */

int Touch_WriteRegister(uint8_t reg, uint8_t data) {
uint8_t v[2];
int rc;

    v[0] = reg;
    v[1] = data;
    rc = I2CMaster_Write( I2C_INTERFACE, I2C_ADDRESS, v, 2);

    return 0;
}


/**
 * @brief  Short Description of Function
 *
 * @param  Description of parameter
 *
 * @return Description of return parameters
 */
int Touch_ReadRegister(uint8_t reg, uint8_t *pdata, int n) {

    rc = I2CMaster_Write( I2C_INTERFACE, I2C_ADDRESS, &reg, 1);
    rc = I2CMaster_Read( I2C_INTERFACE, I2C_ADDRESS, pdata, n);
    return 0;
}

/**
 * @brief  Short Description of Function
 *
 * @param  Description of parameter
 *
 * @return Description of return parameters
 */
int Touch_WriteSequentialRegisters( uint8_t startreg, uint8_t *pdata, int n) {

    rc = I2CMaster_Write( I2C_INTERFACE, I2C_ADDRESS, &startreg, 1);

    return 0;
}

/**
 * @brief  Short Description of Function
 *
 * @param  Description of parameter
 *
 * @return Description of return parameters
 */

int Touch_ReadSequentialRegisters( uint8_t startreg, uint8_t *pdata, int num) {

    return 0;
}


/**
 * @brief  Short Description of Function
 *
 * @param  Description of parameter
 *
 * @return Description of return parameters
 */



int
Touch_Init(uint16_t w, uint16_t h) {

    I2CMaster_Init(I2C_INTERFACE, 0, 0);

    width = w;
    height = h;

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
