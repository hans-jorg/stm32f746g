Ethernet connection using LwIP
==============================


Introduction
------------

To have a working TCP/IP connection a set of requisites must be attended.

* A connection to a Ethernet switch and a DHCP server.
* An Ethernet physical interface
* An Ethernet controller
* A software implementation of the TCP/IP stack over Ethernet


The software implementation of a TCP/IP stack is a daunting task, specially,   
in a embedded system, that generally has constrained resources 
(performance, memory).

The Light-Weight IP (lwIP) library is a library tailored for such systems.



> NOTE: Since the STM32F7xx micro-controllers have a L1 cache, special care must 
> be taken to write software for device that have Direct Memory Capabilities 
> (DMA). Writing to a memory position or reading from it do not mean that you 
> get the memory contents. See below for the cache handling.

Initialization
--------------



The ETH interface 
-----------------

The STM32F746 MCU has an interface (ETH) to an Ethernet controller with 
support to 10/100 Mbps Ethernet (Ethernet/Fast Ethernet).

The controller is compliant to:

* IEEE 802.3 standard (10/100 MHz)
* IEEE 1588 precision network clock synchronization (PTP)
* Media Independent Interface (MII) specification.
* Reduced Media Independent Interface (RMII) specification.

The main component of the Ethernet controller is the Media Access Control (MAC)
interface. It gets data from the network and transfers it to the processor 
memory. To do this, the Ethernet controller has a builtin DMA controller. It does 
not use the STM32F7xxx general purpose DMA controller. 

To bridge the pins of the Ethernet interface and the pins of MAC unit, an
 external physical interface controller (PHY) is needed.

The Ethernet controller has two interface to such devices:

* Media Independent Interface (MII).
* Reduced Media Independent Interface (RMII) 

Furthermore , the ETH controller implements an additional communication mechanism
to the PHY device, the Station Management Interface (SMI). It is a serial
 interface to access PHY registers. It uses two signals:

 * MDC: a clock signal with 2.5 MHz maximal frequency
 * MDIO: a bidirectional data signal.


The MII interface uses 17 pins (including MDC and MDIO) to communicate with the PHY device. 
It needs two clock signals: TX_CLK and RX_CLK from the PHY device. Both are 2.5 MHz for 10 Mbps
and 25 MHz for a 100 Mbps speed. The clock signal for the PHY device can be provided by the STM32F74xx
thru the MCO pin.

The RMII interface 9 pins (including MDC and MDIO). Among them, a REF_CLK signal, that must be a 
50 MHz signal provided by an external source and shared by the MAC interface and the PHY device.
Some PHY devices can generate the 50 MHz using a PLL.

It can generate interrupts with IRQ 68 (69 for wake-up).



Cache handling
--------------

The STM32F746 has a L1 cache splitted in two separated caches:

* Data cache (D-cache)
* Instruction cache (I-Cache)

Both are 32 bytes wide and 4 KBytes large.



The ETH controller has an independent DMA unit. It does not use the General 
Purpose DMA controller available in the STM32F746 devices.

The communication between the CPU and the ETH controller happens thru:

* Registers mapped in a non cacheable memory region
* Descriptors stored in RAM memory and accessed/updated by the CPU and the ETH controller.

The STM32F746 has many builtin RAM units.

| RAM     | Size   | Address range           |                                  |
|---------|--------|-------------------------|----------------------------------|
| SRAM1   | 240 KB | 0x2001_0000-0x2004_BFFF | System RAM 1                     |
| SRAM2   |  16 KB | 0x2004_C000-0x2004_FFFF | System RAM 2                     |
| DTCMRAM |  64 KB | 0x2000_0000-0x2000_FFFF | Data Tightly Coupled RAM         | 
| ITCMRAM |  16 KB | 0x0000_0000-0x0000_3FFF | Instruction Tightly Coupled RAM  |
| BKPSRAM |   4 KB | 0x4002_4000-0x4002_4FFF | Battery backup RAM               |

Different memory regions have 

| RAM      | Attributes                      |
|----------|---------------------------------|
| SRAM1    | Cacheable, Write-Through        |
| SRAM2    | Cacheable, Write-Through 
| DTCMRAM  | Non cacheable
| ITCMRAM  | Non cacheable, non shareable    |



> NOTE: The two default regions of SDRAM banks are not cacheable. So even if the cache is
enabled, the data or instructions will not go through the cache. To benefit from the cache
acceleration, the SDRAM banks can be remapped from 0xC000 0000 and 0xD000 0000 to
0x6000 0000 and 0x7000 0000 respectively, which are, by default, cacheable regions. This
is done by setting the field SWP_FMC [1:0] = 01 in the SYSCFG_MEMRMP register. If the
remapping is not suitable for the application, the Cortex ® -M7 MPU can be used to modify
the propriety of the default SDRAM memory region to be cacheable. (AN4667)

The cache coherency problem consists of one device (CPU) accessing one content 
and other device (DMA) accessing other content from the same memory address. 
This occurs when one device access the memory thru a cache and uses the information
in cache for the information in memory.

When allocating buffers and descriptors for the ETH controller, there are 
the following alternatives:

* Disable D-Cache (with a performance penalty).
* Set a write
* Use normal memory and use cache clean and invalidate before access and clean after.
* Use non cacheable regions for buffers and descriptors.
* Use the MPU to make the memory area used for buffers and descriptors non-cacheable.


The Ethernet interface on the STM32F746 Discovery board
-------------------------------------------------------

The Ethernet interface uses as PHY device the Microchip LAN8742A Ethernet Transceiver.
The connector is a HR961160C connector with embedded transformers and LEDs.

The LAN8742A can work in two clock modes:

* *REF_CLK In Mode (nINT)*: The nINT pin is an interrupt generator or a 
* *REF_CLK Out Mode*: 

The transceiver uses a 25 MHz clock sourced by the same oscillator used by the MCU.
But the MAC and PHY devices need a 50 MHz clock signal.

This is configured by the value at nINTSEL pin at Power On Reset (POR). By default, 
it is configured to nINT mode by a pull-up resistor. In this case, the nINT/REFCLKO pin 
is used by the PHY device to generate interrupts on the STM32F74xx and 
the 50 MHz REF_CLK signal must be generated externally and feed to the XTAL1/CLKIN pin.
When nINTSEL is pulled down, the nINT/REFCLKO pin is used to output the 50 MHz REFCLKO 
signal, that is generated internally from the 25 MHz XTAL or CLKIN clock signal.
In this case, the LAN8742 can not use the nINT pin to generate interrupts. 
The interrupt signal can be routed to the LED1 or LED2 pins, by XXXXXXXx.
But in the STM32F746 Discovery board, the LED1 and LED2 signals are used to
control the LEDs in the RJ45 connector. So the PHY device in the STM32F746 can
 not generate interrupts.

The Ethernet interface uses the MCU pins below.

| Board Signal | MCU Pin  |  PHY  Pin           | Description                |
|--------------|----------|--------------------|-----------------------------|
| RMII_TX_EN   |  PG11    |  TXEN              | Transmit Enable             |
| RMII_TXD0    |  PG13    |  TXD0              | Transmit Data 0             |
| RMII_TXD1    |  PG14    |  TXD1              | Transmit Data 1             |
| RMII_RXD0    |  PC4     |  RXD0/MODE0        | Receive Data 0              |
| RMII_RXD1    |  PC5     |  RDD1/MODE1        | Receive Data 1              |
| RMII_RXER    |  PG2     |  RXER/PHYAD0       | Receive Error               |
| RMII_CRS_DV  |  PA7     |  CRS_DV/MODE2      | Carrier Sense/Data Valid    |
| RMII_MDC     |  PC1     |  MDC               | SMI Clock                   |
| RMII_MDIO    |  PA2     |  MDIO              | SMI Data Input/Output       |
| RMII_REF_CLK |  PA1     |  nINT/REFCLK0      | 50 MHz REF_CLK clock signal |
| NRST         |          |  rRST              | Reset signal                |
| OSC_25M      |          |  XTAL1/CLKIN       | 25 MHz clock signal         |


The pin usage is shown below.

| MCU Pin|  Signal name          | AF | Alternatives |
|--------|-----------------------|----|--------------|
|  PG11  |   ETH_RMII_TXEN       | 11 | PB11         |
|  PG13  |   ETH_RMII_TXD0       | 11 | PB12         |
|  PG14  |   ETH_RMII_TXD1       | 11 | PB13         |
|  PC4   |   ETH_RMII_RXD0       | 11 |              |
|  PC5   |   ETH_RMII_RXD1       | 11 |              |
|  PG2   |   ETH_RMII_RXER       |  0 |              |
|  PA7   |   ETH_RMII_CRS_DV     | 11 |              |
|  PC1   |   ETH_RMII_MDC        | 11 |              |
|  PA2   |   ETH_RMII_MDIO       | 11 |              |
|  PA1   |   ETH_RMII_REFCLK     | 11 |              |  
|        |   nRST                |    |              | 
|        |   XTAL1/CLKIN         |    |              |



Configuration
-------------

The configuration consists of many steps:

1. Pin configuration
2. Clock configuration
3. Interrupt configuration
4. MAC configuration
5. PHY configuration
6. DMA configuration
7. ????


The ETH interface uses information stored in main memory and registered in the ETH registers
to transmit or receive information. These information are stored in descriptors and they can have 
two formats. One called Normal (shorter) and other called Enhanced (bigger), that includes
PTP (precision time protocol) data. The format (normal or enhanced) is specified on bit EDFE of the
DMABMR register. The Enhanced descriptor must be used when the Checksum offload is active 
(bit IPCO of MACCR register) or the PTP protocol is active (bit TSE of PTPTSCR register)

There are differences between descriptors for transmiting data and receiving data.


Format of TX Normal Descriptors   

         +------------------------------------------------------------------------+
    +0   | OWN | CTRL | TTSE | ... | CTRL | ... | TTSS | Status                   |
         +------------------------------------------------------------------------+
    +1   | ... | Buffer 2 size  | ...   | Buffer 1 size                           |
         +------------------------------------------------------------------------+
    +2   |                Buffer 1 Address                                        |
         +------------------------------------------------------------------------+
    +3   |           Buffer 2 Address or Next Descriptor Address                  |
         +------------------------------------------------------------------------+

Format of TX Enhanced Descriptors   

         +------------------------------------------------------------------------+
    +0   | OWN | CTRL | TTSE | ... | CTRL | ... | TTSS | Status                   |
         +------------------------------------------------------------------------+
    +1   | ... | Buffer 2 size  | ...   | Buffer 1 size                           |
         +------------------------------------------------------------------------+
    +2   |                Buffer 1 Address                                        |
         +------------------------------------------------------------------------+
    +3   |           Buffer 2 Address or Next Descriptor Address                  |
         +------------------------------------------------------------------------+
    +4   |                  Reserved                                              |
         +------------------------------------------------------------------------+
    +5   |                  Reserved                                              |
         +------------------------------------------------------------------------+
    +6   |               Time stamp low                                           |
         +------------------------------------------------------------------------+
    +7   |               Time stamp low                                           |
         +------------------------------------------------------------------------+

Format of RX Normal Descriptors   

         +------------------------------------------------------------------------+
    +0   | OWN |                            Status                                |
         +------------------------------------------------------------------------+
    +1   | CTRL | ... | Buffer 2 size | CTRL | ... | Buffer 1 size                |
         +------------------------------------------------------------------------+
    +2   |                Buffer 1 Address                                        |
         +------------------------------------------------------------------------+
    +3   |           Buffer 2 Address or Next Descriptor Address                  |
         +------------------------------------------------------------------------+


Format of RX Enhanced Descriptors   

         +------------------------------------------------------------------------+
    +0   | OWN |                            Status                                |
         +------------------------------------------------------------------------+
    +1   | CTRL | ... | Buffer 2 size | CTRL | ... | Buffer 1 size                |
         +------------------------------------------------------------------------+
    +2   |                Buffer 1 Address                                        |
         +------------------------------------------------------------------------+
    +3   |           Buffer 2 Address or Next Descriptor Address                  |
         +------------------------------------------------------------------------+
    +4   |                      Extended Status                                   |
         +------------------------------------------------------------------------+
    +5   |                       Reserved                                         |
         +------------------------------------------------------------------------+
    +6   |               Time stamp low                                           |
         +------------------------------------------------------------------------+
    +7   |               Time stamp low                                           |
         +------------------------------------------------------------------------+



The ETH controller can use the descriptores in

* a ring structure or
* a chain structure

In the ring structure the descriptors are contiguous and each one can pointer to two buffer area. 
The last one must have the TER bit set for TX buffers or the RER for RX buffers.
In the chain structure, one descriptor uses the field for the second buffer area to point to the
 next descriptor in chain. To signal it, the TCH bit must be set.

It is possible to have a frame larger than the Ethernet maximum packet size (1542). This can be
done by setting the FS bit in the first descriptor and the LS bit in the last one. Each buffer 
must contain a complete Ethernet frame, excluding preamble, pad bytes and FCS (Frame check sequence)
 fields.
 
> NOTE: The ETH DMA controller only updates the first word. It never changes the others, specially,
it never changes the size fields. The size of received data must be obtained in the FL fields in the
first word of the RX descriptors.

Transmission
------------

The ETH interface checks the OWN bit (bit 31 of the first word). If it is zero, the descriptor
and corresponding buffer is controlled by the CPU. If it is one, the descriptor and corresponding
buffer is controlled the the DMA unit, part of the ETH controller.

> It is possible to work in OSF mode. This means the processing of a second frame can start before
the status of the first frame is obtained.



Reception
---------
















The LwIP library
----------------

The Lightweight IP Stack (LwIP) is a open source library that implements the TCP/IP stack for embedded systems. It was originally developed by Adam Dunkels at the Swedish Institute of Computer Science.

It works on 8-bit, 16-bit and 32-bit processors. It is highly customizable.

It is a layered implementation with two higher level APIs.

    +-----------------------------------------------+
    |                 Application                   |
    |---------------------------+-------------------|
    |   Netconn API       |   Socket API            |
    |---------------------------+-------------------|
    |            Low level (raw) core               |
    |-----------------------------------------------|
    |                Device driver                  |
    |-----------------------------------------------|
    |                  Hardware                     |
    +-----------------------------------------------+
    

There are three different APIs.

    +------------------------------------------------+
    |               Socket API                       |
    |       +----------------------------------------+
    |       |           Netconn API                  |
    |       |              +-------------------------|
    |       |              |       Raw API           |
    +-------+--------------+-------------------------+


There are different modes and sub-modes.

- Bare metal systems running RAW API
* Kernel based systems using threads using Netconn API or Socket API

Bare metal systems can use an approach using callbacks or a call in the *Mainloop*.

> NOTE: lwIP is NOT re-entrant!!!!

Bare metal modes are enable by the NO_SYS=1 compiler symbol and only the *raw* API can be used.

When using callbacks, some lwIP functions can be called in a IRQ context and so block interrupts for
a long time. 

When using the call in the *Mainloop*, the *sys\_check\_timeouts* must be called
 periodically from the main loop and lwIP must never be called from an interrupt routine.
 
When using threads, the raw API must be called from only one thread. The sequential (netconn and 
socket) API can be called from other threads. In this case, lwIP must never be called from an 
interrupt routine.

### Contents of lwIP package

The main source is in the *lwip* folder.

    lwip/src
    lwip/src/api - the Netconn API, Socket API, and the tcpip thread
    lwip/src/core - core code: DHCP, TCP, UDP, and support code (memory, netif, etc)
    lwip/src/core/ipv4 - IPv4, ICMPv4
    lwip/src/core/ipv6 - IPv6, ICMPv6
    lwip/src/core/snmp - SNMP
    lwip/src/include - all headers and includes
    lwip/src/netif - ARP and sample Ethernet driver
    lwip/src/netif/ppp - PPP

Additional files, like port must be in a *contrib* folder.

    contrib/PORT/arch/{cc.h,sys_arch.h}
    contrib/PORT/{lwiops.h,ethernetif.c}
    contrib/PORT/*.[ch]

### Bare metal implementation

Step to build a bare metal application using call in the mainloop.

1. Create contrib/stm32f74xx/arch/cc.h.
2. Create contrib/stm32f74xx/arch/sys_arch.h
3. Create the contrib/stm32f74xx/lwipopts.h
4. Create your network driver.
5. Create your sys_now function to obtain a timestamp.
6. Create your main function.
7. Create a makefile that compiles your driver, the lwIP files and the rest of your application.

A detailed explanation can be found in *Porting For Bare Metal* [7]

It needs a *sys_now* function, that returns a timestamp with a resolution of X ms.

Typically, a *ethernetif.c* file must be developed that implements the following functions:

* myif_init
* myif_link_output
* myif_output
* myif_input

There is a skeleton ethernetif.c that offers an outline of the device driver. 

In the *netif* structure, there are members, that, generally, must be initialized to point to the 
functions above.

| Pointer           |      Function to be pointed        |
|-------------------|------------------------------------|
| netif.linkoutput  | myif_link_output                   |
| netif.output      | myif_output or etharp_output       |
| netif.input       | myif_input (Do not initialize)     |


The following *netif* structure variables should be set during initialization:

* my_netif->state - Point this to any internal structure that you would like to keep track of for the "state" of your interface. This could include status information, statistics, or anything else.
* my_netif->hwaddr_len - The number of bytes in the link address (e.g., MAC address for Ethernet)
* my_netif->hwaddr[] - The hardware address itself.
* my_netif->mtu - The MTU (maximum transmission unit) for the interface. This defines the maximum sized packet that may be sent on the link, including headers. If an application attempts to send a packet larger than this, lwIP will split the packet up into pieces of this size.
* my_netif->name[2] - Two-character name, like "en" for Ethernet. This can be used to get a netif by name, via netif_find
* my_netif->num - An optional number to indicate the number in this "series", if there are multiple netif's of the same type. For example, perhaps there are two interfaces named "en"; they can be differentiated by this number. This is completely optional and largely unused by lwIP.
* my_netif->output - Set this equal to the myif_output described above.
* my_netif->link_output - Set this equal to the myif_link_output described above.
* my_netif->input - Do not set this. This is set by netif_add.
* my_netif->flags - Should set any of the following flags that apply to this interface.
        NETIF_FLAG_BROADCAST - If this interface can transmit broadcast packets.
        NETIF_FLAG_POINTTOPOINT - If this interface is on an end of a PPP connection.
        NETIF_FLAG_ETHARP - If this interface uses ARP (see the section below for more information).
        NETIF_FLAG_LINK_UP - Set this flag when the link is established (see note below for more capability).

The netif struct (defined in lwip/src/include/netif.h) has many more parameters, but many of
them are optional.

    struct netif {
        ...
        ip_addr_t ip_addr;              // IPv4 address (in network order!)
        ip_addr_t netmask;              // IPv4 network mask
        ip_addr_t gw;                   // IPv4 gateway
        
        ip_addr_t ip6_addr[LWIP_IPV6_NUM_ADDRESSES];    // IPv6 network addresses
        u8_t ip6_addr_state[LWIP_IPV6_NUM_ADDRESSES];   // status of network addresses
        ...
        netif_input_fn input;         // called to pass a packet up the TCP/IP stack
        netif_output_fn output;       // For IPv4 only. Usually etharp_output
        netif_linkoutput_fn linkoutput;// Called to send a packet on the interface
        netif_output_ip6_fn output_ip6;// For IPv6 only. Usually ethip6_output
        netif_status_callback_fn status_callback; // Callend when set up or down
        netif_status_callback_fn link_callback; // Called when link set up or down
        netif_status_callback_fn remove_callback;// Called when if removed

        void *state;  // Used by device driver to store information for the device

        const char*  hostname;    // the hostname for this netif, NULL is a valid value

        u8_t hwaddr[NETIF_MAX_HWADDR_LEN];        // hardware address of this interface
        u8_t hwaddr_len;  //number of bytes used in hwaddr */
        u8_t flags;
        ...
        char name[2]; // descriptive abbreviation
        u8_t num;
        u8_t ip6_autoconfig_enabled;      // netif enabled for IPv6 autoconfiguration
        ...
    };


Configuring a project with lwIP
------------------------------

1. Download lwip and unpack it into the project folder. Name it *lwip*.
2. Put the device driver in the $(LWIP)/contrib/STM32F74xx folder.
3. Add the folder lwip/src/include/lwip to the include path

    PROJCFLAGS= -I $(LWIP)/lwip/src/include/                \
                -I $(LWIP)/contrib/STM32F74xx/arch         \
                -I $(LWIP)/contrib/STM32F74xx/             \
                -DNOSYS=1

  


Annex A - LwIP mainloop mode
----------------------------

    void
    eth_mac_irq()
    {
        /* Service MAC IRQ here */
        /* Allocate pbuf from pool (avoid using heap in interrupts) */
        struct pbuf* p = pbuf_alloc(PBUF_RAW, eth_data_count, PBUF_POOL);
        if(p != NULL) {
            /* Copy ethernet frame into pbuf */
            pbuf_take(p, eth_data, eth_data_count);
            /* Put in a queue which is processed in main loop */
            if(!queue_try_put(&queue, p)) {
                /* queue is full -> packet loss */
                pbuf_free(p);
            }
        }
    }
    static err_t 
    netif_output(struct netif *netif, struct pbuf *p)
    {
        LINK_STATS_INC(link.xmit);
        /* Update SNMP stats (only if you use SNMP) */
        MIB2_STATS_NETIF_ADD(netif, ifoutoctets, p->tot_len);

        int unicast = ((p->payload[0] & 0x01) == 0);

        if (unicast) {
            MIB2_STATS_NETIF_INC(netif, ifoutucastpkts);
        } else {
            MIB2_STATS_NETIF_INC(netif, ifoutnucastpkts);
        }
        lock_interrupts();
        pbuf_copy_partial(p, mac_send_buffer, p->tot_len, 0);

        /* Start MAC transmit here */
        unlock_interrupts();
        return ERR_OK;
    }

    static void 
    netif_status_callback(struct netif *netif)
    {
        printf("netif status changed %s\n", ip4addr_ntoa(netif_ip4_addr(netif)));
    }
    
    static err_t 
    netif_init(struct netif *netif)
    {
        netif->linkoutput = netif_output;
        netif->output     = etharp_output;
        netif->output_ip6 = ethip6_output;
        netif->mtu        = ETHERNET_MTU;
        netif->flags      = NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHARP | NETIF_FLAG_ETHERNET 
                          | NETIF_FLAG_IGMP | NETIF_FLAG_MLD6;
        MIB2_INIT_NETIF(netif, snmp_ifType_ethernet_csmacd, 100000000);
        
        SMEMCPY(netif->hwaddr, your_mac_address_goes_here, ETH_HWADDR_LEN);
        netif->hwaddr_len = ETH_HWADDR_LEN;
        
        return ERR_OK;
    }
    void 
    main(void)
    {
        struct netif netif;
        
        lwip_init();
        
        netif_add(&netif, IP4_ADDR_ANY, IP4_ADDR_ANY, IP4_ADDR_ANY, NULL, netif_init, netif_input);
        netif.name[0] = 'e';
        netif.name[1] = '0';
        netif_create_ip6_linklocal_address(&netif, 1);
        netif.ip6_autoconfig_enabled = 1;
        netif_set_status_callback(&netif, netif_status_callback);
        netif_set_default(&netif);
        netif_set_up(&netif);

        /* Start DHCP and HTTPD */
        dhcp_start(&netif );
        httpd_init();
        
        /* Main loop */
        while(1) {
            /* Check link state, e.g. via MDIO communication with PHY */
            if(link_state_changed()) {
                if(link_is_up()) {
                    netif_set_link_up(&netif);
                } else {
                    netif_set_link_down(&netif);
                }
            }
        /* Check for received frames, feed them to lwIP */
            lock_interrupts();
            struct pbuf* p = queue_try_get(&queue);
            unlock_interrupts();
            if(p != NULL) {
                LINK_STATS_INC(link.recv);

                /* Update SNMP stats (only if you use SNMP) */
                MIB2_STATS_NETIF_ADD(netif, ifinoctets, p->tot_len);
                int unicast = ((p->payload[0] & 0x01) == 0);
                if (unicast) {
                    MIB2_STATS_NETIF_INC(netif, ifinucastpkts);
                } else {
                    MIB2_STATS_NETIF_INC(netif, ifinnucastpkts);
                }
                    if(netif.input(p, &netif) != ERR_OK) {
                    pbuf_free(p);
                    }
            }

            /* Cyclic lwIP timers check */
            sys_check_timeouts();

            /* your application goes here */
        }
    }


Annex B - Other TCP/IP stacks
-----------------------------


|  Name                   |     URL                                            |
|-------------------------|----------------------------------------------------|
|  uIP                    | https://github.com/adamdunkels/uip                 |
|  lwIP                   | https://savannah.nongnu.org/projects/lwip/         |
|  fNET                   | https://fnet.sourceforge.io/                       |
|  Bentham's TCP/IP stack | Book TCP/IP Lean: Web servers for embedded systems |

    
References
----------

1. [STM32F75xxx and STM32F74xxx advanced Arm ® -based 32-bit MCUs Reference Manual - RM0385](https://www.st.com/resource/en/reference_manual/dm00124865-stm32f75xxx-and-stm32f74xxx-advanced-arm-based-32-bit-mcus-stmicroelectronics.pdf)
2. [STM32F746NG Data sheet](https://www.st.com/resource/en/datasheet/stm32f746ng.pdf)
3. [UMN1713 - User Manual - Developing applications on STM32Cube with LwIP TCP/IP stack](https://www.st.com/resource/en/user_manual/dm00103685-developing-applications-on-stm32cube-with-lwip-tcpip-stack-stmicroelectronics.pdf)
4. [RMII TM Specification](http://ebook.pldworld.com/_eBook/-Telecommunications,Networks-/TCPIP/RMII/rmii_rev12.pdf)
5. [Small Footprint RMII 10/100 Ethernet Transceiver with HP Auto-MDIX and flexPWR ® Technology](https://ww1.microchip.com/downloads/en/DeviceDoc/DS_LAN8742_00001989A.pdf)
6. [LwIP](https://www.nongnu.org/lwip/)
7. [LwIP Wiki](https://lwip.fandom.com/wiki/LwIP_Wiki)
8. [Porting For Bare Metal](https://lwip.fandom.com/wiki/Porting_For_Bare_Metal)
9. [AN3966 LwIP TCP/IP stack demonstration for STM32F4x7](https://www.st.com/resource/en/application_note/dm00036052-lwip-tcpip-stack-demonstration-for-stm32f4x7-microcontrollers-stmicroelectronics.pdf)
10. [Porting Drivers to the STM32F7](https://cwiki.apache.org/confluence/display/NUTTX/Porting+Drivers+to+the+STM32+F7)
11. [AN4839-Level 1 cache on STM32F7 Series and STM32H7 Series](https://www.st.com/resource/en/application_note/dm00272913-level-1-cache-on-stm32f7-series-and-stm32h7-series-stmicroelectronics.pdf)
12. [AN4667-STM32F7 Series system architecture and performance](https://www.st.com/resource/en/application_note/dm00169764-stm32f7-series-system-architecture-and-performance-stmicroelectronics.pdf)
13. [Web Server with a STM32F746](https://github.com/khoih-prog/EthernetWebServer_STM32)
14. [Level 1 cache on STM32F7 Series and STM32H7 Series](http://www.st.com/content/ccc/resource/technical/document/application_note/group0/08/dd/25/9c/4d/83/43/12/DM00272913/files/DM00272913.pdf/jcr:content/translations/en.DM00272913.pdf)
