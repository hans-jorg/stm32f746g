#ifndef CMSIS_OS_H
#define CMSIS_OS_H
/**
 * @file cmsis_os.h
 *
 * @note OS emulation layer for lwip
 *
 * @note not needed when using raw (native) API or not using sequential APIs
 */

#if WITH_RTOS != 0

void sys_init(void);

// etc


#endif // WITH_RTOS != 0

#endif // CMSIS_OS_H

