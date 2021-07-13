/**
 * @file    eth.c
 *
 * @note    ETH interface low level routines
 *
 *
 * @date    10/05/2021
 * @author  Hans
 *
 */


#include <stdarg.h>
#include <stdio.h>

#include "stm32f746xx.h"
#include "system_stm32f746.h"
#include "gpio.h"
#include "eth.h"

#include "debugmessages.h"

/**
 * @brief   Which routine to use for configuring pin
 */
//#define ETH_USEGPIOFORCONFIGURATION     1




/**
 * @brief   Round macro. It returns a value equal or greater than N, that is
 *          a multiple of M.
 * 
 * @param   N: value to be rounded
 * @param   M: rounding parameter, typically, sizeof(type)
 */

#define ROUND(N,M)  ( ( ((N)+(M)-1)/(M) )*(M) )


/**
 * Function prototypes (forward referenced);
 */
static void ETH_FlushTXFIFO(void);
static void ETH_UpdateConfigStatus(void);
static void ETH_ManualConfig(void);


/**
 * @brief   Network configuration
 * 
 * @note    AUTONEGOTIATE overrides the other parameters
 *          When Auto Negotiation does not work, use remaining parameters
 */
///@{
#define ETH_CONFIG_AUTONEGOTIATE (1)
#define ETH_CONFIG_100BASET      (2)
#define ETH_CONFIG_10BASET       (4)
#define ETH_CONFIG_FULLDUPLEX    (8)
#define ETH_CONFIG_HALFDUPLEX    (16)
///@}
static const uint32_t ETH_Config =  0//ETH_CONFIG_AUTONEGOTIATE
// Parameters below only used when Auto Negotiation does not work or not set
                                    |ETH_CONFIG_100BASET
                                    |ETH_CONFIG_FULLDUPLEX;

/**
 * @brief   Ethernet controller status
 */
///@{
#define ETH_STATE_RESET         (0)
#define ETH_STATE_ERROR         (1)
#define ETH_STATE_READY         (2)

static uint32_t ETH_State = ETH_STATE_RESET;
///@}

/**
 * @brief   Ethernet status
 */
///@{

#define ETH_CONFIGSTATUS_LINKDOWN     (1)
#define ETH_CONFIGSTATUS_LINKUP       (2)
#define ETH_CONFIGSTATUS_100BASET     (4)
#define ETH_CONFIGSTATUS_10BASET      (8)
#define ETH_CONFIGSTATUS_FULLDUPLEX   (16)
#define ETH_CONFIGSTATUS_HALFDUPLEX   (32)

static uint32_t ETH_ConfigStatus = ETH_CONFIGSTATUS_LINKDOWN;
///@}


// 48 bit MAC address (must have 12 hexadecimal digits))
#ifndef ETH_MACADDRESS
#define ETH_MACADDRESS 0x2cf05e0bbabdL
#endif


/**
 * @brief   Timing parameters
 * 
 */
///@{
#define ETH_DELAY_AFTERREGISTERWRITE    1000
#define ETH_DELAY_AFTERFLUSH            10
#define ETH_DELAY_AFTERMAC              10
#define ETH_DELAY_AFTERRESET            1000
#define ETH_DELAY_AFTERAUTONEGOTIATION  1000
#define ETH_DELAY_AFTERCONFIG           1000
#define ETH_DELAY_BETWEENTESTS          1000
///@}

/**
 * @brief   Timeout configuration
 */
///@{
#define ETH_RETRIES_AUTONEGOTIATION     100
#define ETH_RETRIES_LINK                1000
//@}
/**
 * @brief   ETH IRQ Configuration
 */

#define ETH_IRQLevel                            (5)

/**
 * @brief   RX and TX descriptors
 * 
 * @note    All sizes are rounded to uint32_t sizes, i.e. multiple of 4.
 */
///@{
#define ETH_TXBUFFERSIZE_INT32UNITS ROUND(ETH_TXBUFFER_SIZE,sizeof(uint32_t))
#define ETH_RXBUFFERSIZE_INT32UNITS ROUND(ETH_RXBUFFER_SIZE,sizeof(uint32_t))
#define ETH_TXBUFFERSIZE_INT8UNITS (ETH_TXBUFFERSIZE_INT32UNITS*sizeof(uint32_t))
#define ETH_RXBUFFERSIZE_INT8UNITS (ETH_RXBUFFERSIZE_INT32UNITS*sizeof(uint32_t))

// where to allocate
#define EXTRAM     __attribute__ ((section(".sdram")))
//#define EXTRAM
    
#ifdef ETH_ALLOCATE_BUFFERS_DYNAMICALLY
    // These pointers must be initialized thru a call to ETH_SetBuffers
    // They must be in a non cacheable area
    ETH_DMADescriptor               *ETH_TXDescriptors = 0;
    ETH_DMADescriptor               *ETH_RXDescriptors = 0;
    static uint32_t                 *ETH_Area;
    static uint8_t                  *txbuffer = 0;
    static uint8_t                  *rxbuffer = 0;
#else 
    EXTRAM static ETH_DMADescriptor  ETH_TXDesc[ETH_TXBUFFER_COUNT] = { 0 };
    EXTRAM static ETH_DMADescriptor  ETH_RXDesc[ETH_RXBUFFER_COUNT] = { 0 };
    ETH_DMADescriptor               *ETH_TXDescriptors = ETH_TXDesc;
    ETH_DMADescriptor               *ETH_RXDescriptors = ETH_RXDesc;
// An alternative is to define them as an array of 32-bit integers for alignment
#define TXBUFFERSIZE (ETH_TXBUFFERSIZE_INT8UNITS*ETH_TXBUFFER_COUNT)
#define RXBUFFERSIZE (ETH_RXBUFFERSIZE_INT8UNITS*ETH_RXBUFFER_COUNT)

    EXTRAM static uint8_t          txbuffer[TXBUFFERSIZE] 
                                   __attribute((aligned(sizeof(uint32_t))));
    EXTRAM static uint8_t          rxbuffer[RXBUFFERSIZE]
                                   __attribute((aligned(sizeof(uint32_t))));
#endif

///@}

/**
 * @brief   Callbacks
 */
struct ETH_Callbacks_s ETH_Callbacks       = {0};

/**
 * @brief   Symbols for fields in the DMA descriptors
 */

///@{
// This bit is the highest order of word 0 of both descriptors
#define ETH_TXDESC_OWN                   (1<<31)
#define ETH_RXDESC_OWN                   (1<<31)

//** For TX Descriptors **
// These fields are in word 0
#define ETH_TXDESC_CHAINED               (1<<20)
#define ETH_TXDESC_ENDOFRING             (1<<21)
#define ETH_TXDESC_CIC                   (3<<22)
#define ETH_TXDESC_FIRST                 (1<<28)
#define ETH_TXDESC_LAST                  (1<<29)
#define ETH_TXDESC_BUFFER1SIZE_MSK       (0x1FFF)
#define ETH_TXDESC_BUFFER2SIZE_MSK       (0x1FFF0000)


//** For RX Descriptors **
// These fields are in Status word (0)
#define ETH_RXDESC_FRAMELENGTH_POS       (16)
#define ETH_RXDESC_FRAMELENGTH_MASK      (0x3FFF0000)
#define ETH_RXDESC_ERROR_SUMMARY         (1<<15)
#define ETH_RXDESC_ERROR_DESCRIPTOR      (1<<14)
#define ETH_RXDESC_ERROR_LENGTH          (1<<12)
#define ETH_RXDESC_ERROR_OVERFLOW        (1<<10)
#define ETH_RXDESC_FIRST                 (1<<9)
#define ETH_RXDESC_LAST                  (1<<8)
#define ETH_RXDESC_ERROR_RECEIVE         (1<<3)
#define ETH_RXDESC_ERROR_CRC             (1<<1)
// These fields are in ControlBufferSize word (1)
#define ETH_RXDESC_BUFFER1SIZE_MASK      (0x1FFF)
#define ETH_RXDESC_BUFFER1SIZE_POS       (0)
#define ETH_RXDESC_BUFFER2SIZE_MASK      (0x1FFF0000)
#define ETH_RXDESC_BUFFER2SIZE_POS       (16)
#define ETH_RXDESC_ENDOFRING             (1<<15)
#define ETH_RXDESC_CHAINED               (1<<14)
#define ETH_RXBUFFER_DIC                 (1<<31)

///@}

/**
 * @brief   Defines for configure PHY
 *
 * @note LAN8742 PHY register description
 *
 *|  # |        | Description                                |  Group          |
 *|----|--------|--------------------------------------------|-----------------|
 *|  0 | BCR    | Basic Control Register                     | Basic           |
 *|  1 | BSR    | Basic Status Register                      | Basic           |
 *|  2 | ID1R   | PHY Identifier 1 Register                  | Extended        |
 *|  3 | ID2R   | PHY Identifier 2 Register                  | Extended        |
 *|  4 | ANAR   | Auto Negotiation Advertisement Register    | Extended        |
 *|  5 | ANLPR  | Auto Negotiation Link Partner Register     | Extended        |
 *|  6 | ANEPR  | Auto Negotiation Expansion Register        | Extended        |
 *|  7 | ANNPTXR| Auto Negotiation Next Page TX Register     | Extended        |
 *|  8 | ANNPRXR| Auto Negotiation Next Page RX Register     | Extended        |
 *| 13 | MMDACR | MMD Access Control Register                | Extended        |
 *| 14 | MMADR  | MMD Access Address/Data Register           | Extended        |
 *| 16 | EDPDR  | EDPD NLP/Crossover TimeRegister            | Vendor-specific |
 *| 17 | MCSR   | Mode Control/Status Register               | Vendor-specific |
 *| 18 | SMR    | Special Modes Register                     | Vendor-specific |
 *| 24 | TDRPDR | TDR Patterns/Delay Control Register        | Vendor-specific |
 *| 25 | TDCSR  | TDR Control/Status Register                | Vendor-specific |
 *| 26 | SECR   | Symbol Error Counter Register              | Vendor-specific |
 *| 27 | SCSIR  | Special Control/Status Indications Register| Vendor-specific |
 *| 28 | CLR    | Cable Length Register                      | Vendor-specific |
 *| 29 | ISFR   | Interrupt Source Flag Register             | Vendor-specific |
 *| 30 | IMR    | Interrupt Mask Register                    | Vendor-specific |
 *| 31 | SCSR   | PHY Special Control/Status Register        | Vendor-specific |
 * 
 * 
 */

/** PHY */
#define ETH_PHY_ADDRESS                         (0)

/** Registers */
///@{
#define ETH_PHY_BCR                             (0)
#define ETH_PHY_BSR                             (1)
#define ETH_PHY_ISFR                           (29)
#define ETH_PHY_IMR                            (30)
#define ETH_PHY_ANA                             (4)
#define ETH_PHY_SCSR                           (31)
///@}


/** Fields of BCR Register */
///@{
#define ETH_PHY_BCR_RESET                       (uint16_t) (0x8000)
#define ETH_PHY_BCR_LOOPBACK                    (uint16_t) (0x4000)
#define ETH_PHY_BCR_SPEED100MHz                 (uint16_t) (0x2000)
#define ETH_PHY_BCR_AUTONEGOTIATIONENABLE       (uint16_t) (0x1000)
#define ETH_PHY_BCR_POWERDOWN                   (uint16_t) (0x0800)
#define ETH_PHY_BCR_ISOLATE                     (uint16_t) (0x0400)
#define ETH_PHY_BCR_AUTONEGOTIATIONRESTART      (uint16_t) (0x0200)
#define ETH_PHY_BCR_FULLDUPLEX                  (uint16_t) (0x0100)
///@}

/* Fields of BSR Register */
///@{
#define ETH_PHY_BSR_100BASET_FULLDUPLEX         (uint16_t) (0x4000)
#define ETH_PHY_BSR_100BASET_HALFDUPLEX         (uint16_t) (0x2000)
#define ETH_PHY_BSR_10BASET_FULLDUPLEX          (uint16_t) (0x1000)
#define ETH_PHY_BSR_10BASET_HALFDUPLEX          (uint16_t) (0x0800)
#define ETH_PHY_BSR_AUTONEGOTIATIONCOMPLETED    (uint16_t) (0x0020)
#define ETH_PHY_BSR_AUTONEGOTEABILITY           (uint16_t) (0x0008)
#define ETH_PHY_BSR_LINKUP                      (uint16_t) (0x0004)
///@}

/* Fields of ISFR Register */
///@{
#define ETH_PHY_ISFR_INT8                       (uint16_t) (0x0100)
#define ETH_PHY_ISFR_INT7                       (uint16_t) (0x0080)
#define ETH_PHY_ISFR_INT6                       (uint16_t) (0x0040)
#define ETH_PHY_ISFR_INT5                       (uint16_t) (0x0020)
#define ETH_PHY_ISFR_INT4                       (uint16_t) (0x0010)
#define ETH_PHY_ISFR_INT3                       (uint16_t) (0x0008)
#define ETH_PHY_ISFR_INT2                       (uint16_t) (0x0004)
#define ETH_PHY_ISFR_INT1                       (uint16_t) (0x0002)
///@}

/* Fields of ISFR Register */
///@{
#define ETH_PHY_IMR_INT8                        (uint16_t) (0x0100)
#define ETH_PHY_IMR_INT7                        (uint16_t) (0x0080)
#define ETH_PHY_IMR_INT6                        (uint16_t) (0x0040)
#define ETH_PHY_IMR_INT5                        (uint16_t) (0x0020)
#define ETH_PHY_IMR_INT4                        (uint16_t) (0x0010)
#define ETH_PHY_IMR_INT3                        (uint16_t) (0x0008)
#define ETH_PHY_IMR_INT2                        (uint16_t) (0x0004)
#define ETH_PHY_IMR_INT1                        (uint16_t) (0x0002)
///@}

/* Fields of ANA Register */
///@{
#define ETH_PHY_ANA_100BASET_FULLDUPLEX         (uint16_t) (0x0100)
#define ETH_PHY_ANA_100BASET                    (uint16_t) (0x0080)
#define ETH_PHY_ANA_10BASET_FULLDUPLEX          (uint16_t) (0x0040)
#define ETH_PHY_ANA_10BASET                     (uint16_t) (0x0020)
#define ETH_PHY_ANA_SELDEFAULT                  (uint16_t) (0x0001)
#define ETH_PHY_ANA_ALL                         (uint16_t) (0x01E1)
///@}

/* Fields of SCSR Register */
///@{
#define ETH_PHY_SCSR_100BASET_FULLDUPLEX        (uint16_t) (0x0018)
#define ETH_PHY_SCSR_100BASET_HALFDUPLEX        (uint16_t) (0x0008)
#define ETH_PHY_SCSR_10BASET_FULLDUPLEX         (uint16_t) (0x0012)
#define ETH_PHY_SCSR_10BASET_HALFDUPLEX         (uint16_t) (0x0002)
#define ETH_PHY_SCSR_SPEED_M                    (uint16_t) (0x001C)
#define ETH_PHY_SCSR_AUTODONE                   (uint16_t) (0x1000)
#define ETH_PHY_SCSR_WRITEONE                   (uint16_t) (0x0040)
///@}




/**
 * @brief   Symbols for fields in MAC Address registers
 * 
 * @note    MAC Address #0 has a different format
 */
///@{
#define ETH_MACADDR_MBC_AE      (1<<31)
#define ETH_MACADDR_MBC_SA      (1<<30)
#define ETH_MACADDR_MBC_BYTE5   (1<<29)
#define ETH_MACADDR_MBC_BYTE4   (1<<28)
#define ETH_MACADDR_MBC_BYTE3   (1<<27)
#define ETH_MACADDR_MBC_BYTE2   (1<<26)
#define ETH_MACADDR_MBC_BYTE1   (1<<25)
#define ETH_MACADDR_MBC_BYTE0   (1<<24)
///@}



#ifdef ETH_ALLOCATE_BUFFERS_DYNAMICALLY
/**
 * @brief   Flags for buffer initialization
 * 
 * @note    ETH_Init must run AFTER buffer initialization
 */
///@{
#define TXBUFFER_INITIALIZED            1
#define RXBUFFER_INITIALIZED            2

static uint8_t buffers_initialized = 0;
///@}
#endif



/**
 * @brief   small simple delay routine
 */

static void delay(int count) {
volatile int c = count;

    while( c-- ) {}

}


////////////////// IRQ Handler /////////////////////////////////////////////////////////////////////

#ifdef ETH_USE_ETH_IRQ
/**
 * @brief   ETH interrupt handler
 */

void ETH_IRQHandler(void) {

    // Check if a frame was received
    if( ETH->DMASR&ETH_DMASR_RS ) {
        /* Call back if defined */
        if( ETH_Callbacks.FrameReceived ) ETH_Callbacks.FrameReceived(0);
        ETH->DMASR = ETH_DMASR_RS;
        /* Set State to Ready */
        ETH_State = ETH_STATE_READY;
    }

    // Check if a frame was transmitted
    if( ETH->DMASR&ETH_DMASR_TS ) {
        /* Call back if defined */
        if( ETH_Callbacks.FrameTransmitted ) ETH_Callbacks.FrameTransmitted(0);
        ETH->DMASR = ETH_DMASR_TS;
        /* Set State to Ready */
        ETH_State = ETH_STATE_READY;
    }

    // ETH DMA Error (Abnormal Interrupt Summary)
    if( ETH->DMASR&ETH_DMASR_AIS ) {
        /* Call back if defined */
        if( ETH_Callbacks.ErrorDetected ) ETH_Callbacks.ErrorDetected(0);
        ETH->DMASR = ETH_DMASR_AIS;
        /* Set State to Ready */
        ETH_State = ETH_STATE_READY;
    }

    // Clear interrupt summary
    ETH->DMASR = ETH_DMASR_NIS;
}
#endif

////////////////// MAC Address Management //////////////////////////////////////////////////////////

/*
 * @brief   rev64
 *
 * @note    reverse byte order of a 64 bit integer
 */


static uint64_t rev64(uint64_t x) {
uint32_t xh,xl,xt;

    xt = (uint32_t) (x>>32);
    xl = __REV(xt);
    xt = (uint32_t) x;
    xh = __REV(xt);
    x = (((uint64_t) xh)<<32) | (uint64_t) xl;
    return x;

}

/**
 * @brief   Set Mac Address #0
 *
 * @note    The MAC address high register holds the upper 16 bits of the 6-byte 
 *          first MAC address of the station. Note that the first DA byte that 
 *          is received on the MII interface corresponds to the LS Byte 
 *          (bits [7:0]) of the MAC address low register. For example, if
 *          0x1122 3344 5566 is received (0x11 is the first byte) on the MII 
 *          as the destination address, then the MAC address register [47:0]
 *          is compared with 0x6655 4433 2211.
 */
void
ETH_SetMACAddress(uint64_t macaddr) {
uint64_t macaddr_rev;
const uint32_t mo = (1<<31);

    macaddr_rev = rev64(macaddr);

    ETH->MACA0HR =  ((uint32_t) (macaddr_rev>>32)&0xFFFF)|mo;
    ETH->MACA0LR =   (uint32_t) (macaddr_rev);
}

/**
 * @brief   Set MAC Address #0 to #3
 *
 * @note    
 * 
 * @param   n: which MAC Address
 * @param   macaddr: 48-bit MAC Address in a 64-bit variable
 * @param   mbc: Mask byte control, source and address enable flag
 * 
 * @note    See 38.8.2 of RM
 */

void
ETH_SetMACAddressN(uint32_t n, uint64_t macaddr, uint32_t mbc) {
const uint32_t mo = (1<<31);
uint64_t macaddr_rev;

    macaddr_rev = rev64(macaddr);
    switch(n) {
    case 0:
        ETH->MACA0HR = ((uint32_t) (macaddr_rev>>32)&0xFF)|mo;
        ETH->MACA0LR =  (uint32_t) (macaddr_rev);
        break;
    case 1:
        ETH->MACA1HR = ((uint32_t) (macaddr>>32)&0xFF)|mbc;
        ETH->MACA1LR =  (uint32_t) (macaddr);
        break;
    case 2:
        ETH->MACA2HR = ((uint32_t) (macaddr>>32)&0xFF)|mbc;
        ETH->MACA2LR =  (uint32_t) (macaddr);
        break;
    case 3:
        ETH->MACA3HR = ((uint32_t) (macaddr>>32)&0xFF)|mbc;
        ETH->MACA3LR =  (uint32_t) (macaddr);
        break;
    }

}

/**
 * @brief   Get MAC Address #0 as a vector of bytes
 *
 * @note    Least significant byte first (CPU order)
 */
void
ETH_GetMACAddressAsVector(uint8_t macaddr[6]) {

    macaddr[0] = (ETH_MACADDRESS>>0)&0xFF;
    macaddr[1] = (ETH_MACADDRESS>>8)&0xFF;
    macaddr[2] = (ETH_MACADDRESS>>16)&0xFF;
    macaddr[3] = (ETH_MACADDRESS>>24)&0xFF;
    macaddr[4] = (ETH_MACADDRESS>>32)&0xFF;
    macaddr[5] = (ETH_MACADDRESS>>40)&0xFF;

}

/**
 * @brief   Get MAC Address #0 as a vector of bytes
 *
 * @note    Most significant byte first (Network order)
 */
void
ETH_GetMACAddressAsNetworkOrderedVector(uint8_t macaddr[6]) {

    macaddr[0] = (ETH_MACADDRESS>>40)&0xFF;
    macaddr[1] = (ETH_MACADDRESS>>32)&0xFF;
    macaddr[2] = (ETH_MACADDRESS>>24)&0xFF;
    macaddr[3] = (ETH_MACADDRESS>>16)&0xFF;
    macaddr[4] = (ETH_MACADDRESS>>8)&0xFF;
    macaddr[5] = (ETH_MACADDRESS>>0)&0xFF;

}

////////////////////////////////// Pin management //////////////////////////////////////////////////

#if ETH_USEGPIOFORCONFIGURATION == 1

/**
 * Configuration for ETH
 *
 *    | Board Signal | MCU Pin  |  AF  |  TRX Pin           | Description              |
 *    |--------------|----------|------|--------------------|--------------------------|
 *    | RMII_TX_EN   |  PG11    |  11  |  TXEN              | Transmit Enable          |
 *    | RMII_TXD0    |  PG13    |  11  |  TXD0              | Transmit Data 0          |
 *    | RMII_TXD1    |  PG14    |  11  |  TXD1              | Transmit Data 1          |
 *    | RMII_RXD0    |  PC4     |  11  |  RXD0/MODE0        | Receive Data 0           |
 *    | RMII_RXD1    |  PC5     |  11  |  RDD1/MODE1        | Receive Data 1           |
 *    | RMII_RXER    |  PG2     |   0  |  RXER/PHYAD0       | Receive Error            |
 *    | RMII_CRS_DV  |  PA7     |  11  |  CRS_DV/MODE2      | Carrier Sense/Data Valid |
 *    | RMII_MDC     |  PC1     |  11  |  MDC               | SMI Clock                |
 *    | RMII_MDIO    |  PA2     |  11  |  MDIO              | SMI Data Input/Output    |
 *    | RMII_REF_CLK |  PA1     |  11  |  nINT/REFCLK0      | 50 MHz REF_CLK           |
 *    | NRST         |          |      |  rRST              | Reset                    |
 *    | OSC_25M      |          |      |  XTAL1/CLKIN       |                          |
 *
 * NOTE: PG2 is used as a GPIO input. This is the default.
 */

static const GPIO_PinConfiguration pinconfig[] = {
/*
 *    | Parameter         |   Value   | Description              |
 *    |-------------------|-----------|--------------------------|
 *    | AF                |    11     | Alternate function ETH   |
 *    | Mode              |     2     | Alternate function       |
 *    | OType             |     0     | Push pull                |
 *    | OSpeed            |     3     | Very High Speed          |
 *    | Pull-up/Push down |     0     | No pull-up or push-down  |
 */
/*    GPIOx    Pin      AF  M  O  S  P  I */
   {  GPIOG,   11,      11, 2, 0, 3, 0, 0  },       // ETH_RMII_TXEN
   {  GPIOG,   13,      11, 2, 0, 3, 0, 0  },       // ETH_RMII_TXD0
   {  GPIOG,   14,      11, 2, 0, 3, 0, 0  },       // ETH_RMII_TXD1
   {  GPIOC,   4,       11, 2, 0, 3, 0, 0  },       // ETH_RMII_RXD0
   {  GPIOC,   5,       11, 2, 0, 3, 0, 0  },       // ETH_RMII_RXD1
   {  GPIOG,   0,       11, 0, 0, 3, 0, 0  },       // ETH_RMII_RXER     
   {  GPIOA,   7,       11, 2, 0, 3, 0, 0  },       // ETH_RMII_CRS_DV
   {  GPIOC,   1,       11, 2, 0, 3, 0, 0  },       // ETH_RMII_MDC
   {  GPIOA,   2,       11, 2, 0, 3, 0, 0  },       // ETH_RMII_MDIO
   {  GPIOA,   1,       11, 2, 0, 3, 0, 0  },       // ETH_RMII_REFCLK
//
   {      0,   0,        0, 0,  0, 0, 0, 0 }        // End of List Mark
};

static void
ETH_ConfigurePins(void) {

    /* Configure pins from table*/
    GPIO_ConfigureMultiplePins(pinconfig);

}


#else

/* Configuring pins using direct access to registers */

#define ETH_AF                    (11)
#define ETH_MODE                  (2)
#define ETH_OTYPE                 (0)
#define ETH_OSPEED                (3)
#define ETH_PUPD                  (0)

static void
ETH_ConfigurePins(void) {
uint32_t mAND,mOR; // Mask

    // Configure pins in GPIOA
    // 1/REFCLK  2/MDIO  7/CRS_DV

    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;

    mAND =   GPIO_AFRH_AFRH1_Msk
            |GPIO_AFRH_AFRH2_Msk
            |GPIO_AFRH_AFRH7_Msk;
    mOR  =   (ETH_AF<<GPIO_AFRH_AFRH1_Pos)
            |(ETH_AF<<GPIO_AFRH_AFRH2_Pos)
            |(ETH_AF<<GPIO_AFRH_AFRH7_Pos);
    GPIOA->AFR[0]  = (GPIOA->AFR[0]&~mAND)|mOR;

    mAND =   GPIO_MODER_MODER1_Msk
            |GPIO_MODER_MODER2_Msk
            |GPIO_MODER_MODER7_Msk;
    mOR  =   (ETH_MODE<<GPIO_MODER_MODER1_Pos)
            |(ETH_MODE<<GPIO_MODER_MODER2_Pos)
            |(ETH_MODE<<GPIO_MODER_MODER7_Pos);
    GPIOA->MODER   = (GPIOA->MODER&~mAND)|mOR;

    mAND =   GPIO_OSPEEDR_OSPEEDR1_Msk
            |GPIO_OSPEEDR_OSPEEDR2_Msk
            |GPIO_OSPEEDR_OSPEEDR7_Msk;
    mOR  =   (ETH_OSPEED<<GPIO_OSPEEDR_OSPEEDR1_Pos)
            |(ETH_OSPEED<<GPIO_OSPEEDR_OSPEEDR2_Pos)
            |(ETH_OSPEED<<GPIO_OSPEEDR_OSPEEDR7_Pos);
    GPIOA->OSPEEDR = (GPIOA->OSPEEDR&~mAND)|mOR;

    mAND =   GPIO_PUPDR_PUPDR1_Msk
            |GPIO_PUPDR_PUPDR2_Msk
            |GPIO_PUPDR_PUPDR7_Msk;
    mOR  =   (ETH_PUPD<<GPIO_PUPDR_PUPDR1_Pos)
            |(ETH_PUPD<<GPIO_PUPDR_PUPDR2_Pos)
            |(ETH_PUPD<<GPIO_PUPDR_PUPDR7_Pos);
    GPIOA->PUPDR   = (GPIOA->PUPDR&~mAND)|mOR;

    mAND =   GPIO_OTYPER_OT1_Msk
            |GPIO_OTYPER_OT2_Msk
            |GPIO_OTYPER_OT7_Msk;
    mOR  =   (ETH_OTYPE<<GPIO_OTYPER_OT1_Pos)
            |(ETH_OTYPE<<GPIO_OTYPER_OT2_Pos)
            |(ETH_OTYPE<<GPIO_OTYPER_OT7_Pos);
    GPIOA->OTYPER  = (GPIOA->OTYPER&~mAND)|mOR;

    // Configure pins in GPIOC
    // 1/MDC  4/RXD0  5/RXD1

    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOCEN;

    mAND =   GPIO_AFRH_AFRH1_Msk
            |GPIO_AFRH_AFRH4_Msk
            |GPIO_AFRH_AFRH5_Msk;
    mOR  =   (ETH_AF<<GPIO_AFRH_AFRH1_Pos)
            |(ETH_AF<<GPIO_AFRH_AFRH4_Pos)
            |(ETH_AF<<GPIO_AFRH_AFRH5_Pos);
    GPIOC->AFR[0]  = (GPIOC->AFR[0]&~mAND)|mOR;

    mAND =   GPIO_MODER_MODER1_Msk
            |GPIO_MODER_MODER4_Msk
            |GPIO_MODER_MODER5_Msk;
    mOR  =   (ETH_MODE<<GPIO_MODER_MODER1_Pos)
            |(ETH_MODE<<GPIO_MODER_MODER4_Pos)
            |(ETH_MODE<<GPIO_MODER_MODER5_Pos);
    GPIOC->MODER   = (GPIOC->MODER&~mAND)|mOR;

    mAND =   GPIO_OSPEEDR_OSPEEDR1_Msk
            |GPIO_OSPEEDR_OSPEEDR4_Msk
            |GPIO_OSPEEDR_OSPEEDR5_Msk;
    mOR  =   (ETH_OSPEED<<GPIO_OSPEEDR_OSPEEDR1_Pos)
            |(ETH_OSPEED<<GPIO_OSPEEDR_OSPEEDR4_Pos)
            |(ETH_OSPEED<<GPIO_OSPEEDR_OSPEEDR5_Pos);
    GPIOC->OSPEEDR = (GPIOC->OSPEEDR&~mAND)|mOR;

    mAND =   GPIO_PUPDR_PUPDR1_Msk
            |GPIO_PUPDR_PUPDR4_Msk
            |GPIO_PUPDR_PUPDR5_Msk;
    mOR  =   (ETH_PUPD<<GPIO_PUPDR_PUPDR1_Pos)
            |(ETH_PUPD<<GPIO_PUPDR_PUPDR4_Pos)
            |(ETH_PUPD<<GPIO_PUPDR_PUPDR5_Pos);
    GPIOC->PUPDR   = (GPIOC->PUPDR&~mAND)|mOR;

    mAND =   GPIO_OTYPER_OT1_Msk
            |GPIO_OTYPER_OT4_Msk
            |GPIO_OTYPER_OT5_Msk;
    mOR  =   (ETH_OTYPE<<GPIO_OTYPER_OT1_Pos)
            |(ETH_OTYPE<<GPIO_OTYPER_OT4_Pos)
            |(ETH_OTYPE<<GPIO_OTYPER_OT5_Pos);
    GPIOC->OTYPER  = (GPIOC->OTYPER&~mAND)|mOR;

    // Configure pins in GPIOG
    // 11/TX_EN  13/TXD0  14/TXD1

    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOGEN;

    mAND =   GPIO_AFRH_AFRH3_Msk
            |GPIO_AFRH_AFRH5_Msk
            |GPIO_AFRH_AFRH6_Msk;
    mOR  =   (ETH_AF<<GPIO_AFRH_AFRH3_Pos)
            |(ETH_AF<<GPIO_AFRH_AFRH5_Pos)
            |(ETH_AF<<GPIO_AFRH_AFRH6_Pos);
    GPIOG->AFR[1]  = (GPIOG->AFR[1]&~mAND)|mOR;

    mAND =   GPIO_MODER_MODER11_Msk
            |GPIO_MODER_MODER13_Msk
            |GPIO_MODER_MODER14_Msk;
    mOR  =   (ETH_MODE<<GPIO_MODER_MODER11_Pos)
            |(ETH_MODE<<GPIO_MODER_MODER13_Pos)
            |(ETH_MODE<<GPIO_MODER_MODER14_Pos);
    GPIOG->MODER   = (GPIOG->MODER&~mAND)|mOR;

    mAND =   GPIO_OSPEEDR_OSPEEDR11_Msk
            |GPIO_OSPEEDR_OSPEEDR13_Msk
            |GPIO_OSPEEDR_OSPEEDR14_Msk;
    mOR  =   (ETH_OSPEED<<GPIO_OSPEEDR_OSPEEDR11_Pos)
            |(ETH_OSPEED<<GPIO_OSPEEDR_OSPEEDR13_Pos)
            |(ETH_OSPEED<<GPIO_OSPEEDR_OSPEEDR14_Pos);
    GPIOG->OSPEEDR = (GPIOG->OSPEEDR&~mAND)|mOR;

    mAND =   GPIO_PUPDR_PUPDR11_Msk
            |GPIO_PUPDR_PUPDR13_Msk
            |GPIO_PUPDR_PUPDR14_Msk;
    mOR  =   (ETH_PUPD<<GPIO_PUPDR_PUPDR11_Pos)
            |(ETH_PUPD<<GPIO_PUPDR_PUPDR13_Pos)
            |(ETH_PUPD<<GPIO_PUPDR_PUPDR14_Pos);
    GPIOG->PUPDR   = (GPIOG->PUPDR&~mAND)|mOR;

    mAND =   GPIO_OTYPER_OT11_Msk
            |GPIO_OTYPER_OT13_Msk
            |GPIO_OTYPER_OT14_Msk;
    mOR  =   (ETH_OTYPE<<GPIO_OTYPER_OT11_Pos)
            |(ETH_OTYPE<<GPIO_OTYPER_OT13_Pos)
            |(ETH_OTYPE<<GPIO_OTYPER_OT14_Pos);
    GPIOG->OTYPER  = (GPIOG->OTYPER&~mAND)|mOR;

}

#endif


/**
 * @brief   ETH_ReadRMIIError
 * 
 * @note    Reads the RMII_RXER signal
 * 
 * @note    This signal is connected to pin 2 of GPIO Port G
 * 
 * @note    There is no provision for the connection of RMII_RX_ER, but thres is
 *          two for MII_RX_ER at PB10 and PI10
 */

uint32_t 
ETH_ReadRMIIError(void) {
const uint32_t ETH_RMIIRXER_MASK = (1<<2);

    return GPIOG->IDR&ETH_RMIIRXER_MASK;

}

#ifdef ETH_USE_GPIO_INTERRUPT
/**
 * @brief   Interrupt routine for RMII_RX_ER
 * 
 * @note    The RMII_RXER is connected to pin 2 of GPIO Port G
 */


void EXTI2_IRQHandler(void) {

     // TODO

}

/**
 * @brief  Configure that RMII_RX_ER generates an interrupt
 * 
 * @note   Since RMII_RX_ER is connected to pin 2 of GPIO G,
 *         and that only one pin 2 of a GPIO port can generate
 *         an interrupt, this routines configure the EXTI controller
 *         that GPIOG generates the interrupt.
 */
void ConfigureEXTI2(void) {

    SYSCFG->EXTICR1 =  (SYSCFG->EXTICR1&~SYSCFG_EXTICR1_EXT2_Msk)
                      |(SYSCFG_EXTICR1_EXTI2_PG);
}
#endif

/////////////////////////////////// Clock management ///////////////////////////////////////////////
/**
 * @brief ETH_EnableClock
 */
void ETH_EnableClock(uint32_t which) {

    // Enable PTP clock
    if( which&ETH_CLOCK_PTP )   RCC->AHB1ENR |= RCC_AHB1ENR_ETHMACPTPEN;
    // Enable Ethernet MAC Reception clock
    if( which&ETH_CLOCK_MACRX ) RCC->AHB1ENR |= RCC_AHB1ENR_ETHMACRXEN;
    // Enable Ethermet Trasmission clock
    if( which&ETH_CLOCK_MACTX ) RCC->AHB1ENR |= RCC_AHB1ENR_ETHMACTXEN;
    // Enable MAC clock address
    if( which&ETH_CLOCK_MAC )   RCC->AHB1ENR |= RCC_AHB1ENR_ETHMACEN;

}

/**
 * @brief ETH_DisableClock
 */
void ETH_DisableClock(uint32_t which) {

    // Disable PTP clock
    if( which&ETH_CLOCK_PTP )   RCC->AHB1ENR &= ~RCC_AHB1ENR_ETHMACPTPEN;
    // Disable Ethernet Reception clock
    if( which&ETH_CLOCK_MACRX ) RCC->AHB1ENR &= ~RCC_AHB1ENR_ETHMACRXEN;
    // Disable Ethermet Trasmission clock
    if( which&ETH_CLOCK_MACTX ) RCC->AHB1ENR &= ~RCC_AHB1ENR_ETHMACTXEN;
    // Disable MAC clock address
    if( which&ETH_CLOCK_MAC )   RCC->AHB1ENR &= ~RCC_AHB1ENR_ETHMACEN;

}


///////////////////////////// PHY management ///////////////////////////////////////////////////////


/**
 * @brief  ETH_WritePHYRegister
 */
static int ETH_WritePHYRegister( uint32_t reg, uint16_t val) {
uint32_t macmiiar = ETH->MACMIIAR;

    // Clear all fields except CR
    macmiiar &= ETH_MACMIIAR_CR_Msk;

    // Set PHY address
    macmiiar |= (ETH_PHY_ADDRESS<<ETH_MACMIIAR_PA_Pos);

    // Set PHY register
    macmiiar |= ((reg)<<ETH_MACMIIAR_MR_Pos);

    // Set operation to write
    macmiiar |= ETH_MACMIIAR_MW;

    // Wait until PHY is ready
    while( (ETH->MACMIIAR&ETH_MACMIIAR_MB) == ETH_MACMIIAR_MB ) {}  // TODO: Add timeout

    // Set data to be written
    ETH->MACMIIDR = val;


    // Set busy flag
    macmiiar |= ETH_MACMIIAR_MB;
    
    // Write operation
    ETH->MACMIIAR = macmiiar;

    // Test if it runs ok
    delay(10);
    while( (ETH->MACMIIAR&ETH_MACMIIAR_MB) == ETH_MACMIIAR_MB ) {}  // TODO: Add timeout

    return 0;
}


/**
 * @brief  ETH_ReadPHYRegister
 */
static int ETH_ReadPHYRegister( uint32_t reg, uint16_t *val) {
uint32_t macmiiar = ETH->MACMIIAR;

    // Set register to 0
    *val = 0;

    // Set data register to 0
    ETH->MACMIIDR = 0;

    // Clear all fields except CR
    macmiiar &= ETH_MACMIIAR_CR_Msk;

    // Set PHY address
    macmiiar |= (ETH_PHY_ADDRESS<<ETH_MACMIIAR_PA_Pos);

    // Set PHY register
    macmiiar |= ((reg)<<ETH_MACMIIAR_MR_Pos);

    // Set read operation
    macmiiar &= ~ETH_MACMIIAR_MW;

    // Wait until PHY is ready
    while( (ETH->MACMIIAR&ETH_MACMIIAR_MB) == ETH_MACMIIAR_MB  ) {}  // TODO: Add timeout

    // Set busy flag
    macmiiar |= ETH_MACMIIAR_MB;

    // Start read operation
    ETH->MACMIIAR = macmiiar;

    // Test if it runs ok
    delay(10);
    while( (ETH->MACMIIAR&ETH_MACMIIAR_MB) == ETH_MACMIIAR_MB ) {}  // TODO: Add timeout

    // Read value
    *val = ETH->MACMIIDR;

    return 0;
}

/**
 * @brief  Configure PHY
 *
 * @note   PHY is Microchip LAN
 */

static int ETH_ConfigurePHY(void) {
uint16_t value;
int retries;
int configured = 0;

    MESSAGE("Entering ConfigurePHY\n");

    ETH_PHYRegisterDump();

    // Reset PHY
    ETH_WritePHYRegister(ETH_PHY_BCR,ETH_PHY_BCR_RESET);
    delay(ETH_DELAY_AFTERRESET*100);
    // Wait until Soft Reset bit self cleared
    do { 
        value = 0xFFFF;
        ETH_ReadPHYRegister(ETH_PHY_BCR,&value);
        delay(ETH_DELAY_AFTERRESET);
    } while ( value&ETH_PHY_BCR_RESET );

    // Configure Auto-Negotiation Advertisement Register
    // Should be done by MODE2_0 pins!!
    //ETH_WritePHYRegister(ETH_PHY_ANA,ETH_PHY_ANA_ALL);
    //delay(ETH_DELAY_BETWEENTESTS);

    // It this needed?
    //value &= ETH_PHY_BCR_LOOPBACK;

    // Wait until connected (linked)
    retries = ETH_RETRIES_LINK*100;
    do {
        value = 0;
        ETH_ReadPHYRegister(ETH_PHY_BSR,&value);
        delay(ETH_DELAY_BETWEENTESTS);
    } while( (retries-->0)&&((value&ETH_PHY_BSR_LINKUP)==0) );

    // Configure using auto negotiation when connected
    if( (ETH_Config&ETH_CONFIG_AUTONEGOTIATE) && (value&ETH_PHY_BSR_LINKUP) ) {
        // Start autonegotiating
        ETH_WritePHYRegister(ETH_PHY_BCR,ETH_PHY_BCR_AUTONEGOTIATIONENABLE);
        delay(ETH_DELAY_AFTERREGISTERWRITE);
    
        // Wait until Auto Negotiation Completed
        retries = ETH_RETRIES_AUTONEGOTIATION;
        do {
            value = 0;
            ETH_ReadPHYRegister(ETH_PHY_BSR,&value);
            delay(ETH_DELAY_BETWEENTESTS);
        } while( (retries-->0)&&((value&ETH_PHY_BSR_AUTONEGOTIATIONCOMPLETED)==0) );

        if( (value&ETH_PHY_BSR_AUTONEGOTIATIONCOMPLETED)  ) {
            // Autonegotiation completed
            configured = 1;
        }           
    }
    if( ! configured ) {
        // Manual configuration
        ETH_ManualConfig();
        configured = 1;
    }
    // Update speed and connection in ConfigStatus
    ETH_UpdateConfigStatus();

    MESSAGEV("Link status = ***%s***\n",ETH_GetLinkInfoString());
    
    /*
     * This code sequence is identical to the one generated by
     * STM32CubeMX.
     * 
     * Since nINT not connected, PHY can not generate interrupts.
     * According LAN8742A datasheet, change of ISFR does not make sense,
     * because its fields are Read-Only.
     * 
     * But this register is vendor-specific. Maybe it does not work
     * following the datasheet.
     * 
     * From oryx-embedded.com
     * Any link failure condition is latched in the BMSR register. Reading
     * the register twice will always return the actual link status
     */
    #if 0
    // Enable interrupt on link status 
    ETH_ReadPHYRegister(ETH_PHY_ISFR,&value);
    value |= ETH_PHY_ISFR_INT4;
    ETH_WritePHYRegister(ETH_PHY_ISFR,value);
    // Read again to clear
    ETH_ReadPHYRegister(ETH_PHY_ISFR,&value);
    #endif

    return 0;
}


/**
 * @brief   Reset ETH controller
 * 
 */
void ETH_Reset(void) {

    // Reset ETH system
    ETH->DMABMR |= ETH_DMABMR_SR;
    // Wait until reset                     
    // TODO: Add timeout
    while( (ETH->DMABMR&ETH_DMABMR_SR) != 0 ) { // TODO: Timeout
        delay(100);
    }

}

/**
 * @brief   Find clock division encoding for SMI
 * 
 * @note    Find divisor of HCLK to generate the clock for the SMI channel
 * 
 * @note    The communication clock must be in the range 1.25-2.5 MHz
 *
/*
 * @note    Table used to set CR clock range to select MDC clock frequency
 *
 * @note    See description of CR field of MACMIIAR register in section 38.8.1 in RM.
 *
 * @note    The order is important. The index is used to configure field!!!
///@{ */
struct {
    uint32_t    minfreq;
    uint32_t    maxfreq;

} static cr_tab[] = {
    {   60000000,       100000000   },      // CR=000
    {  100000000,       150000000   },      // CR=001
    {   20000000,        35000000   },      // CR=010
    {   35000000,        60000000   },      // CR=011
    {  150000000,       216000000   },      // CR=100
    {          0,               0   }
};

static uint32_t findCREncoding(void) {
uint32_t hclkfreq;
int cr;

    hclkfreq = SystemGetHCLKFrequency();
    cr = 0;
    while( cr_tab[cr].minfreq
        && ( (hclkfreq < cr_tab[cr].minfreq)
            || (hclkfreq > cr_tab[cr].maxfreq)
           )
         ) {
        cr++;
    }
    return cr<<ETH_MACMIIAR_CR_Pos;

}
///@}


/**
 * @brief   Configure SMI
 * 
 * @note    Station Management Interface (SMI) is a serial communication
 *          channel providing control and configuration functions for  th
 *          PHY device
 * 
 * @note    It uses a synchronous serial communication with 2 wires: clock
 *          and data
 * 
 * @note    The clock frequency must be in the range 1.25-2.5 MHz 
 * 
 */
void ETH_ConfigureSMI(void) {

    // Configuring MDC CLock
    uint32_t cr = findCREncoding();
    ETH->MACMIIAR = (ETH->MACMIIAR&~ETH_MACMIIAR_CR_Msk)|cr;

}


/**
 * @brief   ETH ConfigureMAC
 *
 * @note    Configuring Media Access Control of the ETH Controller 
 * 
 * @note    MAC Registers
 * 
 *          * MACR: configuration register
 *          * MACFFR: frame filter register
 *          * MACHTHR: hash table high register
 *          * MACHTLR: hash table low register
 *          * MACMIIAR: MII (Media Independent Interface) address register
 *          * MACMIIDR: MII (Media Independent Interface) data register
 *          * MACFCR: flow control register
 *          * MACVLANTR: VLAN tag register
 *          * MACRWUFFR: remote wakeup frame filter register
 *          * MACPMRCSR: PMT (Power Management) control and status register
 *          * MACDBGR: debug register
 *          * MACSR: interrupt status register
 *          * MACIMR: interrupt mask register
 *          * MACA0HR: address 0 high register
 *          * MACA0LR: address 0 low register
 *          * MACA1HR: address 1 high register
 *          * MACA1LR: address 1 low register
 *          * MACA2HR: address 2 high register
 *          * MACA2LR: address 2 low register
 *          * MACA3HR: address 3 high register
 *          * MACA3LR: address 3 low register
 
 * 
 * @note    Configuration generated by STM32CubeMX
 * 
 *          * Watchdog enabled
 *          * Jabber enabled
 *          * Interframe gap set to 96
 *          * Carriersense enabled
 *          * Receive own enable
 *          * Loopback disabled
 *          * Checksum done by hardware
 *          * Retry transmission disabled
 *          * Automatic PAD CRC strip disabled
 *          * Backoff limit set to 10
 *          * Deferral check disabled
 *          * Receive all disabled
 *          * Source address filter disabled
 *          * Block all control frames
 *          * Broadcastframes reception_enabled
 *          * Destination filter normal
 *          * Promiscuous mode disabled
 *          * Multicast frames perfect
 *          * Unicast frames filter perfect
 *          * Zero quanta pause disabled
 *          * Pause low threshold set to -4
 *          * Unicast pause frame detect disabled
 *          * Receive flow control disabled
 *          * Transmit flow control disabled
 *
 */

static int ETH_ConfigureMAC(void) {

    /*************** MACCR: MAC Configuration Register ************************/

    uint32_t maccr = ETH->MACCR;

    // Clear all fields
    maccr &= 0;

    // Configure
    // There are many fields with negative logic (1 to disable, 0 to enable)
    // No CRC stripping
    // Watchdog enable. Receive at most 2048 bytes
    // Jabber enable. Transmit at most 2048 bytes
    // Carrier sense active and generate error/abort transmission
    // Fast Ethernet will be set accordingly later
    // Receive own frames enabled!!!
    // No Loopback
    // Duplex mode will be set accordingly later
    // IPv4 checksum offload (ETH controller calculates CRC)
    // Retry enabled (based on BL)
    // No automatic pad/CRC stripping
    // Back-off limit = 10
    // No deferral check
    // Receiver and transmitter disabled, for now
    maccr |=     0
                |ETH_MACCR_IPCO         // Checksum by hardware
                |ETH_MACCR_IFG_96Bit    // Interframe gap = 96
                |ETH_MACCR_RD           // Retry disable = 1
                |ETH_MACCR_BL_10        // Backoff limit = min(n,4)
                ;

    // Set configuration for Fast Ethernet and Full Duplex, when possible
    if( ETH_ConfigStatus|ETH_CONFIGSTATUS_100BASET )
        maccr |= ETH_MACCR_FES;
    if( ETH_ConfigStatus&ETH_CONFIGSTATUS_FULLDUPLEX ) 
        maccr |= ETH_MACCR_DM;

    // Set configuration
    ETH->MACCR = maccr;

    /*************** MACFFR: MAC Frame Filter Register ************************/

    uint32_t macffr = ETH->MACFFR;

    // Clear all fields
    macffr  &= 0;

    // Configure
    // Receive all disabled
    // Perfect filter????
    // Source address filter disabled
    // Source address inverse filter disabled
    // Block all control frames
    // Broadcast frame enabled
    // Pass all multicast depends on Hash multicast bit
    // Destination address inverse filtering normal
    // Hash multicast perfect
    // No promiscuous mode
    macffr |=    0
                |ETH_MACFFR_PCF_BlockAll    // Block all control frames
                ;
    // Set configuration
    ETH->MACFFR = macffr;
    delay(ETH_DELAY_AFTERREGISTERWRITE);

    /*************** MACHTxR: Hash table high/low register ********************/

    ETH->MACHTHR = 0;
    ETH->MACHTLR = 0;

    /*************** MACMIIAR: MII address register ***************************/
    // Used in ConfigurePHY

    /*************** MACMIIDR: MII data register ******************************/
    // Used in ConfigurePHY

    /*************** MACFCR: Flow control register ****************************/

    uint32_t macfcr = ETH->MACFCR;

    // Clear all fields
    macfcr  &= 0;

    // Configure
    // pause time = 0
    // zero quanta pause at normal operation
    // pause low threshold set to Pause time - 4
    // unicast pause frame detect only for multicast address at 802.3 standard
    // receive flow control disabled
    // transmit flow control disabled
    // flow control ??

    macfcr |=    0
                |ETH_MACFCR_PLT_Minus4      // Pause low = Pause time - 4
                 ;

    // Set configuration
    ETH->MACFCR = macfcr;
    delay(ETH_DELAY_AFTERREGISTERWRITE);

    /*************** MACMVLANTR: VLAN tag register ****************************/

    uint32_t macvlantr = ETH->MACVLANTR;

    // Clear all fields
    macvlantr &= 0;

    // Configure
    // 16 bit comparison
    // Tag = 0
    ETH->MACVLANTR  = macvlantr;
    delay(ETH_DELAY_AFTERREGISTERWRITE);

    /*************** MAC Address **********************************************/

    ETH_SetMACAddress(ETH_MACADDRESS);

    return 0;
}



/**
 * @brief   Configure DMA
 *
 * @note    Configurate DMA
 * 
 * @note    DMA Registers
 * 
 *          * DMAMBR: Bus mode register
 *          * DMATODR: Transmit poll demand register
 *          * DMARPDR: Receive poll demand register
 *          * DMARDLAR: Receive descriptor list address register
 *          * DMARDLAR: Transmit descriptor list address register
 *          * DMASR: Status register
 *          * DMAOMR: Operation mode register
 *          * DMAIER: Interrupt enable register
 *          * DMAMFBOCR: Missed frame and buffer overflow counter register
 *          * DMARSWTR: Receive status watchdog timer register
 *          * DMACHTDR: Current host transmit descriptor register
 *          * DMACHRDR: Current host receive descriptor register
 *          * DMACHRBAR: Current host transmit buffer address register
 *          * DMACHTBAR: Current host transmit buffer address register
 * 
 *
 * @note    Configuration generated by STM32CubeMX
 * 
 *          * Drop TCP/IP Frame on Checksum error enabled
 *          * Receive store forward enabled
 *          * Flush received frame enable
 *          * Transmit store forward enabled
 *          * Transmit threshold control set to 64 bytes
 *          * Forward error frames disabled
 *          * Forward undersized goodframes disabled
 *          * Received threshold control set to 64 bytes
 *          * Second frame operarted enabled
 *          * Address aligned beats enabled
 *          * Fixed burst_Enabled
 *          * Rx DMA burst length set to 32 beats
 *          * TxDMA burst length_set to 32 beats
 *          * DMA enhanced descriptor enabled
 *          * Descripto length set to 0x0;
 *          * DMA arbitration roundrobin RX/TX 1/1
 */

static int ETH_ConfigureDMA(void) {

    /*************** DMA Mode Register ****************************************/

    uint32_t dmabmr = ETH->DMABMR;

    // Clear all fields
    dmabmr &= 0;

    // Configure
    // Fixed burst for length < 16
    // Address aligned beats enabled
    // PBL mode 4x disabled
    // Separate PBL (Programmable Burst Length) for Rx and TX
    // Rx DMA PBL (Programmable Burst Length)
    // Fixed burst enabled
    // Rx/Tx priority ratio = 1:1
    // Programmable Burst Length (PBL) = 32
    // Enhanced descriptor format enabled (IPv4 offload active)
    // Descriptor skip length = 0
    // DMA arbitration = round robin
    // Software

    dmabmr |=  ETH_DMABMR_AAB
              |ETH_DMABMR_USP
              |ETH_DMABMR_RDP_32Beat
              |ETH_DMABMR_FB
              |ETH_DMABMR_RTPR_1_1
              |ETH_DMABMR_PBL_32Beat
              |ETH_DMABMR_EDE
              ;

    // Configure DMABMR
    ETH->DMABMR = dmabmr;
    delay(ETH_DELAY_AFTERREGISTERWRITE);


    /*************** DMA Operation Mode Register ******************************/
   uint32_t dmaomr = ETH->DMAOMR;

    // clear fields
    dmaomr &= 0;

    // Configure
    // Drop frame with checksum error
    // Receive store and forward enabled
    // Enable flush of received framed
    // Transmit store and forward
    // Flush transmit FIFO (do not touch it yet!)
    // Transmit threshold control = 64 
    // Start/stop transmission (do not touch it yet!)
    // Forward error frames disabled
    // Forward undersized good frames disabled
    // Receive threshold control = 64
    // Operate on second frame enabled
    // Start/stop receive

    dmaomr |= ETH_DMAOMR_RSF
             |ETH_DMAOMR_TSF
             |ETH_DMAOMR_TTC_64Bytes
             |ETH_DMAOMR_RTC_64Bytes
             |ETH_DMAOMR_OSF
             ;
    // Configure DMAOMR
    ETH->DMAOMR = dmaomr;
    delay(ETH_DELAY_AFTERREGISTERWRITE);

    return 0;
}


/**
 * @brief   ETH_InitTXDescriptors
 *
 * @note    desc must be an array of DMA Descriptors
 *
 * @note    area must be at least count*ETH_TXBUFFER_SIZE
 *
 * @note    desc and area must be static!!!
 */

void ETH_InitTXDescriptors(ETH_DMADescriptor *desc, int count, uint8_t *area) {
int i;
uint32_t *a = (uint32_t *) area;

    ETH_TXDescriptors = desc;
    for(i=0;i<count;i++) {
        desc->Status    = ETH_TXDESC_CHAINED | ETH_TXDESC_CIC;
        desc->ControlBufferSize = ETH_TXBUFFERSIZE_INT8UNITS;
        desc->Buffer1Addr = (uint32_t) (a + i*ETH_TXBUFFERSIZE_INT32UNITS);
        desc->Buffer2NextDescAddr = (uint32_t) (ETH_TXDescriptors+((i+1)%count));
        desc->ExtendedStatus = 0; 
        desc->Reserved1 = 0;
        desc->TimeStampLow = 0;
        desc->TimeStampHigh = 0;
        desc++;
    }

    // Write table address to the ETH interface
    ETH->DMATDLAR = (uint32_t) ETH_TXDescriptors;

#ifdef ETH_ALLOCATE_BUFFERS_DYNAMICALLY
    buffers_initialized |= TXBUFFER_INITIALIZED;
#endif

}

/**
 * @brief ETH ETH_InitRXDescriptors
 *
 * @note  desc must be an array of DMA Descriptor
 *
 * @note  area must be count*ETH_TXBUFFER_SIZE
 *
 * @note  desc and area must be static!!!
 */


void ETH_InitRXDescriptors(ETH_DMADescriptor *desc, int count, uint8_t *area) {
int i;
uint32_t *a = (uint32_t *) area;

    ETH_RXDescriptors = desc;
    for(i=0;i<count;i++) {
        desc->Status    = ETH_RXDESC_OWN;
        desc->ControlBufferSize = ETH_RXBUFFER_SIZE 
                                | ETH_RXDESC_CHAINED 
                                | ETH_RXBUFFER_DIC;
        desc->Buffer1Addr =  (uint32_t) (a + i*ETH_RXBUFFERSIZE_INT32UNITS);
        desc->Buffer2NextDescAddr = (uint32_t) (ETH_RXDescriptors+((i+1)%count));
        desc->ExtendedStatus = 0; 
        desc->Reserved1 = 0;
        desc->TimeStampLow = 0;
        desc->TimeStampHigh = 0;
        desc++;
    }

    // Write table address to the ETH interface
    ETH->DMARDLAR = (uint32_t) ETH_RXDescriptors;

#ifdef ETH_ALLOCATE_BUFFERS_DYNAMICALLY
    buffers_initialized |= RXBUFFER_INITIALIZED;
#endif

}

#ifdef ETH_ALLOCATE_BUFFERS_DYNAMICALLY
/**
 * @brief ETH InitializeBuffers
 *
 * @note  Initialize all descriptors to point to buffers insided area
 *
 * @note  area must be aligned to a word address.]
 *
 * @note  area must have a size equal or largert to (ETH_TXBUFFER_COUNT*ETH_TXBUFFER_SIZE
 *   +ETH_RXBUFFER_COUNT*ETH_TXBUFFER_SIZE) bytes, with ETH_TXBUFFER_SIZE and ETH_RXBUFFER_SIZE
 *   multiple of sizeof(uint32_t)_
 */

void ETH_InitializeBuffers(char *area) {
uint32_t *a = (uint32_t *) area;

    ETH_InitTXDescriptors(ETH_TXDesc,ETH_TXBUFFER_COUNT,(uint8_t *) a);
    a += ETH_TXBUFFERSIZE_INT32UNITS *ETH_TXBUFFER_COUNT;
    ETH_InitRXDescriptors(ETH_RXDesc,ETH_RXBUFFER_COUNT,(uint8_t *) a);

}
#endif

/**
 * @brief   ConfigureMediaInterface
 * 
 * @note    This configuration must be done while the MAC is under reset and
 *          before enabling the MAC clocks. Since this is done using SYSCFG
 *          controller, its clock must be enabled first.
 */
static void ConfigureMediaInterface(void) {
uint32_t media = 1; // RMII PHY Interface

    // Enable SYSCFG clock to select the ethernet PHY interface to be used
    RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;
    __NOP();
    __NOP();
    __DSB();

    //  Select RMII mode
    SYSCFG->PMC = (SYSCFG->PMC&~(SYSCFG_PMC_MII_RMII_SEL))
                 |(media<<SYSCFG_PMC_MII_RMII_SEL_Pos);
}

/**
 * @brief   ETH_Init
 *
 * @note    Initialize ETH controller
 * 
 * @note    When using dynamically allocated buffers, the must be
 *          registered before calling this function
 * 
 * @note    Initialization steps:
 * 
 *        * Setup buffers
 *        * Configure pins for ETH usage
 *        * Configure clocks
 *        * Reset ETH interface
 *        * Configure clock for serial communication
 *        * Configure PHY
 *        * Configure MAC
 *        * Configure DMA
 */

void ETH_Init(void) {

    // Reset callbacks
    ETH_Callbacks.ErrorDetected    = 0;
    ETH_Callbacks.FrameReceived    = 0;
    ETH_Callbacks.FrameTransmitted = 0;
    ETH_Callbacks.LinkStatusChanged= 0;


#ifdef ETH_ALLOCATE_BUFFERS_DYNAMICALLY
    if( buffers_initialized != (RXBUFFER_INITIALIZED|TXBUFFER_INITIALIZED ) {
        while(1) {}
    }
#else
    ETH_InitTXDescriptors(ETH_TXDesc,ETH_TXBUFFER_COUNT,txbuffer);
    ETH_InitRXDescriptors(ETH_RXDesc,ETH_RXBUFFER_COUNT,rxbuffer);
#endif

    // Enable clocks for ETH
    ETH_EnableClock(ETH_CLOCK_MAC|ETH_CLOCK_MACRX|ETH_CLOCK_MACTX);

    // Configure pins
    ETH_ConfigurePins();

    // Configure Media Interface
    ConfigureMediaInterface();

    // Soft Reset of the ETH Controller
    ETH_Reset();

    // Configure SMI
    ETH_ConfigureSMI();

    // Configuring PHY
    ETH_ConfigurePHY();

    // Clear the MAC configuration
    ETH_ConfigureMAC();

    // Configure DMA
    ETH_ConfigureDMA();

#ifdef ETH_USE_ETH_IRQ
    // Configuring interrupt
    ETH->DMAIER |= (ETH_DMAIER_NISE|ETH_DMAIER_RIE);

    NVIC_SetPriority(ETH_IRQn,ETH_IRQLevel);
    NVIC_EnableIRQ(ETH_IRQn);
#endif

}


/**
 * @brief   ETH Start
 *
 * @note    ETH_Init must be successfully called before
 */

void ETH_Start(void) {

    // Enable transmission
    ETH_EnableTransmissionMAC();

    // Start reception
    ETH_EnableReceptionMAC();

    // Flush FIFO
    ETH_FlushTXFIFO();

    // Enable transmission DMA
    ETH_EnableTransmissionDMA();

    // Enable reception DMA
    ETH_EnableReceptionDMA();

}

/**
 * @brief   ETH Stop
 */

void ETH_Stop(void) {

    // Disable reception DMA
    ETH_DisableReceptionDMA();

    // Disable transmission DMA
    ETH_DisableTransmissionDMA();

    // Flush FIFO
    ETH_FlushTXFIFO();

    // Stop transmission
    ETH_DisableTransmissionDMA();

    // Stop reception
    ETH_DisableReceptionDMA();
}



/**
 * @brief   ETH_ManualConfig
 * 
 * @note    Configure PHY using configuration in ETH_Config
 */

static void
ETH_ManualConfig(void) {
uint16_t value;

    MESSAGE("Entering manual configuration\n");
    value = 0;
    if( ETH_Config&ETH_CONFIG_FULLDUPLEX ) {
        if ( ETH_Config&ETH_CONFIG_100BASET )  {
            MESSAGE("100BASET FULL DUPLEX\n");
            value |= ETH_PHY_BCR_SPEED100MHz|ETH_PHY_BCR_FULLDUPLEX;
        } else if ( ETH_Config&ETH_CONFIG_10BASET ) {
            MESSAGE("10BASET FULL DUPLEX\n");
            value |= ETH_PHY_BCR_FULLDUPLEX;
        }
    } else if( ETH_Config&ETH_CONFIG_HALFDUPLEX ) {
        if ( ETH_Config&ETH_CONFIG_100BASET )  {
            MESSAGE("100BASET HALF DUPLEX\n");
            value |= ETH_PHY_BCR_SPEED100MHz;
        } else if ( ETH_Config&ETH_CONFIG_10BASET ) {
            MESSAGE("10BASET HALF DUPLEX\n");
            value |= 0;
        }
    }
    // Write config
    ETH_WritePHYRegister(ETH_PHY_BCR,value);
    delay(ETH_DELAY_AFTERCONFIG);

    MESSAGE("Exiting manual configuration\n");

}


/**
 * @brief   Update configuration status when connected
 * 
 * @note    To be used when connecting
 * 
 * @returns 0 when link not up
 *          1  when configured by autonegotiation
 *          2  when configured manually
 * 
 * @note    ETH_Status is set according the speed and duplex mode 
 */
static void
ETH_UpdateConfigStatus(void) {
int configured = 0;
int retries = 0;
uint16_t value;

    MESSAGE("Entering UpdateConfigStatus\n");

    // Get the result
    ETH_ReadPHYRegister(ETH_PHY_BSR,&value);

    if( (value&ETH_PHY_BSR_LINKUP) == 0 ) {
        ETH_ConfigStatus = ETH_CONFIGSTATUS_LINKDOWN;
        return;
    }
    ETH_ConfigStatus = ETH_CONFIGSTATUS_LINKUP;

    int status = ETH_GetLinkInfo();   
    
    // Set status
    switch(status) {
    case ETH_LINKINFO_100BASET_FULLDUPLEX:
        ETH_ConfigStatus |= ETH_CONFIG_100BASET;
        ETH_ConfigStatus |= ETH_CONFIG_FULLDUPLEX;
        MESSAGE("100BASET Full Duplex\n");
        break;
    case ETH_LINKINFO_100BASET_HALFDUPLEX:
        ETH_ConfigStatus |= ETH_CONFIG_100BASET;
        ETH_ConfigStatus |= ETH_CONFIG_HALFDUPLEX;
        MESSAGE("100BASET Half Duplex\n");
        break;
    case ETH_LINKINFO_10BASET_FULLDUPLEX:
        ETH_ConfigStatus |= ETH_CONFIG_10BASET;
        ETH_ConfigStatus |= ETH_CONFIG_FULLDUPLEX;
        MESSAGE("10BASET Full Duplex\n");
        break;
    case ETH_LINKINFO_10BASET_HALFDUPLEX:
        ETH_ConfigStatus |= ETH_CONFIG_10BASET;
        ETH_ConfigStatus |= ETH_CONFIG_HALFDUPLEX;
        MESSAGE("10BASET Half Duplex\n");
        break;
    }

    // Set MAC configuration for Fast Ethernet and Full Duplex
    uint32_t maccr = ETH->MACCR;
    maccr &= ~(ETH_MACCR_FES|ETH_MACCR_DM);
    if( ETH_ConfigStatus&ETH_CONFIG_100BASET )   maccr |= ETH_MACCR_FES;
    if( ETH_ConfigStatus&ETH_CONFIG_FULLDUPLEX ) maccr |= ETH_MACCR_DM;
    ETH->MACCR = maccr;

    MESSAGE("Exiting UpdateConfigStatus\n");
}




/**
 * @brief   UpdateLinkStatus
 * 
 * @note    It must be called when there is a change in link status
 */
int
ETH_UpdateLinkStatus(void) {
int configured = 0;
int retries = 0;
uint16_t value;

    MESSAGE("Entering UpdateLinkStatus\n");
    if( ETH_Config&ETH_CONFIG_AUTONEGOTIATE ) {
        MESSAGE("Autonegotiation\n");
        // Autonegotiate set
        retries = ETH_RETRIES_LINK*100;
        do {
            value = 0;
            ETH_ReadPHYRegister(ETH_PHY_BSR,&value);
            delay(ETH_DELAY_BETWEENTESTS);
            retries--;
        } while ((retries>0)&&(value&ETH_PHY_BSR_LINKUP)!=ETH_PHY_BSR_LINKUP);

        if( (value&ETH_PHY_BSR_LINKUP) == 0 ) {
            // Not connected
            MESSAGE("Link down. Not connected?\n");
            configured = 0;
            goto end;
        }

        if( (value&ETH_PHY_BSR_AUTONEGOTEABILITY) == 0 ) {
            MESSAGE("Link down. Not connected?\n");
            configured = 0;
        } else {
            MESSAGE("Starting Autonegotiation: ");
            // Configure Autonegotiate
            ETH_WritePHYRegister(   ETH_PHY_BCR,
                                    ETH_PHY_BCR_AUTONEGOTIATIONRESTART
                                    );
            delay(ETH_DELAY_AFTERAUTONEGOTIATION);
            retries = ETH_RETRIES_AUTONEGOTIATION*10;
            do {
                value = 0;
                delay(ETH_DELAY_BETWEENTESTS*100);
        //       ETH_ReadPHYRegister(ETH_PHY_BSR,&value);
                ETH_ReadPHYRegister(ETH_PHY_BSR,&value);
                __DSB();
            } while(    (retries-->0)
                    && ((value&ETH_PHY_BSR_AUTONEGOTIATIONCOMPLETED)==0) );

            if( value&ETH_PHY_BSR_AUTONEGOTIATIONCOMPLETED ) {
                MESSAGE("Success\n");
                // Update config status
                ETH_UpdateConfigStatus();
                configured = 1;
            } else {
                MESSAGE("Failed\n");
                configured = 0;
            }
        }
    }
    if( ! configured ) {
        MESSAGE("Manual configuration\n");
        // Autonegotiation not set or not working
        // Configure Speed and Duplex Mode
        ETH_ManualConfig();

        ETH_UpdateConfigStatus();
        configured = 2;
    }

end:
    // Post configuration actions
    MESSAGE("Exiting UpdateLinkStatus\n");

    return configured;

}



/**
 * @brief   Transmit data already in DMA buffer 
 * 
 * @note    size must be smaller or equal to ETH_TXBUFFER_SIZE*ETH_MAX_PACKET_SIZE
 * 
 * @note    It uses a default (non OSF) mode, i.e., only one frame beeing
 *          transmitted
 *
 * @note    Buffer can only be modified when there is no TX DMA operation running
 *
 * @note    Procedure to transmit a multiple buffer frame
 *
 *          1. Wait until descriptors are free (OWN=0)
 *          2. Copy the data to the DMA buffers
 *          3. Adjust descriptors flags
 *          4. Set OWN bit in all descriptors
 *          3. Set ST bit in DMAOMR to make DMA enter in Run state
 *    Copy data to transmission device (send it!)
 *          4.    If not the last one, get next descriptor
 *          5. Until all data is transmitted (No more descriptors with OWN bit set) or
 *             a descriptor has the LS bit set.

 *          7. Set ST bit in DMAOMR register to start the DMA transmit engine*
 * 
 * @note    To resume operation, write something to DMATPDR
 *
 */
int
ETH_TransmitFrame(ETH_DMADescriptor *desc, unsigned size) {
int nbuffers,lastbuffersize;


    if( size == 0 )
        return 0;

    MESSAGE("Entering ETH_TransmitFrame\n");

    desc = ETH_TXDescriptors;
    // Check OWN bit in the first descriptor. If it is set, DMA is using it
    if( (desc->Status&ETH_TXDESC_OWN) != 0 ) {
        return -1;      // BUSY!!!!!!
    }

    nbuffers = size/ETH_TXBUFFER_SIZE;
    lastbuffersize = size%ETH_TXBUFFER_SIZE;
    if( lastbuffersize != 0 ) nbuffers++;

    if( nbuffers == 1 ) {
        // Configure first and only descriptor
        desc->Status |= ETH_TXDESC_FIRST|ETH_TXDESC_LAST;
        desc->ControlBufferSize = lastbuffersize&ETH_TXDESC_BUFFER1SIZE_MSK;
        desc->Status |= ETH_TXDESC_OWN;
        __DSB();
    } else {
        // Configure first descriptor
        desc->Status = (desc->Status&~ETH_TXDESC_LAST)|ETH_TXDESC_FIRST;
        desc->ControlBufferSize = ETH_TXBUFFER_SIZE&ETH_TXDESC_BUFFER1SIZE_MSK;
        desc = (ETH_DMADescriptor *) (desc->Buffer2NextDescAddr);
        desc->Status |= ETH_TXDESC_OWN;

        // Configure intermediate descriptors
        for(int i=1;i<nbuffers-1;i++) {
            desc->Status  &= ~(ETH_TXDESC_FIRST|ETH_TXDESC_LAST);
            desc->ControlBufferSize = ETH_TXBUFFER_SIZE&ETH_TXDESC_BUFFER1SIZE_MSK;
            desc = (ETH_DMADescriptor *) (desc->Buffer2NextDescAddr);
            desc->Status |= ETH_TXDESC_OWN;
        }
        // Configure last descriptor
        desc->Status = (desc->Status&~ETH_TXDESC_FIRST)|ETH_TXDESC_LAST;
        desc->ControlBufferSize = lastbuffersize&ETH_TXDESC_BUFFER1SIZE_MSK;
        desc->Status |= ETH_TXDESC_OWN;
        __DSB();
    }

    // Check Transmit buffer status
    if( ETH->DMASR&ETH_DMASR_TBUS ) {
        // Clear it and start transmission
        ETH->DMASR = ETH_DMASR_TBUS;        // Clear
        ETH->DMATPDR = 0;                   // Transmit poll demand
    }

    // Place transmissition in the Run state (just in case)
    ETH->DMAOMR |= ETH_DMAOMR_ST;

    MESSAGE("Exiting ETH_TransmitFrame\n");
    return 0;
}



/**
 * @brief   Receive data
 *
 * @note    Check if there is data in buffers and sets an indicator
 *
 * @note    Procedure to receive a multiple buffer frame
 *
 *       1.  Setup receive descriptors
 *       2.  Set OWN bit
 *       3.  Set bit SR in DMAOMR register to start the DMA receive engine
 *       4.  Repeat
 *             Receive data and store it into the buffer pointed by descriptor
 *             Write the first word (+0 offset) with OWN bit cleared and flags set
 *       5.  Until the last segment is received or the descriptor list does not have
 *           descriptors for the DMA engine, i.e,with OWN bit set.
 *
 * @note    To resume operation or force the scan of the descriptor list, a value 
 *          must be written to the DMA
 *
 * @note    All buffers will use the maximum size, except the last one. One must 
 *          get the FL field to get the size of the data in the last buffer.
 */

int
ETH_ReceiveFrame(ETH_DMAFrameInfo *RxFrameInfo) {
int len;
int bsize;
ETH_DMADescriptor *desc,*descfirst;
uint32_t status;
int first;
int rc = 0;

    // Clean RxFrameInfo
    RxFrameInfo->SegmentCount = 0;
    RxFrameInfo->FirstSegmentDesc = 0;
    RxFrameInfo->LastSegmentDesc = 0;
    RxFrameInfo->FrameLength = 0;

    MESSAGE("Entering ETH_ReceiveFrame\n");

    ETH_EnableReceptionDMA();

    // Searching for First segment
    desc = ETH_RXDescriptors;/*  */
    status = desc->Status;
    first = 0;
    do {
        uint32_t x = (status&ETH_RXDESC_OWN)?1:0;
        message("Processing descriptor at %p (own=%d). ",desc,x);
        first = ((status&ETH_RXDESC_OWN)==0)&&((status&ETH_RXDESC_FIRST)==1);
        if( first ) {
            // found a first segment
            break;
        }
        desc = (ETH_DMADescriptor *) desc->Buffer2NextDescAddr;
        message("Next is %p\n",desc);
        status = desc->Status;
    } while ( desc != ETH_RXDescriptors );


    if( !first ) {
        MESSAGE("A first segment not found\n");
        rc = -1;
        goto err;
    }

    MESSAGE("Found a first segment\n");
    descfirst = desc;
    status = desc->Status;

    while( (status&ETH_RXDESC_OWN) == 0 )  {
        message("processing descriptor at %p\n",desc);
        len = (status&ETH_RXDESC_FRAMELENGTH_MASK)>>ETH_RXDESC_FRAMELENGTH_POS;
        bsize = (desc->ControlBufferSize&ETH_RXDESC_BUFFER1SIZE_MASK)>>ETH_RXDESC_BUFFER1SIZE_POS;

        if( (status&ETH_RXDESC_FIRST) && (status&ETH_RXDESC_LAST) ) {
            MESSAGE("Received only one frame\n");
           // Only one buffer
            RxFrameInfo->FirstSegmentDesc = desc;
            RxFrameInfo->LastSegmentDesc  = 0;
            RxFrameInfo->SegmentCount = 1;
            len = (desc->Status&ETH_RXDESC_FRAMELENGTH_MASK)>>ETH_RXDESC_FRAMELENGTH_POS;
            RxFrameInfo->FrameLength = (len-4);      // exclude CRC
            rc = 1;
            break;
        } else if ( status&ETH_RXDESC_FIRST ) {
            MESSAGE("Received first frame\n");
            // first buffer
            RxFrameInfo->FirstSegmentDesc = desc;
            RxFrameInfo->LastSegmentDesc  = 0;
            RxFrameInfo->SegmentCount = 1;
            RxFrameInfo->FrameLength = bsize;
        } else if ( status&ETH_RXDESC_LAST )  {
            MESSAGE("Received last frame\n");
            // last buffer
            RxFrameInfo->LastSegmentDesc  = desc;
            RxFrameInfo->SegmentCount++;
            len = (desc->Status&ETH_RXDESC_FRAMELENGTH_MASK)>>ETH_RXDESC_FRAMELENGTH_POS;
            RxFrameInfo->FrameLength += (len-4);     // exclude CRC
            rc = 1;
            break;
        } else {
            MESSAGE("Received middle frame\n");
            // intermediary
            RxFrameInfo->SegmentCount++;
            RxFrameInfo->FrameLength += bsize;
        }
        MESSAGE("Next\n");
        desc = (ETH_DMADescriptor *) desc->Buffer2NextDescAddr;
        // Test if it returned to the start
        if( desc == descfirst )
            break;
        status = desc->Status;
    }

err:
    ETH_EnableReceptionDMA();
    MESSAGE("Exiting ETH_ReceiveFrame\n");

    return rc;
}


/**
 * @brief   Check if a frame has been received
 *
 * @note    Specially the last buffer must have been received
 *
 */
int
ETH_CheckReception(void) {
ETH_DMADescriptor *desc;
uint32_t SegmentCount = 0;

    desc = ETH_RXDescriptors;

    SegmentCount = 0;
    while(desc) {
        if( (desc->Status&ETH_RXDESC_OWN) != 0 )  {
            return 0;
        }

        if( (desc->Status&ETH_RXDESC_FIRST) && (desc->Status&ETH_RXDESC_LAST) ) {
            // Only one buffer
            SegmentCount = 1;
            return 1;
        } else if( desc->Status&ETH_RXDESC_FIRST ) {
            // First buffer
            SegmentCount = 1;
        } else if ( desc->Status&ETH_RXDESC_LAST ) {
            // Last buffer
            SegmentCount++;
            return 1;
        } else {
            // Middle buffer
            SegmentCount++;
        }
        desc = (ETH_DMADescriptor *) desc->Buffer2NextDescAddr;
        // Test if it returned to the start
        if( desc == ETH_RXDescriptors )
            break;
    }
    return 0;
}



/**
 * @brief   Flush TX FIFO
 *
 */
static void
ETH_FlushTXFIFO(void) {

    ETH->DMAOMR = ETH_DMAOMR_FTF;

    delay(ETH_DELAY_AFTERFLUSH);

    return;
}


/**
 * @brief   ETH Enable/Disable Transmission DMA
 */
///@{
void ETH_EnableTransmissionDMA(void) {

    ETH->DMAOMR |= ETH_DMAOMR_ST;
}

void ETH_DisableTransmissionDMA(void) {

    ETH->DMAOMR &= ~ETH_DMAOMR_ST;
}
///@}


/**
 * @brief   ETH Enable/Disable Reception DMA
 */
///@{
void ETH_EnableReceptionDMA(void) {

    ETH->DMAOMR |= ETH_DMAOMR_SR;
}

void ETH_DisableReceptionDMA(void) {

    ETH->DMAOMR &= ~ETH_DMAOMR_SR;
}
///@}


/**
 * @brief   ETH Enable/Disable Transmission MAC
 */
///@{
void ETH_EnableTransmissionMAC(void) {

    ETH->MACCR |= ETH_MACCR_TE;

    delay(ETH_DELAY_AFTERMAC);
}

void ETH_DisableTransmissionMAC(void) {

    ETH->MACCR &= ~ETH_MACCR_TE;

    delay(ETH_DELAY_AFTERMAC);
}
///@}

/**
 * @brief   ETH Enable/Disable Reception MAC
 */
///@{
void ETH_EnableReceptionMAC(void) {

    ETH->MACCR |= ETH_MACCR_RE;

    delay(ETH_DELAY_AFTERMAC);
}

void ETH_DisableReceptionMAC(void) {

    ETH->MACCR &= ~ETH_MACCR_RE;

    delay(ETH_DELAY_AFTERMAC);
}
///@}


////////////////// Status Functions /////////////////////////////////////////////////////////////

/**
 * @brief  Check link status and return if it up (connected)
 * 
 * @return int 
 */
int  ETH_IsConnected(void) {
int retries = ETH_RETRIES_LINK;
uint16_t value;

    MESSAGE("Is connected?\n");
    value = 0;
    do {
        ETH_ReadPHYRegister(ETH_PHY_BSR,&value);
        delay(ETH_DELAY_BETWEENTESTS);
    } while((retries-->0)&&((value&ETH_PHY_BSR_LINKUP)==0));

    message("Connected = %04X = ",value);
    value = (value&ETH_PHY_BSR_LINKUP)!=0;
    message(" %d\n",value);
    return value ;
}

/**
 * @brief   Check if link is up;
 * 
 * @brief   Alias for IsConnected
 * 
 * @return int 
 */
int  ETH_IsLinkUp(void) {

    return ETH_IsConnected();
}



/**
 * @brief   Return link status
 * 
 * @brief   Returns a merge of BCR and BSR registers of PHY
 * 
 * @return  higher 16 bits contains BCR and the lower 16 bits, BSR 
 */
unsigned ETH_GetLinkStatus(void) {
uint16_t bcr,bsr;

    ETH_ReadPHYRegister(ETH_PHY_BCR,&bcr);
    delay(ETH_DELAY_BETWEENTESTS);

    ETH_ReadPHYRegister(ETH_PHY_BSR,&bsr);
    delay(ETH_DELAY_BETWEENTESTS);

    return  (((uint32_t) bcr)<<16) | ((uint32_t) bsr);
}

///////////////// Register callback functions ////////////////////////////////////////////////////

/**
 * @brief   Register Callback functions
 *
 * @note    To unregister, set pointer to 0 or NULL
 */
void ETH_RegisterCallback(unsigned which, void (*pFunction)(unsigned) ) {

    switch(which) {
    case ETH_CALLBACK_FRAMERECEIVED:
        ETH_Callbacks.FrameReceived     = pFunction;
        break;
    case ETH_CALLBACK_FRAMETRANSMITTED:
        ETH_Callbacks.FrameTransmitted  = pFunction;
        break;
    case ETH_CALLBACK_ERRORDETECTED:
        ETH_Callbacks.ErrorDetected     = pFunction;
        break;
    case ETH_CALLBACK_LINKSTATUSCHANGED:
        ETH_Callbacks.LinkStatusChanged = pFunction;
    }

    return;
}

struct pyregisternames {
    int     index;
    char *  name;
} regtable[] = {
  {  0, "Basic Control" },
  {  1, "Basic Status" },
  {  2, "PHY Identifier 1" },
  {  3, "PHY Identifier 2" },
  {  4, "Auto Negotiation Advertisement" },
  {  5, "Auto Negotiation Partner Ability" },
  {  6, "Auto Negotiation Expansion" },
  {  7, "Auto Negotiation Next Page TX" },
  {  8, "Auto Negotiation Next Page RX" },
  { 13, "MMD Access Control" },
  { 14, "MMD Access Address/Data" },
  { 16, "EDPD NLP/Crossover Time*" },
  { 17, "Mode Control/Status*" },
  { 18, "Special Modes*" },
  { 24, "TDR Patterns/Delay Control*" },
  { 25, "TDR Control/Status*" },
  { 26, "Symbol Error Counter*" },
  { 27, "Special Control/Status Indications*" },
  { 28, "Cable Length*" },
  { 29, "Interrupt Source Flag*" },
  { 30, "Interrupt Mask*" },
  { 31, "PHY Special Control/Status" },
  { 0,  0 } /* END MARKER */
};

void ETH_PHYRegisterDump(void) {
struct pyregisternames *p = regtable;
uint16_t value;
#if 1
    while( p->name ) {
        ETH_ReadPHYRegister(p->index,&value);
        message("%35s: %04X\n",p->name,value);
        p++;
    }
#endif
}


/**
 * @brief   ETH_GetLinkInfo
 * 
 * @note    returns the link info
 * 
 */

static char const *linkinfo[] = {
    /* 000 */       "Not connected ?",
    /* 001 */       "10BaseT Half Duplex",
    /* 010 */       "100BaseT Half Duplex",
    /* 011 */       "Not connected?",
    /* 100 */       "Not connected ?",
    /* 101 */       "10BaseT Full Duplex",
    /* 110 */       "100BaseT Full Duplex",
    /* 111 */       "Not connected"
};

int
ETH_GetLinkInfo(void) {
uint16_t value;

    ETH_ReadPHYRegister(ETH_PHY_SCSR,&value);

    value >>= 2;
    value &= 0x7;
    return value;
}

char const *
ETH_GetLinkInfoString(void) {
uint16_t value;

    value = ETH_GetLinkInfo();
    return linkinfo[value];
}


/**
 * @brief   ETH_DumpDescriptors
 * 
 */
void ETH_DumpDescriptors(void) {
ETH_DMADescriptor *desc,*next;
char *buffer;

    desc = ETH_TXDescriptors;
    do {
        next = (ETH_DMADescriptor *) (desc->Buffer2NextDescAddr);
        buffer = (char *) (desc->Buffer1Addr);
        message("TX descriptor = %p: next = %p buffer = %p\n",desc,next,buffer);
        desc = next;
    } while ( desc != ETH_TXDescriptors );

    desc = ETH_RXDescriptors;
    do {
        next = (ETH_DMADescriptor *) (desc->Buffer2NextDescAddr);
        buffer = (char *) (desc->Buffer1Addr);
        message("RX descriptor = %p: next = %p buffer = %p\n",desc,next,buffer);
        desc = next;
    } while ( desc != ETH_RXDescriptors );
}