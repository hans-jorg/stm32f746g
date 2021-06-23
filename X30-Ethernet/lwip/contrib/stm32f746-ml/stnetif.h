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

// The functions below must be called in the main loop
void        stnetif_input(struct netif *netif);
void        stnetif_link(struct netif *netif);

//
// Default callback routines 
// They must be defined by the netif_set_{link,remove,status}_callback functions
// They just print a message
//

#if LWIP_NETIF_LINK_CALLBACK
void stnetif_link_callback(struct netif *netif);
#endif

#if LWIP_NETIF_STATUS_CALLBACK
void stnetif_status_callback(struct netif *netif);
#endif

#if LWIP_NETIF_REMOVE_CALLBACK
void stnetif_remove_callback(struct netif *netif);
#endif


#endif
