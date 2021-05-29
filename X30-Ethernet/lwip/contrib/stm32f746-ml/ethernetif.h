#ifndef __ETHERNETIF_H__
#define __ETHERNETIF_H__


#include "lwip/err.h"
#include "lwip/netif.h"
#include "cmsis_os.h"

/* Exported functions ------------------------------------------------------- */
err_t       ethernetif_init(struct netif *netif);
void        ethernetif_set_link(void const *argument);
void        ethernetif_update_config(struct netif *netif);
void        ethernetif_callback_conn_changed(struct netif *netif);


void        ethernetif_input(struct netif *netif);
/**
 * @brief   Counter used to check timeouts
 *
 * @note    Incremented every 1 ms
 *
 * @note    Overruns after 49 days!!!
 */
extern u32_t sys_counter;
/**
 * @brief   sys_count
 *
 * @note    Must be called every 1 ms!!!!!!!!!!!!!!!!1
 */
static inline void        sys_count(void) {
    sys_counter++;
}


u32_t       sys_jiffies(void);


#define HOSTNAME            "lwtst"

#endif
