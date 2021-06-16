#ifndef CC_H
#define CC_H
/** 
 * @file    cc.h
 * 
 * @note    Compiler dependent macros, functions and definitions
 */


#ifndef BYTE_ORDER
#define BYTE_ORDER LITTLE_ENDIAN
#endif

#define lock_interrupts()     __disable_irq()
#define unlock_interrupts()   __enable_irq()

#endif // CC_H
