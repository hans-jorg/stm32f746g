#ifndef SYS_ARCH_H
#define SYS_ARCH_H


/**
 * @brief   Counter used to check timeouts
 *
 * @note    Incremented every 1 ms
 * @note    Overruns after 49 days!!!
 */

extern u32_t sys_counter;
/**
 * @brief   sys_count
 *
 * @note    Must be called every 1 ms!!!!!!!!!!!!!!!!
 * 
 * @note    Called in SysTick IRQ handler
 */
static inline void        sys_count(void) {
    sys_counter++;
}

#endif // SYS_ARCH_H
