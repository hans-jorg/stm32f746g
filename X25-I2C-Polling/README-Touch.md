Touch Controller
================

Introduction
------------

The STM32F746 Discovery board has a LCD display with a touch sensor.

The touch sensor is based on the FT5336GQQ device from Focal Tech incorporated in the RK043FN48H LCD display. A heavly customized LCD Display for ST discovery boards.It is also used in other Discovery boards like STM32H756 and STM32F7508.

The FT5336 can be customized by the LCD manufacturer. There is no clear data sheet for software interfacing to the touch controller.

Some functionalities can be derived from other similar products of the same manufacturer. Some
information can be found by reverse engineering, looking code from ST or other libraries.

One important difference is that the FT5336 implementation on the RK043FN48H can only handle 5 touch points.

Touchscreen connection
----------------------

The touch controller is connected to the MCU thru a I2C interface (I2C3) shared with the audio controller.

There are four connection between the touch controller and the MCU.

| Signal    |   Pin   |
|-----------|---------|
|  LCD_SCL  |  PH7    | 
|  LCD_SDA  |  PH8    |
|  LCD_INT  |  PI13   |
|  LCD_RST  |  NRST   |


Interrupt interface
-------------------

The Touch Controller (TC) generates an interrupt signal (LCD_INT) when a touch is detected.

The interrupt processing must start a reading process, but not wait for it, since it will disable interrupt for a long time (transmitting and receiving data at 100 or 400 bps Kbps).


The STM32F7xx processes interrupts using the EXTI peripheral. The EXTI can generate an interrupt or an event. The main difference is that interrupts cause the execution of the corresponding Interrupt Service Routine (ISR) and events, not. Events are mainly used to make the MCU awakens from a Low Power Level.


There are 16 interrupts, one for each bit of a 16-bit word. For example, PA0, PB0, PC0 and PK0 generates the same interrupt, EXTI0. But some interrupts are grouped. So there are only 7 ISRs. Interrupts for bits 5 to 9 are grouped into a EXTI9\_5 interrupt. So are the interrupts for bits 10 to 15 grouped into EXTI15\_10.

| Pos  | IRQ# | Name            | IRQ Handler            |  Address       |
|------|------|-----------------|------------------------|----------------|
|  22  |   6  | EXTI0\_IRQn     | EXTI0_IRQHandler       | 0x0000 0058    |
|  23  |   7  | EXTI1\_IRQn     | EXTI1_IRQHandler       | 0x0000 005C    |
|  24  |   8  | EXTI2\_IRQn     | EXTI2_IRQHandler       | 0x0000 0060    |
|  25  |   9  | EXTI3\_IRQn     | EXTI3_IRQHandler       | 0x0000 0064    |
|  26  |  10  | EXTI4\_IRQn     | EXTI4_IRQHandler       | 0x0000 0068    |
|  39  |  23  | EXIT9\_5\_IRQn  | EXTI9\_5\_IRQHandler   | 0x0000 009C    |
|  56  |  40  | EXTI15\_10\_IRQn| EXTI15\_10\_IRQHandler | 0x0000 00E0    |


Additional lines (EXTI16 to EXTI23) correspond to other exceptions sources as shown below.

| Signal |IRQ#  | Name             | Handler                    | Description                    |
|--------|------|------------------|----------------------------|--------------------------------|
| EXTI16 |   1  | PVD\_IRQn        | PVD_IRQHandler             | PVD output                     |
| EXTI17 |  41  | RTC_Alarm_IRQn   | RTC_Alarm_IRQHandler       | RTC Alarm event                |
| EXTI18 |  42  | OTG\_FS_WKUP_IRQn| OTG_FS_WKUP_IRQHandler     | USB OTG FS Wakeup event        |
| EXTI19 |  62  | ETH_WKUP_IRQn    | ETH_WKUP_IRQHandler        | Ethernet Wakeup event          |
| EXTI20 |  76  | OTG\_HS_WKUP_IRQn| OTG_HS_WKUP_Handler        | USB OTG HS Wakeup event        |
| EXTI21 |   2  | TAMP_STAMP_IRQn  | RTC_TAMP_STAMP_IRQHandler  | RTC Tamper and TimeStamp events|
| EXTI22 |   3  | RTC_WKUP_IRQn    | RTC_WKUP_IRQHandler        | RTC Wakeup event               |
| EXTI23 |   -  |      -           |              -             | LPTIM1 asynchronous event      |


To enable an interrupt:

1. Configure GPIO pin as input (Enable clock for its GPIO)
2. Set the corresponding bit in EXTI_IMR
3. Configure trigger selection (rising or falling) in EXTI\_RTSR or EXTI\_FTSR.
4. Enable the corresponding IRQn in NVIC.


Interfacing to the FT5336
-------------------------

The FT5336 uses an I2C interface and generated an interrupt.
The interface is done by accessing registers of the FT5336. A list is in [Annex A](#annex-a) 

The format of the I2C packet for a write command is show below

    START | I2C Addr W| Cmd Addr |  Value  | STOP 
    
To read a register, the following sequence is needed.

    START | I2C Addr W| Cmd Addr | STOP 
    START | I2C Addr R| Cmd Addr |  Value  | STOP 


Annex A 
-------

### Registers of Touch Controller

| Addr | Command                       | RW | 5x26 | 5x16 | 5X06 |
|------|-------------------------------|----|------|------|------|
| 0x00 | MODE_SWITCH                   | RW |   X  |   X  |   X  |
| 0x01 | GESTURE/GEST_ID               | RO |   X  |   X  |   X  |
| 0x02 | CUR_POINT/TD_STATUS           | RO |   X  |   X  |   X  |
| 0x03 | TOUCH1_XH                     | RO |   X  |   X  |   X  |
| 0x04 | TOUCH1_XL                     | RO |   X  |   X  |   X  |
| 0x05 | TOUCH1_YH                     | RO |   X  |   X  |   X  |
| 0x06 | TOUCH1_YL                     | RO |   X  |   X  |   X  |
| 0x07 | TOUCH1_WEIGHT                 | RO |   X  |   X  |   X  |
| 0x08 | TOUCH1_MISC                   | RO |   X  |   X  |   X  |
| 0x09 | TOUCH2_XH                     | RO |   X  |   X  |   X  |
| 0x0A | TOUCH2_XL                     | RO |   X  |   X  |   X  |
| 0x0B | TOUCH2_YH                     | RO |   X  |   X  |   X  |
| 0x0C | TOUCH2_YL                     | RO |   X  |   X  |   X  |
| 0x0D | TOUCH2_WEIGHT                 | RO |   X  |   X  |   X  |
| 0x0E | TOUCH2_MISC                   | RO |   X  |   X  |   X  |
| 0x0F | TOUCH3_XH                     | RO |   X  |   X  |   X  |
| 0x10 | TOUCH3_XL                     | RO |   X  |   X  |   X  |
| 0x11 | TOUCH3_YH                     | RO |   X  |   X  |   X  |
| 0x12 | TOUCH3_YL                     | RO |   X  |   X  |   X  |
| 0x13 | TOUCH3_WEIGHT                 | RO |   X  |   X  |   X  |
| 0x14 | TOUCH3_MISC                   | RO |   X  |   X  |   X  |
| 0x15 | TOUCH4_XH                     | RO |   X  |   X  |   X  |
| 0x16 | TOUCH4_XL                     | RO |   X  |   X  |   X  |
| 0x17 | TOUCH4_YH                     | RO |   X  |   X  |   X  |
| 0x18 | TOUCH4_YL                     | RO |   X  |   X  |   X  |
| 0x19 | TOUCH4_WEIGHT                 | RO |   X  |   X  |   X  |
| 0x1A | TOUCH4_MISC                   | RO |   X  |   X  |   X  |
| 0x1B | TOUCH5_XH                     | RO |   X  |   X  |   X  |
| 0x1C | TOUCH5_XL                     | RO |   X  |   X  |   X  |
| 0x1D | TOUCH5_YH                     | RO |   X  |   X  |   X  |
| 0x1E | TOUCH5_YL                     | RO |   X  |   X  |   X  |
| 0x1F | TOUCH5_WEIGHT                 | RO |   X  |   X  |   X  |
| 0x20 | TOUCH5_MISC                   | RO |   X  |   X  |   X  |
| 0x21 | TOUCH6_XH                     | RO |   X  |      |      |
| 0x22 | TOUCH6_XL                     | RO |   X  |      |      |
| 0x23 | TOUCH6_YH                     | RO |   X  |      |      |
| 0x24 | TOUCH6_YL                     | RO |   X  |      |      |
| 0x25 | TOUCH6_WEIGHT                 | RO |   X  |      |      |
| 0x26 | TOUCH6_MISC                   | RO |   X  |      |      |
| 0x27 | TOUCH7_XH                     | RO |   X  |      |      |
| 0x28 | TOUCH7_XL                     | RO |   X  |      |      |
| 0x29 | TOUCH7_YH                     | RO |   X  |      |      |
| 0x2A | TOUCH7_YL                     | RO |   X  |      |      |
| 0x2B | TOUCH7_WEIGHT                 | RO |   X  |      |      |
| 0x2C | TOUCH7_MISC                   | RO |   X  |      |      |
| 0x2D | TOUCH8_XH                     | RO |   X  |      |      |
| 0x2E | TOUCH8_XL                     | RO |   X  |      |      |
| 0x2F | TOUCH8_YH                     | RO |   X  |      |      |
| 0x30 | TOUCH8_YL                     | RO |   X  |      |      |
| 0x31 | TOUCH8_WEIGHT                 | RO |   X  |      |      |
| 0x32 | TOUCH8_MISC                   | RO |   X  |      |      |
| 0x33 | TOUCH9_XH                     | RO |   X  |      |      |
| 0x34 | TOUCH9_XL                     | RO |   X  |      |      |
| 0x35 | TOUCH9_YH                     | RO |   X  |      |      |
| 0x36 | TOUCH9_YL                     | RO |   X  |      |      |
| 0x37 | TOUCH9_WEIGHT                 | RO |   X  |      |      |
| 0x38 | TOUCH9_MISC                   | RO |   X  |      |      |
| 0x39 | TOUCH10_XH                    | RO |   X  |      |      |
| 0x3A | TOUCH10_XL                    | RO |   X  |      |      |
| 0x3B | TOUCH10_YH                    | RO |   X  |      |      |
| 0x3C | TOUCH10_YL                    | RO |   X  |      |      |
| 0x3D | TOUCH10_WEIGHT                | RO |   X  |      |      |
| 0x3E | TOUCH10_MISC                  | RO |   X  |      |      |
| ...  | ...                           | ...| ...  | ...  | ...  |
| 0x80 | ID_G_THGROUP                  | RW |   X  |   X  |   X  |
| 0x81 | ID_G_THPEAK                   | RW |      |   X  |   X  |
| 0x82 | ID_G_THCAL                    | RW |      |   X  |   X  |
| 0x83 | ID_G_COMPENSATE_STATUS/THTEMP | RO |      |   X  |   X  |
| 0x84 | ID_G_COMPENSATE_FLAG          | RO |      |   X  |   X  |
| 0x85 | ID_G_THDIFF                   | RW |   X  |   X  |      | 
| 0x86 | ID_G_CTRL                     | RW |   X  |   X  |   X  |
| 0x87 | ID_G_TIME_ENTER_MONITOR       | RW |   X  |   X  |   X  |
| 0x88 | ID_G_PERIODACTIVE             | RW |   X  |   X  |   X  |
| 0x89 | ID_G_PERIODMONITOR            | RW |   X  |   X  |   X  |
| 0x8A | ID_G_SCAN_RATE                | RO |      |   X  |      |
| 0x8B | ID_G_CHARGER_STATE            | RO |      |   X  |      |
| 0x8C | ID_G_SCAN_REGB                | RO |      |   X  |      |
| 0x8D | ID_G_SCAN_CAP                 | RO |      |   X  |      |
| 0x8E | ID_G_SCAN_FILTERMODE          | RO |      |   X  |      |
| 0x8F | ID_G_SCAN_REFRESH             | RO |      |   X  |      |
| 0x90 | ID_G_MOVSTH_I                 | RW |      |   X  |      |
| 0x91 | ID_G_MOVSTH_N                 | RW |   X  |   X  |      |
| 0x92 | ID_G_LEFT_RIGHT_OFFSET        | RW |   X  |   X  |      |
| 0x93 | ID_G_UP_DOWN_OFFSET           | RW |   X  |   X  |      |
| 0x94 | ID_G_DISTANCE_LEFT_RIGHT      | RW |   X  |   X  |      |
| 0x95 | ID_G_DISTANCE_UP_DOWN         | RW |   X  |   X  |      |
| 0x96 | Reserved                      | RW |   X  |   X  |      |
| 0x97 | ID_G_ZOOM_DIS_SQR             | RW |      |   X  |      |
| 0x98 | ID_G_MAX_X_HIGH               | RW |      |   X  |      |
| 0x99 | ID_G_MAX_X_LOW                | RW |      |   X  |      |
| 0x9A | ID_G_MAX_Y_HIGH               | RW |      |   X  |      |
| 0x9B | ID_G_MAX_Y_LOW                | RW |      |   X  |      |
| 0x9C | ID_G_K_X_HIGH                 | RW |      |   X  |      |
| 0x9D | ID_G_K_X_LOW                  | RW |      |   X  |      |
| 0x9E | ID_G_K_Y_HIGH                 | RW |      |   X  |      |
| 0x9F | ID_G_K_Y_LOW                  | RW |      |   X  |      |
| 0xA0 | ID_G_AUTO_CLB_MODE            | RW |      |   X  |   X  |
| 0xA1 | ID_G_LIB_VERSION_H            | RO |   X  |   X  |   X  |
| 0xA2 | ID_G_LIB_VERSION_L            | RO |   X  |   X  |   X  |
| 0xA3 | ID_G_CIPHER                   | RO |   X  |   X  |   X  |
| 0xA4 | ID_G_MODE                     | RO |   X  |   X  |   X  |
| 0xA5 | ID_G_PMODE                    | RO |   X  |   X  |      |
| 0xA6 | ID_G_FIRMID                   | RO |   X  |   X  |   X  |
| 0xA7 | ID_G_STATE                    | RW |      |   X  |      |
| 0xA8 | ID_G_VENODRID                 | RO |   X  |   X  |      |
| 0xA9 | ID_G_ERR                      | RO |   X  |   X  |      |
| 0xAA | ID_G_CLB                      | RW |      |   X  |      |
| 0xAB | ID_G_STATIC_TH                | RW |      |   X  |      |
| 0xAC | ID_G_MID_SPEED_TH             | RW |      |   X  |      |
| 0xAD | ID_G_HIGH_SPEED_TH            | RW |      |   X  |      |
| 0xAE | ID_G_DRAW_LINE_TH             | RW |      |   X  |   X  |
| 0xAF | ID_G_RELEASE_CODE_ID          | RW |   X  |   X  |      |
| 0xB0 | ID_G_FACE_DEC_MODE            | RW |      |   X  |      |
| 0xB1 | -                             | -  |      |   -  |      |
| 0xB2 | ID_G_PRESIZE_EN               | RW |      |   X  |      |
| 0xB3 | ID_G_BIGAREA_PEAK_TH          | RW |      |   X  |      |
| 0xB4 | ID_G_BIGAREA_PEAK_NUM         | RW |      |   X  |      |
| ...  | ...                           | ...|  ... |  ... |  ... |
| 0xBC | ID_STATE                      | RW |   X  |   X  |      |
| ...  | ...                           | ...|  ... |  ... |  ... |
| 0xFE | LOG_MSG_CNT                   | RO |      |   X  |   X  |
| 0xFF | LOG_CUR_CHA                   | RO |      |   X  |   X  | 




References
----------

1. [FT5336GQQ Datasheet](https://support.newhavendisplay.com/hc/en-us/article_attachments/4414385992599/FT5336.pdf)
2. [RK043FN48H](RK043FN48H LCD Display)
3. [FT5x06 Datasheet](...)
4. [FT5x06 Registers](https://support.newhavendisplay.com/hc/en-us/articles/4410042626839-FT5x06-Registers)
5. [FT5x16 Registers](https://support.newhavendisplay.com/hc/en-us/article_attachments/4414385992599/FT5336.pdf)
6. [Application Note for FT5426 CTPM](https://support.newhavendisplay.com/hc/en-us/article_attachments/4414394382231/FT5426_5526Application_Note_Ver0.1.pdf)
7. [FT5336 Driver](https://github.com/STMicroelectronics/stm32-ft5336)