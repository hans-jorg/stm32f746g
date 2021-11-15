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

#define I2C_CONF_MODE_NORMAL         (0)
#define I2C_CONF_MODE_FAST           (1)
#define I2C_CONF_MODE_FASTPLUS       (2)
#define I2C_CONF_MODE_MASK           (3)

#define I2C_CONF_FILTER_NONE         (1<<4)
#define I2C_CONF_FILTER_ANALOG       (1<<5)
#define I2C_CONF_FILTER_DIGITAL_Pos  (6)
#define I2C_CONF_FILTER_DIGITAL_1    (1<<I2C_FILTER_DIGITAL_Pos)
#define I2C_CONF_FILTER_DIGITAL_2    (2<<I2C_FILTER_DIGITAL_Pos)
#define I2C_CONF_FILTER_DIGITAL_MASK (0xF<<I2C_CONF_FILTER_DIGITAL_Pos)

/*
 * The calculation of the timing parameters (PRESC,SCLDEL,SDADEL,SCLH,SCLL) 
 * is a PITA. 
 * 
 * The easiest way is to use STM32CubeMX.
 * Do not forget to specify tr and tf, because they have a bit impact on the
 * timing parameters
 * 
 * Below there are some precalculated values for timing according the speed 
 * and the filters used
 */

/* For Standard Mode */
#define I2C_TIMING_STANDARD_NONE        0x00503D5A
#define I2C_TIMING_STANDARD_ANALOG      0x00503D58
#define I2C_TIMING_STANDARD_DNF_1       0x00503C59
#define I2C_TIMING_STANDARD_DNF_2       0x00503B58
/* For Fast Mode */
#define I2C_TIMING_FAST_NONE            0x00300718
#define I2C_TIMING_FAST_ANALOG          0x00300617
#define I2C_TIMING_FAST_DNF_1           0x00300617
#define I2C_TIMING_FAST_DNF_2           0x00300912
/* For Fast Plus Mode */
#define I2C_TIMING_FASTPLUS_NONE        0x00200205
#define I2C_TIMING_FASTPLUS_ANALOG      0x00200105
#define I2C_TIMING_FASTPLUS_DNF_1       0x00200004
#define I2C_TIMING_FASTPLUS_DNF_2       0x00200003

int I2CMaster_Init( I2C_TypeDef *i2c, uint32_t conf, uint32_t timing );

int I2CMaster_Write( I2C_TypeDef *i2c, uint32_t address, unsigned char *data, int n);

int I2CMaster_Read( I2C_TypeDef *i2c, uint32_t address, unsigned char *data, int n);

int I2CMaster_WriteAndRead( I2C_TypeDef *i2c, uint32_t address, 
                            unsigned char *writedata, int nwrite,
                            unsigned char *readdata,  int nread);

#endif // I2C_MASTER_H
