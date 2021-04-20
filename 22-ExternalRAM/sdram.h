#ifndef SDAM_H
#define SDRAM_H
/**
 * @file    sdram.h
 *
 * @date    07/10/2020
 * @author  Hans
 */

int SDRAM_Init(void);

/**
 *  @brief  SystemCoreClock for correct working of the SDRAM
 *
 *  @note   It is divided by 2. So the SDRAM runs at 100 MHz
 *
 *  @note   Other frequencies are possible but the FMC and SDRAM must be reconfigured
 */

#define SDRAMCLOCKFREQUENCY     200000000

/**
 *  @brief  SDRAM address
 *
 *  @note   Address of SDRAM Bank 1. It is possible to remap it (not done).
 */

#define SDRAMAREA               0xC0000000

/**
 *  @brief  SDRAM size
 *
 *  @note   8 MBytes = 64 MBit
 *
 *  @note   Only half of the SDRAM is used because only 16 bits
 *          of the 32 bits are used.
 */

#define SDRAMSIZE               0x0800000

#endif
