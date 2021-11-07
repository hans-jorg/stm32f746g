#ifndef ETH_H
#define ETH_H
/**
 * @file    eth.h
 *
 * @date    10/05/2021
 * @author  Hans
 */

#include "stm32f746xx.h"
#include "system_stm32f746.h"

/**
 * @brief By setting this symbol, no buffer is statically allocated.
 *        The buffers must be allocated and then setting using
 *        ETH_SetBuffers (TBD!)
 * 
 */

//#define ETH_ALLOCATE_BUFFERS_DYNAMICALLY  (1)
/*
 * @brief  Compilation flag
 *
 * @note   When using nosys=1, there are two options:
 *         1. Use callbacks during interrupts
 *         2  Do all processing in the main loop
 */
#define ETH_USE_INTERRUPTS 1

/* *
 * @brief  Periodic process
 *
 * @!note  Must be called every 1 ms
 */

//extern void ETH_PeriodicProcess(void);

/**
 * @brief  Descriptor structure for ETH DMA Transfers
 *
 * @note   There are two versions: the standard version and the enhanced one.
 *         The enhanced version has additional fields to support PTP.
 *
 * @note   There are differences between the fields in the RX and TX descriptions. Specially
 *         the chain flag. In TX descriptors, it is in the Status word. In RX descriptors in
 *         the ControlBufferSize. This was the reason for not using bitfields.
 *
 */
typedef struct {
    volatile uint32_t   Status;                 /*!< Status */
    volatile uint32_t   ControlBufferSize;      /*!< Control flags and lengths */
    volatile uint32_t   Buffer1Addr;            /*!< Buffer 1 address */
    volatile uint32_t   Buffer2NextDescAddr;    /*!< Buffer 2 address/Next */
    /* Fields for enhanced descriptor format (includes PTP Info) */
             uint32_t   ExtendedStatus;         /*!< Extended status for PTP  */
             uint32_t   Reserved1;              /*!< Reserved */
             uint32_t   TimeStampLow;           /*!< Time Stamp Low  */
             uint32_t   TimeStampHigh;          /*!< Time Stamp High */
} ETH_DMADescriptor;

/**
 * @brief   Symbols for fields in ETH_DMADescriptor
 */
#define ETH_DMADESCRIPTOR_STATUS_OWN        (1<<31)

/**
  * @brief  Received Frame Information structure definition
   *
   * @note    From stm32f7xx_hal_eth.h (DMARxFramInfos)
   *
   * @note    Uses DMADescriptor
  */
typedef struct {
    ETH_DMADescriptor  *FirstSegmentDesc;   /*!< First Segment Rx Desc */
    ETH_DMADescriptor  *LastSegmentDesc;    /*!< Last Segment Rx Desc */
    uint32_t            SegmentCount;       /*!< Segment count */
    uint32_t            FrameLength;        /*!< Frame length */
//    uint32_t            buffer;           /*!< Frame buffer */
} ETH_DMAFrameInfo;

/**
 * @brief   Received Frame Info
 * 
 */
extern ETH_DMAFrameInfo  ETH_RXFrameInfo;

/**
 * @brief Info about buffer size
 */
///@{
#define ETH_MAX_PACKET_SIZE         (1524)  /*!< HEADER+EXTRA+VLAN+MAX_PAYLOAD+CRC */
#define ETH_HEADER                  (4)     /*!< Destination(6),Source(6),Length(2)*/
#define ETH_CRC                     (4)     /*!< Ethernet CRC */
#define ETH_EXTRA                   (2)     /*!< Extra bytes in some cases */
#define ETH_VLAN_TAG                (4)     /*!< Optional 802.1q VLAN Tag */
#define ETH_MIN_ETH_PAYLOAD         (46)    /*!< Minimum Ethernet payload size */
#define ETH_MAX_ETH_PAYLOAD         (1500)  /*!< Maximum Ethernet payload size */
#define ETH_JUMBO_FRAME_PAYLOAD     (9000)  /*!< Jumbo frame payload size */
///@}

/// Maximum transmission unit
#define ETH_MTU                     ETH_MAX_ETH_PAYLOAD
/**
 * @brief TX and RX BUFFER size and quantity
 */
///@{
#ifndef ETH_TXBUFFER_COUNT
    #define ETH_TXBUFFER_COUNT      (4)
#endif
#ifndef ETH_RXBUFFER_COUNT
    #define ETH_RXBUFFER_COUNT      (4)
#endif
#define ETH_TXBUFFER_SIZE           ETH_MAX_PACKET_SIZE
#define ETH_RXBUFFER_SIZE           ETH_MAX_PACKET_SIZE
///@}



/**
 * lwIP examples requires this definition
 */
#define ETHERNET_MTU   ETH_MAX_ETH_PAYLOAD
/**
 * @brief   RX and TX descriptors
 */
///@{
extern ETH_DMADescriptor *ETH_TXDescriptors;
extern ETH_DMADescriptor *ETH_RXDescriptors;
///@}

/**
 * @brief Pointer to callback functions
 */

struct ETH_Callbacks_s {
    void    (*FrameReceived)(unsigned);
    void    (*FrameTransmitted)(unsigned);
    void    (*ErrorDetected)(unsigned);
    void    (*LinkStatusChanged)(unsigned);
};

extern struct ETH_Callbacks_s ETH_Callbacks;


/**
 * @brief Parameter for ETH_RegisterCallback
 */
///@{
#define ETH_CALLBACK_FRAMERECEIVED              1
#define ETH_CALLBACK_FRAMETRANSMITTED           2
#define ETH_CALLBACK_ERRORDETECTED              3
#define ETH_CALLBACK_LINKSTATUSCHANGED          4
///@}

/**
 * @brief Clock flags used to enable/disable clock
 */
///@{
#define ETH_CLOCK_PTP                           0x0001
#define ETH_CLOCK_MACRX                         0x0002
#define ETH_CLOCK_MACTX                         0x0004
#define ETH_CLOCK_MAC                           0x0008
#define ETH_CLOCK_ALL                           0x000F
///@}

/**
 * @brief   Codification of link info
 * 
 */
#define ETH_LINKINFO_100BASET_FULLDUPLEX        0x6
#define ETH_LINKINFO_100BASET_HALFDUPLEX        0x2
#define ETH_LINKINFO_10BASET_FULLDUPLEX         0x5
#define ETH_LINKINFO_10BASET_HALFDUPLEX         0x16

// Initialization functions
void ETH_Init(void);
#ifdef ETH_ALLOCATE_BUFFERS_DYNAMICALLY
void ETH_InitTXDescriptors(ETH_DMADescriptor *desc, int count, uint8_t *area);
void ETH_InitRXDescriptors(ETH_DMADescriptor *desc, int count, uint8_t *area);
#endif

// Operation function
int  ETH_TransmitFrame(ETH_DMADescriptor *desc, unsigned size);
int  ETH_ReceiveFrame(ETH_DMAFrameInfo *RxFrameInfo);
int  ETH_CheckReception(void);

// Control functions (Enable/Disable)
void ETH_Start(void);
void ETH_Stop(void);
void ETH_EnableClock(uint32_t which);
void ETH_DisableClock(uint32_t which);

// Callback routines 
void ETH_RegisterCallback(unsigned, void (*)(unsigned));

// MAC Address Management functions
void ETH_SetMACAddress(uint64_t macaddr);
void ETH_SetMACAddressN(uint32_t n, uint64_t macaddr, uint32_t mbc);
void ETH_GetMACAddress(uint8_t macaddr[6]);
void ETH_GetMACAddressAsVector(uint8_t macaddr[6]);
void ETH_GetMACAddressAsNetworkOrderedVector(uint8_t macaddr[6]);

// Link status function
int  ETH_IsLinkUp(void);
int  ETH_IsConnected(void);

// Reconfigure Link 
int  ETH_UpdateLinkStatus(void);
unsigned ETH_GetLinkStatus(void);
void ETH_PHYRegisterDump(void);
int ETH_GetLinkInfo(void);
char const * ETH_GetLinkInfoString(void);
#endif

void ETH_DumpDescriptors(uint32_t); // which = 1: TX, 2:RX, 3:both