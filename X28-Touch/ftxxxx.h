#ifndef FTXXXX_H
#define FTXXXX_H
/**
 * @file ftxxxx.h
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

/**
 * @brief   Default for the number of points tracked by the FT5336
 */
#ifndef FTXXXX_NPOINTS
#define FTXXXX_NPOINTS                      5
#endif

int FTXXXX_Init(void);
int FTXXXX_ReadInterruptPinStatus(void);
int FTXXXX_WriteRegister(uint8_t reg, uint8_t data);
int FTXXXX_ReadRegister(uint8_t reg, uint8_t *pdata);
int FTXXXX_WriteSequentialRegisters( uint8_t startreg, uint8_t *pdata, int num);
int FTXXXX_ReadSequentialRegisters( uint8_t startreg, uint8_t *pdata, int num);
void FTXXXX_ProcessInterrupt(void);


typedef struct {
    uint16_t    x;      // X-pos
    uint16_t    y;      // Y-pos
} FTXXXX_Info;

/*
 * Registers of the FT5336 controller. From the FT5x16 documentation
 */
#define FTXXXX_REG_DEVICE_MODE              0x00
#define FTXXXX_REG_GEST_ID                  0x01
#define FTXXXX_REG_TD_STATUS                0x02
#define FTXXXX_REG_TOUCH1_XH                0x03
#define FTXXXX_REG_TOUCH1_XL                0x04
#define FTXXXX_REG_TOUCH1_YH                0x05
#define FTXXXX_REG_TOUCH1_YL                0x06
#define FTXXXX_REG_TOUCH1_WEIGHT            0x07
#define FTXXXX_REG_TOUCH1_MISC              0x08
#define FTXXXX_REG_TOUCH2_XH                0x09
#define FTXXXX_REG_TOUCH2_XL                0x0A
#define FTXXXX_REG_TOUCH2_YH                0x0B
#define FTXXXX_REG_TOUCH2_YL                0x0C
#define FTXXXX_REG_TOUCH2_WEIGHT            0x0D
#define FTXXXX_REG_TOUCH2_MISC              0x0E
#define FTXXXX_REG_TOUCH3_XH                0x0F
#define FTXXXX_REG_TOUCH3_XL                0x10
#define FTXXXX_REG_TOUCH3_YH                0x11
#define FTXXXX_REG_TOUCH3_YL                0x12
#define FTXXXX_REG_TOUCH3_WEIGHT            0x13
#define FTXXXX_REG_TOUCH3_MISC              0x14
#define FTXXXX_REG_TOUCH4_XH                0x15
#define FTXXXX_REG_TOUCH4_XL                0x16
#define FTXXXX_REG_TOUCH4_YH                0x17
#define FTXXXX_REG_TOUCH4_YL                0x18
#define FTXXXX_REG_TOUCH4_WEIGHT            0x19
#define FTXXXX_REG_TOUCH4_MISC              0x1A
#define FTXXXX_REG_TOUCH5_XH                0x1B
#define FTXXXX_REG_TOUCH5_XL                0x1C
#define FTXXXX_REG_TOUCH5_YH                0x1E
#define FTXXXX_REG_TOUCH5_YL                0x1E
#define FTXXXX_REG_TOUCH5_WEIGHT            0x1F
#define FTXXXX_REG_TOUCH5_MISC              0x20
#if FTXXXX_NPOINTS == 10
#define FTXXXX_REG_TOUCH6_XH                0x21
#define FTXXXX_REG_TOUCH6_XL                0x22
#define FTXXXX_REG_TOUCH6_YH                0x23
#define FTXXXX_REG_TOUCH6_YL                0x24
#define FTXXXX_REG_TOUCH6_WEIGHT            0x25
#define FTXXXX_REG_TOUCH6_MISC              0x26
#define FTXXXX_REG_TOUCH7_XH                0x27
#define FTXXXX_REG_TOUCH7_XL                0x28
#define FTXXXX_REG_TOUCH7_YH                0x29
#define FTXXXX_REG_TOUCH7_YL                0x2A
#define FTXXXX_REG_TOUCH7_WEIGHT            0x2B
#define FTXXXX_REG_TOUCH7_MISC              0x2C
#define FTXXXX_REG_TOUCH8_XH                0x2E
#define FTXXXX_REG_TOUCH8_XL                0x2E
#define FTXXXX_REG_TOUCH8_YH                0x2F
#define FTXXXX_REG_TOUCH8_YL                0x30
#define FTXXXX_REG_TOUCH8_WEIGHT            0x31
#define FTXXXX_REG_TOUCH8_MISC              0x32
#define FTXXXX_REG_TOUCH9_XH                0x33
#define FTXXXX_REG_TOUCH9_XL                0x34
#define FTXXXX_REG_TOUCH9_YH                0x35
#define FTXXXX_REG_TOUCH9_YL                0x36
#define FTXXXX_REG_TOUCH9_WEIGHT            0x37
#define FTXXXX_REG_TOUCH9_MISC              0x38
#define FTXXXX_REG_TOUCH10_XH               0x39
#define FTXXXX_REG_TOUCH10_XL               0x3A
#define FTXXXX_REG_TOUCH10_YH               0x3B
#define FTXXXX_REG_TOUCH10_YL               0x3C
#define FTXXXX_REG_TOUCH10_WEIGHT           0x3D
#define FTXXXX_REG_TOUCH10_MISC             0x3E
#endif
#define FTXXXX_REG_THGROUP                  0x80
#define FTXXXX_REG_THPEAK                   0x81
#define FTXXXX_REG_THCAL                    0x82
#define FTXXXX_REG_COMP_STATUS              0x83
#define FTXXXX_REG_COMP_FLAG                0x84
#define FTXXXX_REG_THDIFF                   0x85
#define FTXXXX_REG_CTRL                     0x86
#define FTXXXX_REG_TIME_MONITOR             0x87
#define FTXXXX_REG_PERIODACTIVE             0x88
#define FTXXXX_REG_PERIOD_MONITOR           0x89
#define FTXXXX_REG_SCAN_RATE                0x8A
#define FTXXXX_REG_CHARGER_RATE             0x8B
#define FTXXXX_REG_SCAN_REGB                0x8C
#define FTXXXX_REG_SCAN_CAP                 0x8D
#define FTXXXX_REG_SCAN_FILTERMODE          0x8E
#define FTXXXX_REG_SCAN_REFRESH             0x8F
#define FTXXXX_REG_MOVSTH_I                 0x90
#define FTXXXX_REG_MOVSTH_N                 0x91
#define FTXXXX_REG_LEFT_RIGHT_OFFSET        0x92
#define FTXXXX_REG_UP_DOWN_OFFSET           0x93
#define FTXXXX_REG_LEFT_RIGHT_DISTANCE      0x94
#define FTXXXX_REG_UP_DOWN_DISTANCE         0x95
#define FTXXXX_REG_ZOOM_DIST_SQR            0x97
#define FTXXXX_REG_MAX_X_H                  0x98
#define FTXXXX_REG_MAX_X_L                  0x98
#define FTXXXX_REG_MAX_Y_H                  0x98
#define FTXXXX_REG_MAX_Y_L                  0x98
#define FTXXXX_REG_AUTO_CLB_MODE            0xA0
#define FTXXXX_REG_LIB_VERSION_H            0xA1
#define FTXXXX_REG_LIB_VERSION_L            0xA2
#define FTXXXX_REG_CHIPID                   0xA3
#define FTXXXX_REG_MODE                     0xA4
#define FTXXXX_REG_PMODE                    0xA5
#define FTXXXX_REG_FIRMID                   0xA6
#define FTXXXX_REG_STATE                    0xA7
#define FTXXXX_REG_VENDORID                 0xA8
#define FTXXXX_REG_ERR                      0xA9
#define FTXXXX_REG_CLB                      0xAA
#define FTXXXX_REG_DRAW_LINE_TH             0xAE
#define FTXXXX_REG_RELEASE_CODE             0xAF
#define FTXXXX_REG_FACE_DETECT_MODE         0xB0
#define FTXXXX_REG_PRESIZE_EN               0xB2
#define FTXXXX_REG_BIGAREA_PEAK_TH          0xB3
#define FTXXXX_REG_BIGAREA_PEAK_NUM         0xB4
#define FTXXXX_REG_LOG_MSG_CNT              0xFE
#define FTXXXX_REG_LOG_CUR_CHA              0xFF
/* Aliases: Using Register names of the FT5x06 */
#define FTXXXX_REG_THWATER                  FTXXXX_REG_COMP_STATUS
#define FTXXXX_REG_THTEMP                   FTXXXX_REG_COMP_FLAG
#define FTXXXX_REG_CIPHER                   FTXXXX_REG_CHIPID
#define FTXXXX_REG_FT520ID                  FTXXXX_REG_VENDORID
#define FTXXXX_REG_OFFSET_LR                FTXXXX_REG_LEFT_RIGHT_OFFSET
#define FTXXXX_REG_OFFSET_UD                FTXXXX_REG_UP_DOWN_OFFSET
#define FTXXXX_REG_DISTANCE_LR              FTXXXX_REG_LEFT_RIGHT_DISTANCE
#define FTXXXX_REG_DISTANCE_UD              FTXXXX_REG_UP_DOWN_DISTANCE
#define FTXXXX_REG_DISTANCE_ZOOM            FTXXXX_REG_ZOOM_DIST_SQR

/**
 * @note  Register fields for the FT5336
 *
 * @note  Not all registers are listed here
 *
 * @note  There register are used in normal mode. In other modes, there are different registers
 *        using the same adresses.
 */

/** Device Mode Register Fields */
#define FTXXXX_DEVICE_MODE_ID_MASK          0x70
#define FTXXXX_DEVICE_MODE_ID_SHIFT            4
#define FTXXXX_DEVICE_MODE_ID_NORMAL        0x00
#define FTXXXX_DEVICE_MODE_ID_SYSTEMINFO    0x10
#define FTXXXX_DEVICE_MODE_ID_TEST_0        0x40
#define FTXXXX_DEVICE_MODE_ID_TEST_1        0x60
/** Gesture ID Register Fields */
#define FTXXXX_GEST_ID_ID_MASK              0xFF
#define FTXXXX_GEST_ID_ID_SHIFT                0
#define FTXXXX_GEST_ID_ID_NO                0x00
#define FTXXXX_GEST_ID_ID_ZOOM_OUT          0x49
#define FTXXXX_GEST_ID_ID_ZOOM_IN           0x48
#define FTXXXX_GEST_ID_ID_MOVE_RIGHT        0x1C
#define FTXXXX_GEST_ID_ID_MOVE_LEFT         0x14
#define FTXXXX_GEST_ID_ID_MOVE_DOWN         0x18
#define FTXXXX_GEST_ID_ID_MOVE_UP           0x10
/** Touch Status Register : Number of touches */
#define FTXXXX_TD_STATUS_NUM_MASK           0x0F
#define FTXXXX_TD_STATUS_NUM_SHIFT             0
/**
 * Touch Data Info Register fields
 *
 * They can be read as a sequence: XH, Xl, YH, YL, WEIGHT, MISC
 */
#define FTXXXX_TDx_XH_POS_MASK              0x0F
#define FTXXXX_TDx_XH_POS_SHIFT                0
#define FTXXXX_TDx_XH_EVENT_MASK            0xC0
#define FTXXXX_TDx_XH_EVENT_SHIFT              6
#define FTXXXX_TDx_XH_EVENT_NO              0xC0
#define FTXXXX_TDx_XH_EVENT_DOWN            0x00
#define FTXXXX_TDx_XH_EVENT_LIFT_UP         0x40
#define FTXXXX_TDx_XH_EVENT_CONTACT         0x80
#define FTXXXX_TDx_XL_POS_MASK              0xFF
#define FTXXXX_TDx_XL_POS_SHIFT                0
#define FTXXXX_TDx_YH_ID_MASK               0xF0
#define FTXXXX_TDx_YH_ID_SHIFT                 4
#define FTXXXX_TDx_YL_POS_MASK              0xFF
#define FTXXXX_TDx_YL_POS_SHIFT                0
#define FTXXXX_TDx_WEIGHT_VAL_MASK          0xFF
#define FTXXXX_TDx_WEIGHT_VAL_SHIFT            0
#define FTXXXX_TDx_MISC_AREA_MASK           0xF0
#define FTXXXX_TDx_MISC_AREA_SHIFT             4
#define FTXXXX_TDx_MISC_DIR_MASK            0x0C
#define FTXXXX_TDx_MISC_DIR_SHIFT              2
#define FTXXXX_TDx_MISC_DIR_UP              0x00
#define FTXXXX_TDx_MISC_DIR_DOWN            0x40
#define FTXXXX_TDx_MISC_DIR_LEFT            0x80
#define FTXXXX_TDx_MISC_DIR_RIGHT           0xC0
#define FTXXXX_TDx_MISC_SPEED_MASK          0x03
#define FTXXXX_TDx_MISC_SPEED_SHIFT            0
#define FTXXXX_TDx_MISC_SPEED_STATIC        0x00
#define FTXXXX_TDx_MISC_SPEED_NORMAL        0x01
#define FTXXXX_TDx_MISC_SPEED_HIGH          0x02
/**
 * @note Global Registers
 */
/* Valid Touching Detect Threshold Register */
#define FTXXXX_THGROUP_VAL_MASK             0xFF
#define FTXXXX_THGROUP_VAL_SHIFT               0
/* Valid Touching Peak Detect Threshold Register */
#define FTXXXX_THCAL_VAL_MASK                0xFF
#define FTXXXX_THCAL_VAL_SHIFT                  0
/* Compensation Status Register */
#define FTXXXX_COMP_STATUS_VAL_MASK         0xFF
#define FTXXXX_COMP_STATUS_VAL_SHIFT           0
/* Compensation Flag Register */
#define FTXXXX_COMP_FLAG_VAL_MASK           0xFF
#define FTXXXX_COMP_FLAG_VAL_SHIFT             0
/* Control Register */
#define FTXXXX_CONTROL_MONITOR_MASK         0x01
#define FTXXXX_CONTROL_MONITOR_SHIFT           0
#define FTXXXX_CONTROL_MONITOR_ENABLED         1
#define FTXXXX_CONTROL_MONITOR_DISABLED        0
/* More registers here */
/* Maximum resolution of X axis (Higher byte)*/
#define FTXXXX_MAX_X_H_VAL_MASK             0xFF
#define FTXXXX_MAX_X_H_VAL_SHIFT               0
/* Maximum resolution of X axis (Lower byte)*/
#define FTXXXX_MAX_X_L_VAL_MASK             0xFF
#define FTXXXX_MAX_X_L_VAL_SHIFT               0
/* Maximum resolution of Y axis (Higher byte)*/
#define FTXXXX_MAX_Y_H_VAL_MASK             0xFF
#define FTXXXX_MAX_Y_H_VAL_SHIFT               0
/* Maximum resolution of Y axis (Lower byte)*/
#define FTXXXX_MAX_Y_L_VAL_MASK             0xFF
#define FTXXXX_MAX_Y_L_VAL_SHIFT               0


/* More registers here */
/* Auto Calibration Mode Register */
#define FTXXXX_AUTO_CLB_MODE_MASK           0xFF
#define FTXXXX_AUTO_CLB_MODE_SHIFT             0
#define FTXXXX_AUTO_CLB_MODE_ENABLE         0x00
#define FTXXXX_AUTO_CLB_MODE_DISABLE        0xFF
/* Library version Register (High byte) */
#define FTXXXX_LIB_VERSION_H_VAL_MASK       0xFF
#define FTXXXX_LIB_VERSION_H_VAL_SHIFT         0
/* Library version Register (Low byte) */
#define FTXXXX_LIB_VERSION_L_VAL_MASK       0xFF
#define FTXXXX_LIB_VERSION_L_VAL_SHIFT         0
/* Chip ID Register */
#define FTXXXX_CHIPID_VAL_MASK              0xFF
#define FTXXXX_CHIPID_VAL_SHIFT                0
/* Interrupt Status (Mode) Register */
#define FTXXXX_MODE_VAL_MASK                0x01
#define FTXXXX_MODE_VAL_SHIFT                  0
#define FTXXXX_MODE_VAL_POLLING                0
#define FTXXXX_MODE_VAL_TRIGGER                1
/* Power Consume Register */
#define FTXXXX_PMODE_VAL_MASK               0xFF
#define FTXXXX_PMODE_VAL_SHIFT                 0
#define FTXXXX_PMODE_VAL_ACTIVE                0
#define FTXXXX_PMODE_VAL_MONITOR               1
#define FTXXXX_PMODE_VAL_HIBERNATE             3
/* Firmware ID Register */
#define FTXXXX_FIRMID_VAL_MASK              0xFF
#define FTXXXX_FIRMID_VAL_SHIFT                0
/* Running State Register */
#define FTXXXX_STATE_VAL_MASK               0xFF
#define FTXXXX_STATE_VAL_SHIFT                 0
#define FTXXXX_STATE_VAL_CONFIGURE             0
#define FTXXXX_STATE_VAL_WORK                  1
#define FTXXXX_STATE_VAL_CALIBRATION           2
#define FTXXXX_STATE_VAL_FACTORY               3
#define FTXXXX_STATE_VAL_AUTOCALIBRATION       4
/* Vendor ID Register */
#define FTXXXX_VENDORID_VAL_MASK            0xFF
#define FTXXXX_VENDORID_VAL_SHIFT              0
/* Error Register */
#define FTXXXX_ERR_VAL_MASK                 0xFF
#define FTXXXX_ERR_VAL_SHIFT                   0
#define FTXXXX_ERR_VAL_OK                   0x01
#define FTXXXX_ERR_VAL_INVALIDREAD          0x03
#define FTXXXX_ERR_VAL_STARTFAIL            0x05
#define FTXXXX_ERR_VAL_CALIBRATIONERROR     0x1A
/* More registers here */
/* Log Message Count Register */
#define FTXXXX_MSG_CNT_VAL_MASK             0xFF
#define FTXXXX_MSG_CNT_VAL_SHIFT               0
/* Log Message Data (char) Register */
#define FTXXXX_MSG_CHA_VAL_MASK             0xFF
#define FTXXXX_MSG_CHA_VAL_SHIFT               0


#endif // FTXXXX_H
