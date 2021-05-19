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

/*
 * @brief  Compilation flag
 *
 * @note   When using nosys=1, there are two options:
 *         1. Use callbacks during interrupts
 *         2  Do all processing in the main loop
 */
#define ETH_USE_INTERRUPTS

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
    uint32_t   Status;                      /*!< Status */
    uint32_t   ControlBufferSize;           /*!< Control, lengths of buffer 1 and 2 */
    uint32_t   Buffer1Addr;                 /*!< Buffer 1 address */
    uint32_t   Buffer2NextDescAddr;         /*!< Buffer 2 address/Nex descriptor address */
    /* Fields for enhanced descriptor format (includes PTP Info) */
    uint32_t   ExtendedStatus;              /*!< Extended status for PTP receive descriptor */
    uint32_t   Reserved1;                   /*!< Reserved */
    uint32_t   TimeStampLow;                /*!< Time Stamp Low  */
    uint32_t   TimeStampHigh;               /*!< Time Stamp High */
} ETH_DMADescriptor;


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

/**
 * @brief TX and RX BUFFER size and quantity
 */
///@{
#define ETH_TXBUFFER_SIZE           ETH_MAX_PACKET_SIZE
#define ETH_TXBUFFER_COUNT          5
#define ETH_RXBUFFER_SIZE           ETH_MAX_PACKET_SIZE
#define ETH_RXBUFFER_COUNT          5
///@}


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
};

extern struct ETH_Callbacks_s ETH_Callbacks;


/**
 * @brief Parameter for ETH_RegisterCallback
 */
///@{
#define ETH_CALLBACK_FRAMERECEIVED              1
#define ETH_CALLBACK_FRAMETRANSMITTED           2
#define ETH_CALLBACK_ERRORDETECTED              3
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

void ETH_Init(void);
void ETH_EnableClock(uint32_t which);
void ETH_DisableClock(uint32_t which);
void ETH_InitializeDescriptorsTX(ETH_DMADescriptor *desc, int count, char *area);
void ETH_InitializeDescriptorsRX(ETH_DMADescriptor *desc, int count, char *area);
void ETH_Start(void);
void ETH_Stop(void);
int  ETH_TransmitFrame(unsigned size);
void ETH_EnableTransmissionDMA(void);
void ETH_DisableTransmissionDMA(void);
void ETH_EnableReceptionDMA(void);
void ETH_DisableReceptionDMA(void);
void ETH_EnableTransmissionMAC(void);
void ETH_DisableTransmissionMAC(void);
void ETH_EnableReceptionMAC(void);
void ETH_DisableReceptionMAC(void);
void ETH_IRQHandler(void);

void ETH_RegisterCallback(unsigned, void (*)(unsigned));

#endif

