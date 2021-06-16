/**
 * @file    sys_arch.c
 * 
 * @note    Platform dependent support routines 
 */
#include "lwip/arch.h"
#include "sys_arch.h"
/**
 * @brief   Counter used to check timeouts
 *
 * @note    Incremented every 1 ms
 * @note    Overruns after 49 days!!!
 */

u32_t sys_counter;

/**
 * @brief   Implementation of sys_now
 * 
 * @note    It can not be inline'd because timeout.c does not include sys_arch.h
 * 
 * @return u32_t 
 */

u32_t sys_now(void) {
    return sys_counter;
}

/**
 * @brief Used to initialize random
 * 
 * @note    It can not be inline'd because timeout.c does not include sys_arch.h
 * 
 * @return u32_t 
 */
u32_t sys_jiffies(void) {
    return sys_counter;
}
