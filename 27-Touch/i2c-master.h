#ifndef I2C_MASTER_H
#define I2C_MASTER_H
/**
 * @file    i2c-master.h
 *
 * @brief   I2C implementarion of master interface
 *
 * @note    Simple implementation
 *
 * @author  Hans
 */

#define I2C_CONF_MODE_NORMAL         (0)
#define I2C_CONF_MODE_FAST           (1)
#define I2C_CONF_MODE_FASTPLUS       (2)
#define I2C_CONF_MODE_MASK           (3)

#define I2C_CONF_FILTER_NONE         (0)
#define I2C_CONF_FILTER_ANALOG       (4)
#define I2C_CONF_FILTER_DIGITAL_Pos  (4)
#define I2C_CONF_FILTER_DIGITAL_MASK (0xF<<I2C_CONF_FILTER_DIGITAL_Pos)
#define I2C_CONF_FILTER_DIGITAL_1    (1<<I2C_FILTER_DIGITAL_Pos)
#define I2C_CONF_FILTER_DIGITAL_2    (2<<I2C_FILTER_DIGITAL_Pos)

/* Not yet */
/*#define I2C_FILTER_DIGITAL_3    (3<<I2C_FILTER_DIGITAL_Pos)*/
/*#define I2C_FILTER_DIGITAL_4    (4<<I2C_FILTER_DIGITAL_Pos)*/
/*#define I2C_FILTER_DIGITAL_5    (5<<I2C_FILTER_DIGITAL_Pos)*/
/*#define I2C_FILTER_DIGITAL_6    (6<<I2C_FILTER_DIGITAL_Pos)*/
/*#define I2C_FILTER_DIGITAL_7    (7<<I2C_FILTER_DIGITAL_Pos)*/
/*#define I2C_FILTER_DIGITAL_8    (8<<I2C_FILTER_DIGITAL_Pos)*/
/*#define I2C_FILTER_DIGITAL_9    (9<<I2C_FILTER_DIGITAL_Pos)*/
/*#define I2C_FILTER_DIGITAL_10   (10<<I2C_FILTER_DIGITAL_Pos)*/
/*#define I2C_FILTER_DIGITAL_11   (11<<I2C_FILTER_DIGITAL_Pos)*/
/*#define I2C_FILTER_DIGITAL_12   (12<<I2C_FILTER_DIGITAL_Pos)*/
/*#define I2C_FILTER_DIGITAL_13   (13<<I2C_FILTER_DIGITAL_Pos)*/
/*#define I2C_FILTER_DIGITAL_14   (14<<I2C_FILTER_DIGITAL_Pos)*/
/*#define I2C_FILTER_DIGITAL_15   (15<<I2C_FILTER_DIGITAL_Pos)*/


int I2CMaster_Init( I2C_TypeDef *i2c, uint32_t conf);

int I2CMaster_Write( I2C_TypeDef *i2c, uint32_t address, unsigned char data, int n);

#endif // I2C_MASTER_H
