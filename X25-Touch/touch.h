#ifndef TOUCH_H
#define TOUCH_H
/**
 * @file touch.h
 *
 * @brief Touch Screen interface using a FT5336
 *
 * @note  The FT5336 in the STM32F746G Discovery Board uses an I2C interface
 *        with slave address 0x38. So the first byte must be 0x70 for a write
 *        operation or 0x71 for a read operation
 *
 * @note  There is no documentation for the FT5336 touch controller. Info about
 *        its registers are derived from the documentation of the FT5x06, FT5x16 and FT5x26
 *        documentation.
 *
 * @author Hans (hans@ele.ufes.br)
 * @version 0.1
 * @date 2023-05-28
 *
 * @copyright Copyright (c) 2023
 *
 */


typedef struct {
    uint16_t event;
    uint16_t id;
    uint16_t x;
    uint16_t y;
    uint16_t weight;
    uint16_t misc;
} Touch_Info;

int Touch_Init(uint16_t w, uint16_t h);
int Touch_Read(Touch_Info *touch);

int Touch_WriteRegister(uint8_t reg, uint8_t data);
int Touch_ReadRegister(uint8_t reg, uint8_t *pdata);
int Touch_WriteSequentialRegisters( uint8_t startreg, uint8_t *pdata, int num);
int Touch_ReadSequentialRegisters( uint8_t startreg, uint8_t *pdata, int num);

/*
 * Registers of the FT5336 controller. From the FT5x16 documentation
 */
#define TOUCH_REG_DEVICE_MODE              0x00
#define TOUCH_REG_GEST_ID                  0x01
#define TOUCH_REG_TD_STATUS                0x02
#define TOUCH_REG_TOUCH1_XH                0x03
#define TOUCH_REG_TOUCH1_XL                0x04
#define TOUCH_REG_TOUCH1_YH                0x05
#define TOUCH_REG_TOUCH1_YL                0x06
#define TOUCH_REG_TOUCH1_WEIGHT            0x07
#define TOUCH_REG_TOUCH1_MISC              0x08
#define TOUCH_REG_TOUCH2_XH                0x09
#define TOUCH_REG_TOUCH2_XL                0x0A
#define TOUCH_REG_TOUCH2_YH                0x0B
#define TOUCH_REG_TOUCH2_YL                0x0C
#define TOUCH_REG_TOUCH2_WEIGHT            0x0D
#define TOUCH_REG_TOUCH2_MISC              0x0E
#define TOUCH_REG_TOUCH3_XH                0x0F
#define TOUCH_REG_TOUCH3_XL                0x10
#define TOUCH_REG_TOUCH3_YH                0x11
#define TOUCH_REG_TOUCH3_YL                0x12
#define TOUCH_REG_TOUCH3_WEIGHT            0x13
#define TOUCH_REG_TOUCH3_MISC              0x14
#define TOUCH_REG_TOUCH4_XH                0x15
#define TOUCH_REG_TOUCH4_XL                0x16
#define TOUCH_REG_TOUCH4_YH                0x17
#define TOUCH_REG_TOUCH4_YL                0x18
#define TOUCH_REG_TOUCH4_WEIGHT            0x19
#define TOUCH_REG_TOUCH4_MISC              0x1A
#define TOUCH_REG_TOUCH5_XH                0x1B
#define TOUCH_REG_TOUCH5_XL                0x1C
#define TOUCH_REG_TOUCH5_YH                0x1E
#define TOUCH_REG_TOUCH5_YL                0x1E
#define TOUCH_REG_TOUCH5_WEIGHT            0x1F
#define TOUCH_REG_TOUCH5_MISC              0x20
#if TOUCH_10_POINTS
#define TOUCH_REG_TOUCH6_XH                0x21
#define TOUCH_REG_TOUCH6_XL                0x22
#define TOUCH_REG_TOUCH6_YH                0x23
#define TOUCH_REG_TOUCH6_YL                0x24
#define TOUCH_REG_TOUCH6_WEIGHT            0x25
#define TOUCH_REG_TOUCH6_MISC              0x26
#define TOUCH_REG_TOUCH7_XH                0x27
#define TOUCH_REG_TOUCH7_XL                0x28
#define TOUCH_REG_TOUCH7_YH                0x29
#define TOUCH_REG_TOUCH7_YL                0x2A
#define TOUCH_REG_TOUCH7_WEIGHT            0x2B
#define TOUCH_REG_TOUCH7_MISC              0x2C
#define TOUCH_REG_TOUCH8_XH                0x2E
#define TOUCH_REG_TOUCH8_XL                0x2E
#define TOUCH_REG_TOUCH8_YH                0x2F
#define TOUCH_REG_TOUCH8_YL                0x30
#define TOUCH_REG_TOUCH8_WEIGHT            0x31
#define TOUCH_REG_TOUCH8_MISC              0x32
#define TOUCH_REG_TOUCH9_XH                0x33
#define TOUCH_REG_TOUCH9_XL                0x34
#define TOUCH_REG_TOUCH9_YH                0x35
#define TOUCH_REG_TOUCH9_YL                0x36
#define TOUCH_REG_TOUCH9_WEIGHT            0x37
#define TOUCH_REG_TOUCH9_MISC              0x38
#define TOUCH_REG_TOUCH10_XH               0x39
#define TOUCH_REG_TOUCH10_XL               0x3A
#define TOUCH_REG_TOUCH10_YH               0x3B
#define TOUCH_REG_TOUCH10_YL               0x3C
#define TOUCH_REG_TOUCH10_WEIGHT           0x3D
#define TOUCH_REG_TOUCH10_MISC             0x3E
#endif
#define TOUCH_REG_THGROUP                  0x80
#define TOUCH_REG_THPEAK                   0x81
#define TOUCH_REG_THCAL                    0x82
#define TOUCH_REG_COMP_STATUS              0x83
#define TOUCH_REG_COMP_FLAG                0x84
#define TOUCH_REG_THDIFF                   0x85
#define TOUCH_REG_CTRL                     0x86
#define TOUCH_REG_TIME_MONITOR             0x87
#define TOUCH_REG_PERIODACTIVE             0x88
#define TOUCH_REG_PERIOD_MONITOR           0x89
#define TOUCH_REG_SCAN_RATE                0x8A
#define TOUCH_REG_CHARGER_RATE             0x8B
#define TOUCH_REG_SCAN_REGB                0x8C
#define TOUCH_REG_SCAN_CAP                 0x8D
#define TOUCH_REG_SCAN_FILTERMODE          0x8E
#define TOUCH_REG_SCAN_REFRESH             0x8F
#define TOUCH_REG_MOVSTH_I                 0x90
#define TOUCH_REG_MOVSTH_N                 0x91
#define TOUCH_REG_LEFT_RIGHT_OFFSET        0x92
#define TOUCH_REG_UP_DOWN_OFFSET           0x93
#define TOUCH_REG_LEFT_RIGHT_DISTANCE      0x94
#define TOUCH_REG_UP_DOWN_DISTANCE         0x95
#define TOUCH_REG_ZOOM_DIST_SQR            0x97
#define TOUCH_REG_MAX_X_H                  0x98
#define TOUCH_REG_MAX_X_L                  0x98
#define TOUCH_REG_MAX_Y_H                  0x98
#define TOUCH_REG_MAX_Y_L                  0x98
#define TOUCH_REG_AUTO_CLB_MODE            0xA0
#define TOUCH_REG_LIB_VERSION_H            0xA1
#define TOUCH_REG_LIB_VERSION_L            0xA2
#define TOUCH_REG_CHIPID                   0xA3
#define TOUCH_REG_MODE                     0xA4
#define TOUCH_REG_PMODE                    0xA5
#define TOUCH_REG_FIRMID                   0xA6
#define TOUCH_REG_STATE                    0xA7
#define TOUCH_REG_VENDORID                 0xA8
#define TOUCH_REG_ERR                      0xA9
#define TOUCH_REG_CLB                      0xAA
#define TOUCH_REG_DRAW_LINE_TH             0xAE
#define TOUCH_REG_RELEASE_CODE             0xAF
#define TOUCH_REG_FACE_DETECT_MODE         0xB0
#define TOUCH_REG_PRESIZE_EN               0xB2
#define TOUCH_REG_BIGAREA_PEAK_TH          0xB3
#define TOUCH_REG_BIGAREA_PEAK_NUM         0xB4
#define TOUCH_REG_LOG_MSG_CNT              0xFE
#define TOUCH_REG_LOG_CUR_CHA              0xFF
/* Aliases: Using Register names of the FT5x06 */
#define TOUCH_REG_THWATER                  TOUCH_REG_COMP_STATUS
#define TOUCH_REG_THTEMP                   TOUCH_REG_COMP_FLAG
#define TOUCH_REG_CIPHER                   TOUCH_REG_CHIPID
#define TOUCH_REG_FT520ID                  TOUCH_REG_VENDORID
#define TOUCH_REG_OFFSET_LR                TOUCH_REG_LEFT_RIGHT_OFFSET
#define TOUCH_REG_OFFSET_UD                TOUCH_REG_UP_DOWN_OFFSET
#define TOUCH_REG_DISTANCE_LR              TOUCH_REG_LEFT_RIGHT_DISTANCE
#define TOUCH_REG_DISTANCE_UD              TOUCH_REG_UP_DOWN_DISTANCE
#define TOUCH_REG_DISTANCE_ZOOM            TOUCH_REG_ZOOM_DIST_SQR


#endif // TOUCH_H
