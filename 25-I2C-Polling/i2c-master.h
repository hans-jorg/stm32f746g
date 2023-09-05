#ifndef I2C_MASTER_H
#define I2C_MASTER_H
/**
 * @file    i2c-master.h
 *
 * @brief   I2C implementation of master interface
 *
 * @note    Simple implementation of a I2C Master
 *
 * @note    NORMAL MODE\:            100 KHz
 *          FAST MODE\:              400 KHz
 *          FAST PLUS MODE\:        1000 KHz
 *
 * @author  Hans
 */
/* Field 1-0: Mode=Speed */
#define I2C_CONF_MODE_Pos            (0)
#define I2C_CONF_MODE_NORMAL         (0<<I2C_CONF_MODE_Pos)
#define I2C_CONF_MODE_FAST           (1<<I2C_CONF_MODE_Pos)
#define I2C_CONF_MODE_FASTPLUS       (2<<I2C_CONF_MODE_Pos)
#define I2C_CONF_MODE_MASK           (3<<I2C_CONF_MODE_Pos)
/* Field 5-4: Filter to use */
#define I2C_CONF_FILTER_DIGITAL_Pos  (4)
#define I2C_CONF_FILTER_NONE         (0<<I2C_CONF_FILTER_DIGITAL_Pos)
#define I2C_CONF_FILTER_ANALOG       (1<<I2C_CONF_FILTER_DIGITAL_Pos)
#define I2C_CONF_FILTER_DIGITAL      (2<<I2C_CONF_FILTER_DIGITAL_Pos)
#define I2C_CONF_FILTER_BOTH         (3<<I2C_CONF_FILTER_DIGITAL_Pos)
#define I2C_CONF_FILTER_MASK         (3<<I2C_CONF_FILTER_DIGITAL_Pos)
/* Field  10-7  : DNF. Only used when Digital Filter is enabled */
#define I2C_CONF_FILTER_DNF_Pos      (7)
#define I2C_CONF_FILTER_DNF_0        (0<<I2C_FILTER_FILTER_DNF_Pos)
#define I2C_CONF_FILTER_DNF_1        (1<<I2C_FILTER_FILTER_DNF_Pos)
#define I2C_CONF_FILTER_DNF_2        (2<<I2C_FILTER_FILTER_DNF_Pos)
#define I2C_CONF_FILTER_DNF_3        (3<<I2C_FILTER_FILTER_DNF_Pos)
#define I2C_CONF_FILTER_DNF_4        (4<<I2C_FILTER_FILTER_DNF_Pos)
#define I2C_CONF_FILTER_DNF_5        (5<<I2C_FILTER_FILTER_DNF_Pos)
#define I2C_CONF_FILTER_DNF_6        (6<<I2C_FILTER_FILTER_DNF_Pos)
#define I2C_CONF_FILTER_DNF_7        (7<<I2C_FILTER_FILTER_DNF_Pos)
#define I2C_CONF_FILTER_DNF_8        (8<<I2C_FILTER_FILTER_DNF_Pos)
#define I2C_CONF_FILTER_DNF_9        (9<<I2C_FILTER_FILTER_DNF_Pos)
#define I2C_CONF_FILTER_DNF_10       (10<<I2C_FILTER_FILTER_DNF_Pos)
#define I2C_CONF_FILTER_DNF_11       (11<<I2C_FILTER_FILTER_DNF_Pos)
#define I2C_CONF_FILTER_DNF_12       (12<<I2C_FILTER_FILTER_DNF_Pos)
#define I2C_CONF_FILTER_DNF_13       (13<<I2C_FILTER_FILTER_DNF_Pos)
#define I2C_CONF_FILTER_DNF_14       (14<<I2C_FILTER_FILTER_DNF_Pos)
#define I2C_CONF_FILTER_DNF_15       (15<<I2C_FILTER_FILTER_DNF_Pos)
#define I2C_CONF_FILTER_DNF_MASK     (0xF<<I2C_CONF_FILTER_DNF_Pos)


/** @brief Clock source
 *  @note  Field 15-12: Clock source.
 *         The encoding used here is different from the one in I2CxSEL field!!
 *         When this configuration parameter, is omitted, the default is HSI
 */
#define I2C_CONF_CLOCK_Pos           (12)
#define I2C_CONF_CLOCK_HSICLK        (0<<I2C_CONF_CLOCK_Pos)
#define I2C_CONF_CLOCK_SYSCLK        (1<<I2C_CONF_CLOCK_Pos)
#define I2C_CONF_CLOCK_APB1CLK       (2<<I2C_CONF_CLOCK_Pos)
#define I2C_CONF_CLOCK_MASK          (3<<I2C_CONF_CLOCK_Pos)


/**
 * @brief I2C Status
 */
typedef enum {
        I2C_UNINITIALIZED = 0,
        I2C_READY = 3,
        I2C_READING = 4,
        I2C_WRITING = 5,
        I2C_DISABLED = 6,
        I2C_ERROR = 7
     } I2C_Status_t;

/* Function prototypes */

int I2CMaster_Init(         I2C_TypeDef *i2c,
                            uint32_t conf,
                            uint32_t timing
                            );

int I2CMaster_Write(        I2C_TypeDef *i2c,
                            uint16_t address,
                            uint8_t *data,
                            uint16_t n
                            );

int I2CMaster_Read(         I2C_TypeDef *i2c,
                            uint16_t address,
                            uint8_t *data,
                            uint16_t n
                            );

int I2CMaster_WriteAndRead( I2C_TypeDef *i2c,
                            uint16_t address,
                            uint8_t *writedata, int nwrite,
                            uint8_t *readdata,  int nread
                            );

int I2CMaster_Detect(       I2C_TypeDef *i2c,
                            uint16_t addr );

I2C_Status_t
I2CMaster_GetStatus(        I2C_TypeDef *i2c );


#endif // I2C_MASTER_H
