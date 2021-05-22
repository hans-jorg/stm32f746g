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
 */
#define ETH_CONFIG_AUTONEGOCIATE                (1)
#define ETH_CONFIG_100BASET                     (2)
#define ETH_CONFIG_10BASET                      (4)
#define ETH_CONFIG_FULLDUPLEX                   (8)
#define ETH_CONFIG_HALFDUPLEX                   (16)

const uint32_t  eth_config = ETH_CONFIG_AUTONEGOCIATE;
uint32_t        eth_status = 0;

// 48 bit MAC address (must have 12 hexadecimal digits))
uint64_t        eth_macaddress = 0x2F85936E2C78L;

/**
 * @brief   ETH Configuration
 */


#define ETH_IRQLevel                            (5)

/**
 * @brief   RX and TX descriptors
 */
///@{
ETH_DMADescriptor *ETH_TXDescriptors = 0;
ETH_DMADescriptor *ETH_RXDescriptors = 0;
static ETH_DMAFrameInfo  RXFrameInfo = {0};
///@}

/**
 * @brief   Callbacks
 */
struct ETH_Callbacks_s ETH_Callbacks = {0};

/**
 * @brief   Defines for configure PHY
 *
 * @note LAN8742 PHY register description
 *
 *   |   #  |        | Description                                     |  Group           |
 *   |------|--------|-------------------------------------------------|------------------|
 *   |   0  | BCR    | Basic Control Register                          | Basic            |
 *   |   1  | BSR    | Basic Status Register                           | Basic            |
 *   |   2  | ID1R   | PHY Identifier 1 Register                       | Extended         |
 *   |   3  | ID2R   | PHY Identifier 2 Register                       | Extended         |
 *   |   4  | ANAR   | Auto Negotiation Advertisement Register         | Extended         |
 *   |   5  | ANLPR  | Auto Negotiation Link Partner Ability Register  | Extended         |
 *   |   6  | ANEPR  | Auto Negotiation Expansion Register             | Extended         |
 *   |   7  | ANNPTXR| Auto Negotiation Next Page TX Register          | Extended         |
 *   |   8  | ANNPRXR| Auto Negotiation Next Page RX Register          | Extended         |
 *   |  13  | MMDACR | MMD Access Control Register                     | Extended         |
 *   |  14  | MMADR  | MMD Access Address/Data Register                | Extended         |
 *   |  16  | EDPDR  | EDPD NLP/Crossover TimeRegister                 | Vendor-specific  |
 *   |  17  | MCSR   | Mode Control/Status Register                    | Vendor-specific  |
 *   |  18  | SMR    | Special Modes Register                          | Vendor-specific  |
 *   |  24  | TDRPDR | TDR Patterns/Delay Control Register             | Vendor-specific  |
 *   |  25  | TDCSR  | TDR Control/Status Register                     | Vendor-specific  |
 *   |  26  | SECR   | Symbol Error Counter Register                   | Vendor-specific  |
 *   |  27  | SCSIR  | Special Control/Status Indications Register     | Vendor-specific  |
 *   |  28  | CLR    | Cable Length Register                           | Vendor-specific  |
 *   |  29  | ISFR   | Interrupt Source Flag Register                  | Vendor-specific  |
 *   |  30  | IMR    | Interrupt Mask Register                         | Vendor-specific  |
 *   |  31  | SCSR   | PHY Special Control/Status Register             | Vendor-specific  |
 */



/** PHY */
#define ETH_PHY_ADDRESS                 1

/** Registers */
///@{
#define ETH_PHY_BCR                             (0)
#define ETH_PHY_BSR                             (1)
#define ETH_PHY_ISFR                           (29)
///@}


/** Delays */
///@{
#define ETH_DELAY_AFTERRESET                1000
#define ETH_DELAY_AFTERAUTONEGOCIATION      1000
#define ETH_DELAY_AFTERCONFIG               1000
#define ETH_DELAY_BETWEENTESTS              1000

#define ETH_RETRIES_LINK                    1000
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



/**
 * @brief   Get Mac Address as a vector of bytes
 *
 * @note    Least significant byte first
 */
void
ETH_GetMACAddress(uint8_t macaddr[6]) {

    macaddr[0] = (eth_macaddress>>0)&0xFF;
    macaddr[1] = (eth_macaddress>>8)&0xFF;
    macaddr[2] = (eth_macaddress>>16)&0xFF;
    macaddr[3] = (eth_macaddress>>24)&0xFF;
    macaddr[4] = (eth_macaddress>>32)&0xFF;
    macaddr[5] = (eth_macaddress>>40)&0xFF;

}

////////////////////////////////// Pin management //////////////////////////////////////////////////

#if ETH_USEGPIO == 1

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
//   {  GPIOG,   2,       11, 0, 0, 3, 1, 0  },       //     ETH_RMII_RXER     AF0 or AF11 ?????
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

#define ETH_AF      (11)
#define ETH_MODE    (2)
#define ETH_OTYPE   (0)
#define ETH_OSPEED  (3)
#define ETH_PUPD    (0)

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
    while( (ETH->MACMIIAR&ETH_MACMIIAR_MB) != 0 ) {}                // TODO: Add timeout

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
    while( (ETH->MACMIIAR&ETH_MACMIIAR_MB) != 0 ) {}                // TODO: Add timeout

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

    ETH_WritePHYRegister(ETH_PHY_BCR,ETH_PHY_BCR_RESET);
    delay(ETH_DELAY_AFTERRESET);
    eth_status = 0;

    if( eth_config&ETH_CONFIG_AUTONEGOCIATE ) {
        // Check status
        retries = ETH_RETRIES_LINK;
        do {
            ETH_ReadPHYRegister(ETH_PHY_BSR,&value);
            delay(ETH_DELAY_BETWEENTESTS);
            retries--;
        } while((retries>0)&&(value&ETH_PHY_BSR_LINKUP)!=ETH_PHY_BSR_LINKUP);
        if( (value&ETH_PHY_BSR_LINKUP) != ETH_PHY_BSR_LINKUP )
            return -1;

        // Configure Autonegociate
        ETH_WritePHYRegister(ETH_PHY_BCR,ETH_PHY_BCR_AUTONEGOCIATION);
        delay(ETH_DELAY_AFTERAUTONEGOCIATION);
        do {
            ETH_ReadPHYRegister(ETH_PHY_BSR,&value);
            delay(ETH_DELAY_BETWEENTESTS);
        } while((value&ETH_PHY_BSR_AUTONEGOCIATIONCOMPLETED)!=ETH_PHY_BSR_AUTONEGOCIATIONCOMPLETED);
        // Get the result
        ETH_ReadPHYRegister(ETH_PHY_BSR,&value);
        // Set status
        if( value&(ETH_PHY_BSR_100BASET_FULLDUPLEX|ETH_PHY_BSR_10BASET_FULLDUPLEX) ) {
            eth_status |= ETH_CONFIG_FULLDUPLEX;
        } else {
            eth_status |= ETH_CONFIG_HALFDUPLEX;
        }
        if( value&(ETH_PHY_BSR_100BASET_FULLDUPLEX|ETH_PHY_BSR_100BASET_HALFDUPLEX) ) {
            eth_status |= ETH_CONFIG_100BASET;
        } else {
            eth_status |= ETH_CONFIG_10BASET;
        }
    } else {
        // Autonegotiation not set. Configure Speed and Duplex Mode
        value = 0;
        if( eth_config&ETH_CONFIG_FULLDUPLEX ) {
            if ( eth_config&ETH_CONFIG_100BASET )  {
                value = ETH_PHY_BSR_100BASET_FULLDUPLEX;
            } else if ( eth_config&ETH_CONFIG_10BASET ) {
                value = ETH_PHY_BSR_10BASET_FULLDUPLEX;
            }
        } else if( eth_config&ETH_CONFIG_HALFDUPLEX ) {
            if ( eth_config&ETH_CONFIG_100BASET )  {
                value = ETH_PHY_BSR_100BASET_HALFDUPLEX;
            } else if ( eth_config&ETH_CONFIG_10BASET ) {
                value = ETH_PHY_BSR_10BASET_HALFDUPLEX;
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
 * @brief  Configure DMA
 *
 * @note Configuration for DMA
 *
 *   Drop TCP/IP Frame on Checksum error enabled
 *   Receive store forward enabled
 *   Flush received frame enable
 *   Transmit store forward enabled
 *   Transmit threshold control set to 64 bytes
 *   Forward error frames disabled
 *   Forward undersized goodframes disabled
 *   Received threshold control set to 64 bytes
 *   Second frame operarted enabled
 *
 *   Address aligned beats enabled
 *   Fixed burst_Enabled
 *   Rx DMA burst length set to 32 beats
 *   TxDMA burst length_set to 32 beats
 *   DMA enhanced descriptor enabled
 *   Descripto length set to 0x0;
 *   DMA arbitration roundrobin RX/TX 1/1
 */

#define DMA_PBL                         (32)
#define DMA_PM                          (0)
#define DMA_DSL                         (0)
#define DMA_TTC                         (0)

#define ETH_DELAY_AFTERREGWRITE         1000
#define ETH_DELAY_AFTERFLUSH            10
#define ETH_DELAY_AFTERMAC              10

static int ETH_ConfigureDMA(void) {

    // Configure DMA mode register
    uint32_t dmabmr = ETH->DMABMR;

    // Clear all fields
    dmabmr = 0;

    // Configure
    dmabmr |=  ETH_DMABMR_AAB
              |ETH_DMABMR_FB
              |(DMA_PM<<ETH_DMABMR_FPM_Pos)
              |(DMA_PBL<<ETH_DMABMR_PBL_Pos)
              |ETH_DMABMR_EDE
              |(DMA_DSL<<ETH_DMABMR_DSL_Pos)
              ;

    // Configure DMABMR
    ETH->DMABMR = dmabmr;
    delay(ETH_DELAY_AFTERREGWRITE);


    // Configure operation mode
    uint32_t dmaomr = ETH->DMAOMR;

    // clear fields
    dmaomr = 0;

    // Configure
    dmaomr |= ETH_DMAOMR_DTCEFD
             |ETH_DMAOMR_RSF
             |ETH_DMAOMR_TSF
             |(DMA_TTC<<ETH_DMAOMR_TTC_Pos)
             ;
    // Configure DMAOMR
    ETH->DMAOMR = dmaomr;

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


/*
 * @brief   rev64
 *
 * @note    reverse byte order of a 64 bit integer
 */


static uint64_t rev64(uint64_t x) {
uint32_t xh,xl;

    xh = x>>32;
    xl = x;
    xl = __REV(xl);
    xh = __REV(xh);
    x = (((uint64_t) xh)<<32) | (uint64_t) xl;
    return x;

}


/**
 * @brief   ETH ConfigureMAC
 *
 * @note  General configuration
 *    Watchdog enabled
 *    Jabber enabled
 *    Interframe gap set to 96
 *    Carriersense enabled
 *    Receive own enable
 *    Loopback disabled
 *    Checksum done by hardware
 *    Retry transmission disabled
 *    Automatic PAD CRC strip disabled
 *    Backoff limit set to 10
 *    Deferral check disabled
 *
 * @note Filter configuration
 *
 *    Receive all disabled
 *    Source address filter disabled
 *    Block all control frames
 *    Broadcastframes reception_enabled
 *    Destination filter normal
 *    Promiscuous mode disabled
 *    Multicast frames perfect
 *    Unicast frames filter perfect
 *    Zero quanta pause disabled
 *    Pause low threshold set to -4
 *    Unicast pause frame detect disabled
 *    Receive flow control disabled
 *    Transmit flow control disabled
 *
 * @note  IFG Gap options
 *
 *     | IFG|  # bit times |
 *     |----|--------------|
 *     |  0 |  96          |
 *     |  1 |  88          |
 *     |  2 |  80          |
 *     |  4 |  64          |
 *     |  5 |  56          |
 *     |  6 |  48          |
 *     |  7 |  40          |
 *
 *
 * @note  Back-off limit. Random time delay after collision
 *
 *     | BL  |  Meaning           |
 *     |-----|--------------------|
 *     |  00 | 2^k, k = min(n,10) |
 *     |  01 | 2^k, k = min(n,8)  |
 *     |  10 | 2^k, k = min(n,4)  |
 *     |  11 | 2^k, k = min(n,1)  |
 *
 *     where n is retransmission attempt count
 *
 *
 * @note  Pause low threshold
 *
 *      | PLT  |  Meaning                        |
 *      |------|---------------------------------|
 *      |  00  | Pause time minus 4 slot times   |
 *      |  01  | Pause time minus 28 slot times  |
 *      |  10  | Pause time minus 144 slot times |
 *      |  11  | Pause time minus 256 slot times |
 */

#define MAC_IFGGAP                          0
#define MAC_BL                              2
#define MAC_PCF                             0
#define MAC_PAM                             0
#define MAC_PT                              4
#define MAC_PLT                             0

static int ETH_ConfigureMAC(void) {
    uint32_t maccr = ETH->MACCR;
    maccr &= ~( ETH_MACCR_WD_Msk
               |ETH_MACCR_JD_Msk
               |ETH_MACCR_IFG_Msk
               |ETH_MACCR_CSD_Msk
               |ETH_MACCR_FES_Msk
               |ETH_MACCR_ROD_Msk
               |ETH_MACCR_LM_Msk
               |ETH_MACCR_DM_Msk
               |ETH_MACCR_IPCO_Msk
               |ETH_MACCR_RD_Msk
               |ETH_MACCR_APCS_Msk
               |ETH_MACCR_BL_Msk
               |ETH_MACCR_DC_Msk )
               ;

    // Set defaults
    maccr |= (MAC_IFGGAP<<ETH_MACCR_IFG_Pos)
            |ETH_MACCR_IPCO
            |(MAC_BL<<ETH_MACCR_BL_Pos);

    // Set configuration
    if( eth_config&ETH_CONFIG_100BASET )   maccr |= ETH_MACCR_FES;
    if( eth_config&ETH_CONFIG_FULLDUPLEX ) maccr |= ETH_MACCR_DM;

    // Set configuration
    ETH->MACCR = maccr;

    // Configure filter
    uint32_t macffr = ETH->MACFFR;

    // Clear all fields
    macffr = 0;

    // Set defaults
    macffr |= ETH_MACFFR_HPF
             |(MAC_PCF<<ETH_MACFFR_PCF_Pos)
             |(MAC_PAM<<ETH_MACFFR_PAM_Pos)
             ;

    // Set configuration
    ETH->MACFFR = macffr;

    // Configure flow control
    uint32_t macfcr = ETH->MACFCR;

    // Clear all fields
    macfcr  = 0;

    // set defaults
    macfcr  |= (MAC_PT<<ETH_MACFCR_PT_Pos)
              |(MAC_PLT<<ETH_MACFCR_PLT_Pos)
              ;
    delay(ETH_DELAY_AFTERREGWRITE);

    // Se configuration
    ETH->MACFCR = macfcr;

    // Configure hast tables
    ETH->MACHTHR = 0;
    ETH->MACHTLR = 0;

    // Configure VLAN register
    ETH->MACVLANTR  = 0;

    // Configure MAC Address

    uint64_t ma = rev64(eth_macaddress);

    ETH->MACA0HR = (uint32_t) (ma>>32)|(1<<31);
    ETH->MACA0LR = (uint32_t) ma;

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


void ETH_InitializeDescriptorsTX(ETH_DMADescriptor *desc, int count, char *area) {
int i;

    ETH_TXDescriptors = desc;
    for(i=0;i<count;i++) {
        desc->Status    = ETH_TXDESC_CHAINED | ETH_TXDESC_CIC;
        desc->ControlBufferSize = 0;
        desc->Buffer1Addr = (uint32_t) (area + i*ETH_TXBUFFER_SIZE);
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

    ETH_RXDescriptors = desc;
    for(i=0;i<count;i++) {
        desc->Status    = ETH_RXDESC_OWN;
        desc->ControlBufferSize = ETH_RXBUFFER_SIZE | ETH_RXDESC_CHAINED;
        desc->Buffer1Addr =  (uint32_t) (area + i*ETH_RXBUFFER_SIZE);
        desc->Buffer2NextDescAddr =  (uint32_t) (desc + ((i+1)%count));
        desc++;
    }

    // Write table address to the ETH interface
    ETH->DMARDLAR = (uint32_t) desc;
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
uint32_t media = 1;

    // Configure pins
    ConfigureETHPins();

    // Enable SYSCFG clock to select the ethernet PHY interface to be used
    RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;
    __NOP();
    __NOP();
    __DSB();

    //  Select RMII mode
    SYSCFG->PMC = (SYSCFG->PMC&~(SYSCFG_PMC_MII_RMII_SEL))|(media<<SYSCFG_PMC_MII_RMII_SEL_Pos);

    // Enable clocks for ETH
    ETH_EnableClock(ETH_CLOCK_MAC|ETH_CLOCK_MACRX|ETH_CLOCK_MACTX);

    // Reset ETH system
    ETH->DMABMR |= ETH_DMABMR_SR;
    // Wait until reset
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
void ETH_EnableTransmissionDMA(void);
void ETH_DisableTransmissionDMA(void);
void ETH_EnableReceptionDMA(void);
void ETH_DisableReceptionDMA(void);
void ETH_EnableTransmissionMAC(void);
void ETH_DisableTransmissionMAC(void);
void ETH_EnableReceptionMAC(void);
void ETH_DisableReceptionMAC(void);


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
 * @brief    Receive data
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

////////////////// IRQ Handler //////////////////////////////////////////////////////////////////

/**
 * @brief   ETH interrupt handler
 */

void ETH_IRQHandler(void) {

    // Check if a frame was received
    if( ETH->DMASR&ETH_DMASR_RS ) {
        if( ETH_Callbacks.FrameReceived ) ETH_Callbacks.FrameReceived(0);
        ETH->DMASR = ETH_DMASR_RS;

    }
    // Check if a frame was transmitted
    if( ETH->DMASR&ETH_DMASR_TS ) {
        if( ETH_Callbacks.FrameTransmitted ) ETH_Callbacks.FrameTransmitted(0);
        ETH->DMASR = ETH_DMASR_TS;
    }

    // Check error
    if( ETH->DMASR&ETH_DMASR_AIS ) {
        if( ETH_Callbacks.ErrorDetected ) ETH_Callbacks.ErrorDetected(0);
        ETH->DMASR = ETH_DMASR_AIS;
    }

    // Clear interrupt summary
    ETH->DMASR = ETH_DMASR_NIS;
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

