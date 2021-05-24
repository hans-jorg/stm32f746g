#ifndef CC_H
#define CC_H


#define BYTE_ORDER LITTLE_ENDIAN

#define lock_interrupts()     __disable_irq()
#define unlock_interrupts()   __enable_irq()


#endif // CC_H
