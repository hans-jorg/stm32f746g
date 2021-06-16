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



#include "stm32f746xx.h"
#include "system_stm32f746.h"
#include "gpio.h"
#include "eth.h"


/**
 * @brief   Round macro. It returns a value equal or greater than N, that is
 *          a multiple of M.
 * 
 * @param   N: value to be rounded
 * @param   M: rounding parameter, typically, sizeof(type)
 */

#define ROUND(N,M)  ( ( ((N)+(M)-1)/(M) )*(M) )
/**
 * @brief   ETH DMA Descriptor structure
 *
 * @note    From stm32f7xx_hal_eth.h (DMADescTypeDef)
 *
 * @note    Uses extended descriptor format (include PTP info)
 */

/**
 * Function prototypes (forward referenced);
 */
static void ETH_FlushTXFIFO(void);



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
static const uint32_t ETH_Config =   ETH_CONFIG_AUTONEGOTIATE
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

#define ETH_STATUS_LINKDOWN     (1)
#define ETH_STATUS_LINKUP       (2)
#define ETH_STATUS_100BASET     (4)
#define ETH_STATUS_10BASET      (8)
#define ETH_STATUS_FULLDUPLEX   (16)
#define ETH_STATUS_HALFDUPLEX   (32)

static uint32_t ETH_Status = ETH_STATUS_LINKDOWN;
///@}


// 48 bit MAC address (must have 12 hexadecimal digits))
#ifndef ETH_MACADDRESS
#define ETH_MACADDRESS 0x0080E1010101L
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
#define ETH_TXBUFFERSIZE_UINT32U ROUND(ETH_TXBUFFER_SIZE,sizeof(uint32_t))
#define ETH_RXBUFFERSIZE_UINT32U ROUND(ETH_RXBUFFER_SIZE,sizeof(uint32_t))

#ifdef ETH_ALLOCATE_BUFFERS_DYNAMICALLY
// These pointers must be initialized thru a call to ETH_SetBuffers
ETH_DMADescriptor       *ETH_TXDescriptors = 0;
ETH_DMADescriptor       *ETH_RXDescriptors = 0;
#else
ETH_DMADescriptor        ETH_TXDesc[ETH_TXBUFFER_COUNT] = { 0 };
ETH_DMADescriptor        ETH_RXDesc[ETH_RXBUFFER_COUNT] = { 0 };
ETH_DMADescriptor       *ETH_TXDescriptors = ETH_TXDesc;
ETH_DMADescriptor       *ETH_RXDescriptors = ETH_RXDesc;
// An alternative is to define as an array of 32-bit integers for alignement
static uint8_t           txbuffer[ETH_TXBUFFERSIZE_UINT32U*ETH_TXBUFFER_COUNT] 
    __attribute((aligned(sizeof(uint32_t))));
static uint8_t           rxbuffer[ETH_RXBUFFERSIZE_UINT32U*ETH_RXBUFFER_COUNT]
    __attribute((aligned(sizeof(uint32_t))));
#endif
static ETH_DMAFrameInfo  RXFrameInfo       = {0};
///@}

/**
 * @brief   Callbacks
 */
struct ETH_Callbacks_s ETH_Callbacks       = {0};

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
#define ETH_PHY_ADDRESS                         (1)

/** Registers */
///@{
#define ETH_PHY_BCR                             (0)
#define ETH_PHY_BSR                             (1)
#define ETH_PHY_ISFR                           (29)
///@}


/** Fields of BCR Register */
///@{
#define ETH_PHY_BCR_RESET                       (uint16_t) (0x8000)
#define ETH_PHY_BCR_LOOPBACK                    (uint16_t) (0x4000)
#define ETH_PHY_BCR_SPEED100MHz                 (uint16_t) (0x2000)
#define ETH_PHY_BCR_AUTONEGOCIATION             (uint16_t) (0x1000)
#define ETH_PHY_BCR_POWERDOWN                   (uint16_t) (0x0800)
///@}

/* Fields of BSR Register */
///@{
#define ETH_PHY_BSR_100BASET_FULLDUPLEX         (uint16_t) (0x4000)
#define ETH_PHY_BSR_100BASET_HALFDUPLEX         (uint16_t) (0x2000)
#define ETH_PHY_BSR_10BASET_FULLDUPLEX          (uint16_t) (0x1000)
#define ETH_PHY_BSR_10BASET_HALFDUPLEX          (uint16_t) (0x0800)
#define ETH_PHY_BSR_AUTONEGOCIATIONCOMPLETED    (uint16_t) (0x0020)
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





/**
 * @brief   small simple delay routine
 */

static void delay(int count) {
volatile int c = count;

    while( c-- ) {}

}


////////////////// IRQ Handler /////////////////////////////////////////////////////////////////////

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
#define ETH_MACADDR_MBC_AE      (1<<31)
#define ETH_MACADDR_MBC_SA      (1<<30)
#define ETH_MACADDR_MBC_BYTE5   (1<<29)
#define ETH_MACADDR_MBC_BYTE4   (1<<28)
#define ETH_MACADDR_MBC_BYTE3   (1<<27)
#define ETH_MACADDR_MBC_BYTE2   (1<<26)
#define ETH_MACADDR_MBC_BYTE1   (1<<25)
#define ETH_MACADDR_MBC_BYTE0   (1<<24)

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
 *    | RMII_RXER    |  PG2     |   ?  |  RXER/PHYAD0       | Receive Error            |
 *    | RMII_CRS_DV  |  PA7     |  11  |  CRS_DV/MODE2      | Carrier Sense/Data Valid |
 *    | RMII_MDC     |  PC1     |  11  |  MDC               | SMI Clock                |
 *    | RMII_MDIO    |  PA2     |  11  |  MDIO              | SMI Data Input/Output    |
 *    | RMII_REF_CLK |  PA1     |  11  |  nINT/REFCLK0      | Active Low interrupt Req |
 *    | NRST         |          |      |  rRST              |                          |
 *    | OSC_25M      |          |      |  XTAL1/CLKIN       |                          |
 *
 * NOTE: PG2 is not listed as having a AF11 alternate function on datasheet!!!!!!!
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
   {  GPIOG,   11,      11, 2, 0, 3, 1, 0  },       //     ETH_RMII_TXEN
   {  GPIOG,   13,      11, 2, 0, 3, 1, 0  },       //     ETH_RMII_TXD0
   {  GPIOG,   14,      11, 2, 0, 3, 1, 0  },       //     ETH_RMII_TXD1
   {  GPIOC,   4,       11, 2, 0, 3, 1, 0  },       //     ETH_RMII_RXD0
   {  GPIOC,   5,       11, 2, 0, 3, 1, 0  },       //     ETH_RMII_RXD1
//    There is a mismatch: AF0 according or AF11 according 
//   {  GPIOG,   2,       11, 0, 0, 3, 1, 0  },       //     ETH_RMII_RXER     
   {  GPIOA,   7,       11, 2, 0, 3, 1, 0  },       //     ETH_RMII_CRS_DV
   {  GPIOC,   1,       11, 2, 0, 3, 1, 0  },       //     ETH_RMII_MDC
   {  GPIOA,   2,       11, 2, 0, 3, 1, 0  },       //     ETH_RMII_MDIO
   {  GPIOA,   1,       11, 2, 0, 3, 1, 0  },       //     ETH_RMII_REFCLK
//
   {     0,    0,        0, 0,  0, 0, 0, 0  }       // End of List Mark
};

static void
ConfigureETHPins(void) {

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
ConfigureETHPins(void) {
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
    GPIOA->AFR[0]  = (GPIOC->AFR[0]&~mAND)|mOR;

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
    GPIOA->OSPEEDR = (GPIOD->OSPEEDR&~mAND)|mOR;

    mAND =   GPIO_PUPDR_PUPDR1_Msk
            |GPIO_PUPDR_PUPDR2_Msk
            |GPIO_PUPDR_PUPDR7_Msk;
    mOR  =   (ETH_PUPD<<GPIO_PUPDR_PUPDR1_Pos)
            |(ETH_PUPD<<GPIO_PUPDR_PUPDR2_Pos)
            |(ETH_PUPD<<GPIO_PUPDR_PUPDR7_Pos);
    GPIOA->PUPDR   = (GPIOD->PUPDR&~mAND)|mOR;

    mAND =   GPIO_OTYPER_OT1_Msk
            |GPIO_OTYPER_OT2_Msk
            |GPIO_OTYPER_OT7_Msk;
    mOR  =   (ETH_OTYPE<<GPIO_OTYPER_OT1_Pos)
            |(ETH_OTYPE<<GPIO_OTYPER_OT2_Pos)
            |(ETH_OTYPE<<GPIO_OTYPER_OT7_Pos);
    GPIOA->OTYPER  = (GPIOD->OTYPER&~mAND)|mOR;

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
    GPIOC->MODER   = (GPIOA->MODER&~mAND)|mOR;

    mAND =   GPIO_OSPEEDR_OSPEEDR1_Msk
            |GPIO_OSPEEDR_OSPEEDR4_Msk
            |GPIO_OSPEEDR_OSPEEDR5_Msk;
    mOR  =   (ETH_OSPEED<<GPIO_OSPEEDR_OSPEEDR1_Pos)
            |(ETH_OSPEED<<GPIO_OSPEEDR_OSPEEDR4_Pos)
            |(ETH_OSPEED<<GPIO_OSPEEDR_OSPEEDR5_Pos);
    GPIOC->OSPEEDR = (GPIOD->OSPEEDR&~mAND)|mOR;

    mAND =   GPIO_PUPDR_PUPDR1_Msk
            |GPIO_PUPDR_PUPDR4_Msk
            |GPIO_PUPDR_PUPDR5_Msk;
    mOR  =   (ETH_PUPD<<GPIO_PUPDR_PUPDR1_Pos)
            |(ETH_PUPD<<GPIO_PUPDR_PUPDR4_Pos)
            |(ETH_PUPD<<GPIO_PUPDR_PUPDR5_Pos);
    GPIOC->PUPDR   = (GPIOD->PUPDR&~mAND)|mOR;

    mAND =   GPIO_OTYPER_OT1_Msk
            |GPIO_OTYPER_OT4_Msk
            |GPIO_OTYPER_OT5_Msk;
    mOR  =   (ETH_OTYPE<<GPIO_OTYPER_OT1_Pos)
            |(ETH_OTYPE<<GPIO_OTYPER_OT4_Pos)
            |(ETH_OTYPE<<GPIO_OTYPER_OT5_Pos);
    GPIOC->OTYPER  = (GPIOD->OTYPER&~mAND)|mOR;

    // Configure pins in GPIOG
    // 2/RXCVER  11/RXD0  13/RXD1  14

    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOGEN;

/*
 *   mAND =   GPIO_AFRH_AFRH2_Msk;
 *   mOR  =   (ETH_AF<<GPIO_AFRH_AFRH2_Pos);
 *   GPIOG->AFR[0]  = (GPIOC->AFR[0]&~mAND)|mOR;
 */
    mAND =   GPIO_AFRH_AFRH3_Msk
            |GPIO_AFRH_AFRH5_Msk
            |GPIO_AFRH_AFRH6_Msk;
    mOR  =   (ETH_AF<<GPIO_AFRH_AFRH3_Pos)
            |(ETH_AF<<GPIO_AFRH_AFRH5_Pos)
            |(ETH_AF<<GPIO_AFRH_AFRH6_Pos);
    GPIOG->AFR[1]  = (GPIOC->AFR[1]&~mAND)|mOR;

    mAND =   GPIO_MODER_MODER3_Msk
            |GPIO_MODER_MODER11_Msk
            |GPIO_MODER_MODER13_Msk
            |GPIO_MODER_MODER14_Msk;
    mOR  =   (ETH_MODE<<GPIO_MODER_MODER3_Pos)
            |(ETH_MODE<<GPIO_MODER_MODER11_Pos)
            |(ETH_MODE<<GPIO_MODER_MODER13_Pos)
            |(ETH_MODE<<GPIO_MODER_MODER14_Pos);
    GPIOG->MODER   = (GPIOA->MODER&~mAND)|mOR;

    mAND =   GPIO_OSPEEDR_OSPEEDR3_Msk
            |GPIO_OSPEEDR_OSPEEDR11_Msk
            |GPIO_OSPEEDR_OSPEEDR13_Msk
            |GPIO_OSPEEDR_OSPEEDR14_Msk;
    mOR  =   (ETH_OSPEED<<GPIO_OSPEEDR_OSPEEDR3_Pos)
            |(ETH_OSPEED<<GPIO_OSPEEDR_OSPEEDR11_Pos)
            |(ETH_OSPEED<<GPIO_OSPEEDR_OSPEEDR13_Pos)
            |(ETH_OSPEED<<GPIO_OSPEEDR_OSPEEDR14_Pos);
    GPIOG->OSPEEDR = (GPIOD->OSPEEDR&~mAND)|mOR;

    mAND =   GPIO_PUPDR_PUPDR3_Msk
            |GPIO_PUPDR_PUPDR11_Msk
            |GPIO_PUPDR_PUPDR13_Msk
            |GPIO_PUPDR_PUPDR14_Msk;
    mOR  =   (ETH_PUPD<<GPIO_PUPDR_PUPDR3_Pos)
            |(ETH_PUPD<<GPIO_PUPDR_PUPDR11_Pos)
            |(ETH_PUPD<<GPIO_PUPDR_PUPDR13_Pos)
            |(ETH_PUPD<<GPIO_PUPDR_PUPDR14_Pos);
    GPIOG->PUPDR   = (GPIOD->PUPDR&~mAND)|mOR;

    mAND =   GPIO_OTYPER_OT3_Msk
            |GPIO_OTYPER_OT11_Msk
            |GPIO_OTYPER_OT13_Msk
            |GPIO_OTYPER_OT14_Msk;
    mOR  =   (ETH_OTYPE<<GPIO_OTYPER_OT1_Pos)
            |(ETH_OTYPE<<GPIO_OTYPER_OT11_Pos)
            |(ETH_OTYPE<<GPIO_OTYPER_OT13_Pos)
            |(ETH_OTYPE<<GPIO_OTYPER_OT14_Pos);
    GPIOG->OTYPER  = (GPIOD->OTYPER&~mAND)|mOR;

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
static int ETH_WritePHYRegister( uint32_t reg, uint32_t val) {
uint32_t macmiiar = ETH->MACMIIAR;

    // Clear all fields except CR
    macmiiar &= ~ETH_MACMIIAR_CR_Msk;

    // Set PHY address
    macmiiar |= (ETH_PHY_ADDRESS<<ETH_MACMIIAR_PA_Pos);

    // Set PHY register
    macmiiar |= ((reg)<<ETH_MACMIIAR_MR_Pos);

    // Set operation to write
    macmiiar |= ETH_MACMIIAR_MW;

    // Wait until PHY is ready
    while( (ETH->MACMIIAR&ETH_MACMIIAR_MB) != 0 ) {} // TODO: Add timeout

    // Set busy flag
    macmiiar |= ETH_MACMIIAR_MB;

    // Start write value
    ETH->MACMIIDR = val;

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
static int ETH_ReadPHYRegister( uint32_t reg, uint32_t *val) {
uint32_t macmiiar = ETH->MACMIIAR;

    // Clear all fields except CR
    macmiiar &= ~ETH_MACMIIAR_CR_Msk;

    // Set PHY address
    macmiiar |= (ETH_PHY_ADDRESS<<ETH_MACMIIAR_PA_Pos);

    // Set PHY register
    macmiiar |= ((reg)<<ETH_MACMIIAR_MR_Pos);

    // Set write operation
    macmiiar |= ETH_MACMIIAR_MW;

    // Wait until PHY is ready
    while( (ETH->MACMIIAR&ETH_MACMIIAR_MB) != 0 ) {}    // TODO: Add timeout

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
uint32_t value;
int retries;
int configured = 0;

    ETH_WritePHYRegister(ETH_PHY_BCR,ETH_PHY_BCR_RESET);
    delay(ETH_DELAY_AFTERRESET);

    ETH_Status = 0;
    if( ETH_Config&ETH_CONFIG_AUTONEGOTIATE ) {
        // Autonegotiate set
        retries = ETH_RETRIES_LINK;
        do {
            ETH_ReadPHYRegister(ETH_PHY_BSR,&value);
            delay(ETH_DELAY_BETWEENTESTS);
            retries--;
        } while((retries>0)&&(value&ETH_PHY_BSR_LINKUP)!=ETH_PHY_BSR_LINKUP);
        if( (value&ETH_PHY_BSR_LINKUP) != ETH_PHY_BSR_LINKUP ) {
            return -1;
        }

        // Configure Autonegociate
        ETH_WritePHYRegister(ETH_PHY_BCR,ETH_PHY_BCR_AUTONEGOCIATION);
        delay(ETH_DELAY_AFTERAUTONEGOTIATION);
        retries = ETH_RETRIES_AUTONEGOTIATION;
        do {
            ETH_ReadPHYRegister(ETH_PHY_BSR,&value);
            delay(ETH_DELAY_BETWEENTESTS);
        } while((retries>0)&&(value&ETH_PHY_BSR_AUTONEGOCIATIONCOMPLETED)!=ETH_PHY_BSR_AUTONEGOCIATIONCOMPLETED);
        if( (value&ETH_PHY_BSR_AUTONEGOCIATIONCOMPLETED)!=ETH_PHY_BSR_AUTONEGOCIATIONCOMPLETED ) {
            return -1;
        }
        // Get the result
        ETH_ReadPHYRegister(ETH_PHY_BSR,&value);
        // Set status
        if( value&(ETH_PHY_BSR_100BASET_FULLDUPLEX|ETH_PHY_BSR_10BASET_FULLDUPLEX) ) {
            ETH_Status |= ETH_CONFIG_FULLDUPLEX;
        } else {
            ETH_Status |= ETH_CONFIG_HALFDUPLEX;
        }
        if( value&(ETH_PHY_BSR_100BASET_FULLDUPLEX|ETH_PHY_BSR_100BASET_HALFDUPLEX) ) {
            ETH_Status |= ETH_CONFIG_100BASET;
        } else {
            ETH_Status |= ETH_CONFIG_10BASET;
        }
        configured = 1;
    }
    if( ! configured ) {
        // Autonegotiation not set or not working
        // Configure Speed and Duplex Mode
        value = 0;
        if( ETH_Config&ETH_CONFIG_FULLDUPLEX ) {
            if ( ETH_Config&ETH_CONFIG_100BASET )  {
                value |= ETH_PHY_BSR_100BASET_FULLDUPLEX;
                ETH_Status |= ETH_CONFIG_100BASET|ETH_CONFIG_FULLDUPLEX;
            } else if ( ETH_Config&ETH_CONFIG_10BASET ) {
                value |= ETH_PHY_BSR_10BASET_FULLDUPLEX;
                ETH_Status |= ETH_CONFIG_10BASET|ETH_CONFIG_FULLDUPLEX;
            }
        } else if( ETH_Config&ETH_CONFIG_HALFDUPLEX ) {
            if ( ETH_Config&ETH_CONFIG_100BASET )  {
                value |= ETH_PHY_BSR_100BASET_HALFDUPLEX;
                ETH_Status |= ETH_CONFIG_100BASET|ETH_CONFIG_HALFDUPLEX;
            } else if ( ETH_Config&ETH_CONFIG_10BASET ) {
                value |= ETH_PHY_BSR_10BASET_HALFDUPLEX;
                ETH_Status |= ETH_CONFIG_10BASET|ETH_CONFIG_HALFDUPLEX;
            }
        }
        // Write config
        ETH_WritePHYRegister(ETH_PHY_BCR,value);
        delay(ETH_DELAY_AFTERCONFIG);

    }
    // Enable interrupt on link status
    ETH_ReadPHYRegister(ETH_PHY_ISFR,&value);
    value |= ETH_PHY_ISFR_INT4;
    ETH_WritePHYRegister(ETH_PHY_ISFR,value);
    return 0;
}


/**
 * @brief  Table used to set CR clock range to select MDC clock frequency
 *
 * @note   See description of CR field of MACMIIAR register in section 38.8.1 in RM.
 *
 * @note   The order is important. The index is used to configure field!!!
 */
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

/**
 * @brief  Find CR encoding
 */
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
    if( ETH_Status&ETH_CONFIG_100BASET )   maccr |= ETH_MACCR_FES;
    if( ETH_Status&ETH_CONFIG_FULLDUPLEX ) maccr |= ETH_MACCR_DM;

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
 * @brief ETH InitializeDescriptorsTX
 *
 * @note  desc must be an array of DMA Descriptor
 *
 * @note  area must be count*ETH_TXBUFFER_SIZE
 *
 * @note  desc and area must be static!!!
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

//** For RX Descriptors **
// These fields are in word 0
#define ETH_RXDESC_RCH                   (1<<14)
#define ETH_RXDESC_FIRST                 (1<<9)
#define ETH_RXDESC_LAST                  (1<<8)
#define ETH_RXDESC_FIELDLENGTH_POS       (16)
#define ETH_RXDESC_FIELDLENGTH_MASK      (0x3FFF0000)
// These fields are in word 1
#define ETH_RXDESC_BUFFER1SIZE_MASK      (0x1FFF)
#define ETH_RXDESC_BUFFER1SIZE_POS       (0)
#define ETH_RXDESC_BUFFER2SIZE_MASK      (0x1FFF0000)
#define ETH_RXDESC_BUFFER2SIZE_POS       (16)
#define ETH_RXDESC_ENDOFRING             (1<<15)
#define ETH_RXDESC_CHAINED               (1<<14)
///@}

void ETH_InitializeDescriptorsTX(ETH_DMADescriptor *desc, int count, char *area) {
int i;
uint32_t *a = (uint32_t *) area;

    ETH_TXDescriptors = desc;
    for(i=0;i<count;i++) {
        desc->Status    = ETH_TXDESC_CHAINED | ETH_TXDESC_CIC;
        desc->ControlBufferSize = 0;
        desc->Buffer1Addr = (uint32_t) (a + i*ETH_TXBUFFERSIZE_UINT32U);
        desc->Buffer2NextDescAddr = (uint32_t) (desc+((i+1)%count));
        desc++;
    }

    // Write table address to the ETH interface
    ETH->DMATDLAR = (uint32_t) desc;
}

/**
 * @brief ETH InitializeDescriptorsTX
 *
 * @note  desc must be an array of DMA Descriptor
 *
 * @note  area must be count*ETH_TXBUFFER_SIZE
 *
 * @note  desc and area must be static!!!
 */


void ETH_InitializeDescriptorsRX(ETH_DMADescriptor *desc, int count, char *area) {
int i;
uint32_t *a = (uint32_t *) area;

    ETH_RXDescriptors = desc;
    for(i=0;i<count;i++) {
        desc->Status    = ETH_RXDESC_OWN;
        desc->ControlBufferSize = ETH_RXBUFFER_SIZE | ETH_RXDESC_CHAINED;
        desc->Buffer1Addr =  (uint32_t) (a + i*ETH_RXBUFFERSIZE_UINT32U);
        desc->Buffer2NextDescAddr =  (uint32_t) (desc + ((i+1)%count));
        desc++;
    }

    // Write table address to the ETH interface
    ETH->DMARDLAR = (uint32_t) desc;
}


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

    ETH_InitializeDescriptorsTX(ETH_TXDesc,ETH_TXBUFFER_COUNT,(char *) a);
    a += ETH_TXBUFFER_SIZE*ETH_TXBUFFER_COUNT;
    ETH_InitializeDescriptorsRX(ETH_RXDesc,ETH_RXBUFFER_COUNT,(char *) a);

}


/**
 * @brief   ConfigureRMII
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
 * @brief ETH_Init
 *
 * @note Configure pins for ETH usage
 * @note Configure clocks
 * @note Reset ETH interface
 * @note Configure clock for serial communication
 * @note Configure PHY
 * @note Configure MAC
 * @note COnfigure DMA
 */

void ETH_Init(void) {


    // Configure pins
    ConfigureETHPins();

    // Configure Media Interface
    ConfigureMediaInterface();

    // Enable clocks for ETH
    ETH_EnableClock(ETH_CLOCK_MAC|ETH_CLOCK_MACRX|ETH_CLOCK_MACTX);

    // Reset ETH system
    ETH->DMABMR |= ETH_DMABMR_SR;
    // Wait until reset TODO: Add timeout
    while( (ETH->DMABMR&ETH_DMABMR_SR) != 0 ) {
        delay(100);
    }

    // Configuring MDC CLock
    uint32_t cr = findCREncoding();
    ETH->MACMIIAR = (ETH->MACMIIAR&~ETH_MACMIIAR_CR_Msk)|cr;

    // Configuring PHY
    ETH_ConfigurePHY();

    // Clear the MAC configuration
    ETH_ConfigureMAC();

    // Configure DMA
    ETH_ConfigureDMA();

    // Configuring interrupt
    ETH->DMAIER |= (ETH_DMAIER_NISE|ETH_DMAIER_RIE);

    NVIC_SetPriority(ETH_IRQn,ETH_IRQLevel);
    NVIC_EnableIRQ(ETH_IRQn);

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
 * @brief   Transmit data in buffer (maximal ETH_TXBUFFER_SIZE*ETH_MAX_PACKET_SIZE)
 *
 * @note    size must be smaller or equal to ETH_TXBUFFER_SIZE*ETH_MAX_PACKET_SIZE
 *
 * @note    Buffer can only be modified when there is no tx operation running
 *
 * @note    Procedure to transmit a multiple buffer frame
 *
 *          1. Setup the descriptors.
 *          2. Set OWN bit in all
 *          3. Set ST bit in DMAOMR register to start the DMA transmit engine
 *          4. Repeat
 *          5.    Copy data to transmission device (send it!)
 *          6.    If not the last one, get next descriptor
 *          7. Until all data is transmitted (No more descriptors with OWN bit set) or
 *             a descriptor has the LS bit set.
 *
 * @note    To resume operation, write something to DMATPDR
 *
 */
int
ETH_TransmitFrame(unsigned size) {
int nbuffers,lastbuffersize;
ETH_DMADescriptor *desc;


    desc = ETH_TXDescriptors;
    // Check OWN bit in the first descriptor
    if( (desc->Status&ETH_TXDESC_OWN) == 0 ) {
        return -1;      // BUSY!!!!!!
    }

    nbuffers = size/ETH_TXBUFFER_SIZE;
    lastbuffersize = size%ETH_TXBUFFER_SIZE;
    if( lastbuffersize != 0 ) nbuffers++;

    if( nbuffers == 1 ) {
        // Configure first and only descriptor
        desc->Status = ETH_TXDESC_FIRST|ETH_TXDESC_LAST;
        desc->ControlBufferSize = lastbuffersize;
        desc->Status |= ETH_TXDESC_OWN;
    } else {
        // Configure first descriptor
        desc->Status = ETH_TXDESC_FIRST;
        desc->ControlBufferSize = ETH_TXBUFFER_SIZE;
        desc = (ETH_DMADescriptor *) (desc->Buffer2NextDescAddr);
        desc->Status |= ETH_TXDESC_OWN;

        // Configure intermediate descriptors
        for(int i=1;i>nbuffers-1;i++) {
            desc->Status  &= ~(ETH_TXDESC_FIRST|ETH_TXDESC_LAST);
            desc->ControlBufferSize = ETH_TXBUFFER_SIZE;
            desc = (ETH_DMADescriptor *) (desc->Buffer2NextDescAddr);
            desc->Status |= ETH_TXDESC_OWN;
        }
        // Configure last descriptor
        desc->Status = ETH_TXDESC_LAST;
        desc->ControlBufferSize = lastbuffersize;
        desc->Status |= ETH_TXDESC_OWN;
    }

    // Check Transmit buffer status
    if( ETH->DMASR&ETH_DMASR_TBUS ) {
        ETH->DMASR = ETH_DMASR_TBUS;        //
        ETH->DMATPDR = 0;                   // Resume transmission
    }

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
 *       5.     Receive data and store it into the buffer pointed by descriptor
 *       4.     Write the first word (+0 offset) with OWN bit cleared and flags set
 *       5.  Until the last segment is received or the descriptor list does not have
 *           descriptors for the DMA engine, i.e,with OWN bit set.
 *
 * @note To resume operation or force the scan of the descriptor list, a value must
 *       be written to the DMA
 *
 * @note All buffers will use the maximum size, except the last one. One must get the FL field
 *       to get the size of the data in the last buffer.
 */

static int
ETH_ReceiveFrame(void) {
int len;
int bsize;
ETH_DMADescriptor *desc;

    desc = ETH_RXDescriptors;
    while( (desc->Status&ETH_RXDESC_OWN) == 0 )  {
        len = (desc->Status&ETH_RXDESC_FIELDLENGTH_MASK)>>ETH_RXDESC_FIELDLENGTH_POS;
        bsize = (desc->ControlBufferSize&ETH_RXDESC_BUFFER1SIZE_MASK)>>ETH_RXDESC_BUFFER1SIZE_POS;
        if( (desc->Status&ETH_RXDESC_FIRST) && (desc->Status&ETH_RXDESC_LAST) ) {
            // Only one buffer
            RXFrameInfo.FirstSegmentDesc = desc;
            RXFrameInfo.LastSegmentDesc  = 0;
            RXFrameInfo.SegmentCount = 1;
            len = (desc->Status&ETH_RXDESC_FIELDLENGTH_MASK)>>ETH_RXDESC_FIELDLENGTH_POS;
            RXFrameInfo.FrameLength = (len-4);      // exclude CRC
            return 1;
        } else if ( desc->Status&ETH_RXDESC_FIRST ) {
            // first buffer
            RXFrameInfo.FirstSegmentDesc = desc;
            RXFrameInfo.LastSegmentDesc  = 0;
            RXFrameInfo.SegmentCount = 1;
            RXFrameInfo.FrameLength = bsize;
        } else if ( desc->Status&ETH_RXDESC_LAST )  {
            // last buffer
            RXFrameInfo.LastSegmentDesc  = desc;
            RXFrameInfo.SegmentCount++;
            RXFrameInfo.FrameLength += (len-4);     // exclude CRC
            return 1;
        } else {
            // intermediary
            RXFrameInfo.SegmentCount++;
            RXFrameInfo.FrameLength += bsize;
        }

        desc = (ETH_DMADescriptor *) desc->Buffer2NextDescAddr;
        if( desc == ETH_RXDescriptors )
            break;
    }
    return -1;
}



/**
 * @brief   Check if a frame has been received
 *
 * @note    Specially the last buffer must be received
 *
 * @note    If not, set XXXX to point to the next buffer
 */
static int
ETH_CheckReception(void) {
ETH_DMADescriptor *desc;

    desc = ETH_RXDescriptors;
    if( (desc->Status&ETH_RXDESC_OWN) != 0 )  {
        return 0;
    }

    if( (desc->Status&ETH_RXDESC_FIRST) && (desc->Status&ETH_RXDESC_LAST) ) {
        // Only one buffer
        RXFrameInfo.FirstSegmentDesc = desc;
        RXFrameInfo.LastSegmentDesc  = desc;
        RXFrameInfo.SegmentCount = 1;
        return 1;
    } else if( desc->Status&ETH_RXDESC_FIRST ) {
        // First buffer
        RXFrameInfo.FirstSegmentDesc = desc;
        RXFrameInfo.LastSegmentDesc  = 0;
        RXFrameInfo.SegmentCount = 1;
    } else if ( desc->Status&ETH_RXDESC_LAST ) {
        // Last buffer
        RXFrameInfo.LastSegmentDesc = desc;
        RXFrameInfo.SegmentCount++;
        return 1;
    } else {
        // Middle buffer
        RXFrameInfo.SegmentCount++;
    }
    // Point to next
    ETH_RXDescriptors = (ETH_DMADescriptor *) desc->Buffer2NextDescAddr;
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


    return 1;
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
        ETH_Callbacks.FrameReceived = pFunction;
        break;
    case ETH_CALLBACK_FRAMETRANSMITTED:
        ETH_Callbacks.FrameTransmitted = pFunction;
        break;
    case ETH_CALLBACK_ERRORDETECTED:
        ETH_Callbacks.ErrorDetected = pFunction;
        break;
    }

    return;
}

