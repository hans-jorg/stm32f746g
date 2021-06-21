#ifndef STNEIF_H
#define STNEIF_H


#include "lwip/err.h"
#include "lwip/netif.h"

/**
 * @brief   Name of ethernet interface
 */
#define IFNAME0         'e'
#define IFNAME1         't'

/* Exported functions ------------------------------------------------------- */
err_t       stnetif_init(struct netif *netif);
void        stnetif_set_link(void const *argument);
void        stnetif_update_config(struct netif *netif);
void        stnetif_callback_conn_changed(struct netif *netif);


struct pbuf *stnetif_input(struct netif *netif);


#endif
