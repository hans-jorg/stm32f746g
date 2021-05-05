
/**
 ** @file     uart.h
 ** @brief    Hardware Abstraction Layer (HAL) for UART
 ** @version  V1.0
 ** @date     23/01/2016
 **
 ** @note     Direct access to registers
 ** @note     No library except CMSIS is used
 ** @note     Only asynchronous communication
 **
 **/

#include "stm32f746xx.h"
#include "system_stm32f746.h"
#include "gpio.h"
#include "uart.h"
#include "fifo.h"

/**
 ** @brief Bit manipulation macros
 **/
//@{
#define BIT(N)                      (1UL<<(N))
#define BITMASK(M,N)                ((BIT((M)-(N)+1)-1)<<(N))
#define BITVALUE(V,N)               ((V)<<(N))
#define SETBITFIELD(VAR,MASK,VAL)   (VAR)=(((VAR)&~(MASK))|(VAL))
//@}

/**
 ** @brief Info and data area for UARTS
 **/
typedef struct {
    USART_TypeDef           *device;
    GPIO_PinConfiguration   txpinconf;
    GPIO_PinConfiguration   rxpinconf;
    struct {
    unsigned            irqlevel :5;
    unsigned            irqn     :10;
    unsigned            useinputfifo:1;
    unsigned            useoutputfifo:1;
    } conf;
    // Could be unions
    FIFO                    inputfifo;
    FIFO                    outputfifo;
    char                    inputbuffer;
    char                    outputbuffer;
} UART_Info;

/*
 * Default FIFO area
 */
#define INPUTAREASIZE    (16)
#define OUTPUTAREASIZE   (16)

DECLARE_FIFO_AREA(inputarea,INPUTAREASIZE);
DECLARE_FIFO_AREA(outputarea,OUTPUTAREASIZE);

/**
 * @brief   Interrupt level for UARTs
 */
#define INTLEVEL 6

/**
 ** @brief  List of known UARTs
 **
 ** @note   There are pin alternatives for most of UARTs
 **/
//@{
static UART_Info uarttab[] = {
/* Device          txconfig                            rxconfig                                   */
/*            Port  Pin AF  M  O  S  P  I   Port  Pin AF  M   O  S  P  I                          */
{ USART1,   { GPIOA, 9, 7, 2, 1, 1, 0, 0 }, { GPIOB, 7, 7, 2, 1, 1, 0, 0 }, INTLEVEL, USART1_IRQn },
{ USART2,   { GPIOA, 2, 7, 2, 1, 1, 0, 0 }, { GPIOA, 3, 7, 2, 1, 1, 0, 0 }, INTLEVEL, USART2_IRQn },
{ USART3,   { GPIOD, 8, 7, 2, 1, 1, 0, 0 }, { GPIOD, 9, 7, 2, 1, 1, 0, 0 }, INTLEVEL, USART3_IRQn },
{ UART4,    { GPIOC,10, 8, 2, 1, 1, 0, 0 }, { GPIOC,11, 8, 2, 1, 1, 0, 0 }, INTLEVEL, UART4_IRQn  },
{ UART5,    { GPIOC,12, 7, 2, 1, 1, 0, 0 }, { GPIOD, 2, 8, 2, 1, 1, 0, 0 }, INTLEVEL, UART5_IRQn  },
{ USART6,   { GPIOC, 6, 8, 2, 1, 1, 0, 0 }, { GPIOC, 7, 8, 2, 1, 1, 0, 0 }, INTLEVEL, USART6_IRQn },
{ UART7,    { GPIOE, 8, 8, 2, 1, 1, 0, 0 }, { GPIOE, 7, 8, 2, 1, 1, 0, 0 }, INTLEVEL, UART7_IRQn  },
{ UART8,    { GPIOE, 1, 8, 2, 1, 1, 0, 0 }, { GPIOE, 0, 8, 2, 1, 1, 0, 0 }, INTLEVEL, UART8_IRQn  }
};
static const int uarttabsize = sizeof(uarttab)/sizeof(UART_Info)-1;
//@}

/**
 * @brief   Enable clock for UART
 */
void UART_EnableClock(USART_TypeDef *uart) {

    if ( uart == USART1 )       RCC->APB2ENR |= RCC_APB2ENR_USART1EN;
    else if ( uart == USART2 )  RCC->APB1ENR |= RCC_APB1ENR_USART2EN;
    else if ( uart == USART3 )  RCC->APB1ENR |= RCC_APB1ENR_USART3EN;
    else if ( uart == UART4 )   RCC->APB1ENR |= RCC_APB1ENR_UART4EN;
    else if ( uart == UART5 )   RCC->APB1ENR |= RCC_APB1ENR_UART5EN;
    else if ( uart == USART6 )  RCC->APB2ENR |= RCC_APB2ENR_USART6EN;
    else if ( uart == UART7 )   RCC->APB1ENR |= RCC_APB1ENR_UART7EN;
    else if ( uart == UART8 )   RCC->APB1ENR |= RCC_APB1ENR_UART8EN;
}


/**
 * @brief   Interrupt processing
 *
 * @note    Identical to all uarts/usarts
 */
static void ProcessInterrupt(int un) {
USART_TypeDef  *uart;

    uart = uarttab[un].device;

    /* Receiving  */
    if( uart->ISR & USART_ISR_RXNE  ) { // RX not empty
        if( uarttab[un].conf.useinputfifo ) {
            /* Multibyte buffer */
            fifo_insert(uarttab[un].inputfifo,uart->RDR);
        } else {
            /* Single byte buffer */
            uarttab[un].inputbuffer = uart->RDR;
        }
    }
    /* Transmitting */
    if( uart->ISR & (USART_ISR_TC|USART_ISR_TXE)  ) { // TX completed or TX buffer empty
        if( uarttab[un].conf.useoutputfifo ) {
            /* Multibyte buffer */
            if( fifo_empty(uarttab[un].outputfifo) ) {
                uart->CR1 &= ~(USART_CR1_TXEIE|USART_CR1_TCIE);
            } else {
                uart->CR1 |= (USART_CR1_TXEIE|USART_CR1_TCIE);
                uart->TDR = fifo_remove(uarttab[un].outputfifo);
            }
        } else {
            /* Single byte buffer */
            if ( uarttab[un].outputbuffer == 0 ) { // no more data
                //uart->RQR = USART_RQR_TXFRQ;    // Repeated code!!! Arghh!!!
                uart->CR1 &= ~(USART_CR1_TXEIE|USART_CR1_TCIE);
            } else {
                uart->TDR = uarttab[un].outputbuffer;
                uarttab[un].outputbuffer = 0;
            }
        }
    }
    uart->ICR = 0x00021B5F;     // Clear all pending interrupts

}
/**
 ** @brief  Interrupt routines for USART and UART
 **/
///@{

/// IRQ Handler for USART1
void USART1_IRQHandler(void) {

    ProcessInterrupt(UART_1);
}

/// IRQ Handler for USART2
void USART2_IRQHandler(void) {

    ProcessInterrupt(UART_2);
}

/// IRQ Handler for USART3
void USART3_IRQHandler(void) {

    ProcessInterrupt(UART_3);
}

/// IRQ Handler for UART4
void UART4_IRQHandler(void) {

    ProcessInterrupt(UART_4);
}

/// IRQ Handler for UART5
void UART5_IRQHandler(void) {

    ProcessInterrupt(UART_5);
}


/// IRQ Handler for USART6
void USART6_IRQHandler(void) {

    ProcessInterrupt(UART_6);
}


/// IRQ Handler for UART7
void UART7_IRQHandler(void) {

    ProcessInterrupt(UART_7);
}

/// IRQ Handler for UART8
void UART8_IRQHandler(void) {

    ProcessInterrupt(UART_8);
}
///@}

/**
 ** @brief UART Initialization Simplified
 **
 ** @note  Use defines in uart.h to configure the uart, or'ing the parameters
 **/
int
UART_Init(int uartn, unsigned config) {
FIFO in;
FIFO out;

    in  = fifo_init(inputarea,INPUTAREASIZE);
    out = fifo_init(outputarea,OUTPUTAREASIZE);

    return UART_InitExt(uartn,config,in,out);
}


/**
 ** @brief UART Initialization Extended
 **
 ** @note  Use defines in uart.h to configure the uart, or'ing the parameters
 **/
int
UART_InitExt(int uartn, unsigned config, FIFO in, FIFO out) {
uint32_t baudrate,div,t,over;
USART_TypeDef * uart;
uint32_t uartfreq;
uint32_t cr1,cr2,cr3,ckcfgr;

    if( uartn >= uarttabsize ) return -1;

    // Configure FIFO and buffer
    uarttab[uartn].inputfifo    = in;
    uarttab[uartn].outputfifo   = out;
    uarttab[uartn].inputbuffer  = 0;
    uarttab[uartn].outputbuffer = 0;
    if( in )
        uarttab[uartn].conf.useinputfifo = 1;
    if( out )
        uarttab[uartn].conf.useoutputfifo = 1;

    // Configure pins
    GPIO_ConfigureSinglePin(&uarttab[uartn].txpinconf);
    GPIO_ConfigureSinglePin(&uarttab[uartn].rxpinconf);

    // Configure clock for UxARTy at RCC DCKCFGR2
    ckcfgr = RCC->DCKCFGR2;

    // Get pointer to UART registers
    uart = uarttab[uartn].device;

    // Select clock source
    uartfreq = 0;
    ckcfgr &= ~BITVALUE(3,uartn*2);
    switch(config&UART_CLOCK_M) {
    case UART_CLOCK_APB:
        ckcfgr |= BITVALUE(0,uartn*2);
        uartfreq = SystemGetAPB1Frequency();
        break;
    case UART_CLOCK_SYSCLK:
        ckcfgr |= BITVALUE(1,uartn*2);
        uartfreq = SystemCoreClock;
        break;
    case UART_CLOCK_HSI:
        ckcfgr |= BITVALUE(2,uartn*2);
        uartfreq = HSI_FREQ;
        break;
    case UART_CLOCK_LSE:
        ckcfgr |= BITVALUE(3,uartn*2);
        uartfreq = LSE_FREQ;
        break;
    }
    RCC->DCKCFGR2 = ckcfgr;

    // Enable Clock
    UART_EnableClock(uart);


    // Configuration only with disabled UAR
    uart->CR1 &= ~USART_CR1_UE;


    // Configure UART CR1
    cr1 = uart->CR1;
    cr1 &= ~(USART_CR1_M|USART_CR1_OVER8|USART_CR1_PCE|USART_CR1_PS|USART_CR1_UE);

    // data length
    switch( config&UART_SIZE_M ) {
    case UART_8BITS:                  ; break;
    case UART_7BITS: cr1 |= USART_CR1_M0; break;
    case UART_9BITS: cr1 |= USART_CR1_M1; break;
    default:
        return 2;
    }
    // parity
    switch( config&UART_PARITY_M ) {
    case UART_NOPARITY:                                    ; break;
    case UART_ODDPARITY:  cr1 |= USART_CR1_PCE|USART_CR1_PS; break;
    case UART_EVENPARITY: cr1 |= USART_CR1_PCE;              break;
    }
    if( config&UART_OVER8 ) {
        cr1 |= USART_CR1_OVER8;
        over = 8;
    } else {
        cr1 &= ~USART_CR1_OVER8;
        over = 16;
    }

    // Configure UART CR2 register
    cr2 = uart->CR2;
    cr2 &= ~USART_CR2_STOP;

    // stop bits
    switch( config&UART_STOP_M ) {
    case UART_STOP_1:
        cr2 |= 0;
        break;
    case UART_STOP_0_5:
        cr2 |= USART_CR2_STOP_0;
        break;
    case UART_STOP_2:
        cr2 |= USART_CR2_STOP_1;
        break;
    case UART_STOP_1_5:
        cr2 |= USART_CR2_STOP_0|USART_CR2_STOP_1;
        break;
    default:
        return 3;
    }

    // Configure UART CR3 register
    cr3 = uart->CR3;
    cr3 = 0;

    // Configure UART BRR register (baudrate)
    baudrate = ((config&UART_BAUD_M)>>UART_BAUD_P);

    if( over == 16 ) {
        div = uartfreq/baudrate;
        uart->BRR = div;
    } else {
        div = 2*uartfreq/baudrate;
        uart->BRR = (div&~0xF)|((div&0xF)>>1);
    }

    // Set configuration
    uart->CR1 = cr1;
    uart->CR2 = cr2;
    uart->CR3 = cr3;

    // Enable interrupts on NVIC
    NVIC_SetPriority(uarttab[uartn].conf.irqn,uarttab[uartn].conf.irqlevel);
    NVIC_ClearPendingIRQ(uarttab[uartn].conf.irqn);
    NVIC_EnableIRQ(uarttab[uartn].conf.irqn);

    // Enable interrupts
    uart->CR1 |= USART_CR1_RXNEIE;          // Enable interrupt when RX not empty
    uart->CR1 |= USART_CR1_TXEIE;           // Enable interrupt when TX is empty

    // Enable UART
    uart->CR1 |= USART_CR1_TE|USART_CR1_RE;
    uart->CR1 |= USART_CR1_UE;
    return 0;
}

/**
 ** @brief UART Send a character
 **
 **/
int
UART_WriteChar(int uartn, unsigned c) {
USART_TypeDef *uart;

    if( uartn >= uarttabsize ) return -1;

    uart = uarttab[uartn].device;
#if 1
    if( uarttab[uartn].conf.useoutputfifo ) {
        /* Multibyte buffer */
        if ( fifo_empty(uarttab[uartn].outputfifo) ) {
            while( (uart->ISR&USART_ISR_TXE)==0 ) {}
            uart->TDR = c;
        } else {
            fifo_insert(uarttab[uartn].outputfifo,c);
        }
    } else {
        /* Singlebyte buffer */
        while ( uarttab[uartn].outputbuffer != 0 ) {}
        if ( (uart->ISR&USART_ISR_TXE)==0 ) {
            uart->TDR = c;
        } else {
            uarttab[uartn].outputbuffer = c;
        }
    }
    // Enable interrupt when transmitting completed or TX buffer empty
    uart->CR1 |= (USART_CR1_TCIE|USART_CR1_TXEIE);
#else
    /* Polling */
    while( (uart->ISR&USART_ISR_TXE)==0 ) {}
    uart->TDR = c;
#endif
    return 0;
}

/**
 ** @brief UART Send a string
 **
 ** @note  It uses UART_WriteChar
 **
 **/
int
UART_WriteString(int uartn, char s[]) {

    if( uartn >= uarttabsize ) return -1;

    while(*s) {
        UART_WriteChar(uartn,*s++);
    }
    return 0;
}

/**
 ** @brief Read a character from UART
 **
 ** @note  It blocks until a character is entered
 **
 **/
int
UART_ReadChar(int uartn) {
USART_TypeDef *uart;
uint32_t c;

    if( uartn >= uarttabsize ) return -1;

    uart = uarttab[uartn].device;

    if( uarttab[uartn].conf.useinputfifo ) {
        while( fifo_empty(uarttab[uartn].inputfifo) ) {}
        c = fifo_remove(uarttab[uartn].inputfifo);
    } else {
        while( uarttab[uartn].inputbuffer == 0 ) {}
        c = uarttab[uartn].inputbuffer;
        uarttab[uartn].inputbuffer = 0;
    }

    if( uart->ISR & USART_ISR_ORE )  {  // overun error
        uart->ICR |= USART_ICR_ORECF;
    }

    return c;
}

/**
 ** @brief Read a character from UART
 **
 ** @note  It doest not block. It return 0
 **
 **/
int
UART_ReadCharNoWait(int uartn) {
USART_TypeDef *uart;
uint32_t c;

    if( uartn >= uarttabsize ) return -1;

    uart = uarttab[uartn].device;

    if( uarttab[uartn].conf.useinputfifo ) {
        if( fifo_empty(uarttab[uartn].inputfifo)) {
            c = 0;
        } else {
            c = fifo_remove(uarttab[uartn].inputfifo);
        }
    } else {
        if( uarttab[uartn].inputbuffer ) {
            c = uarttab[uartn].inputbuffer;
            uarttab[uartn].inputbuffer = 0;
        } else {
            c = 0;
        }
    }

    if( uart->ISR & USART_ISR_ORE )  {  // overun error
        uart->ICR |= USART_ICR_ORECF;
    }

    return c;
}

/**
 ** @brief UART Send a string
 **
 ** @note  It block until "n" characters are entered or
 **        a newline is entered
 **
 ** @note  It uses UART_ReadChar
 **
 **/
int
UART_ReadString(int uartn, char *s, int n) {
int i;

    if( uartn >= uarttabsize ) return -1;

    for(i=0;i<n-1;i++) {
        s[i] = UART_ReadChar(uartn);
        if( s[i] == '\n' || s[i] == '\r' )
            break;
    }
    s[i] = '\0';
    return i;
}

/**
 ** @brief UART Get Status
 **
 ** @note  Return status
 **
 **/
int
UART_GetStatus(int uartn) {
USART_TypeDef *uart;
uint32_t status;

    if( uartn >= uarttabsize ) return -1;

    uart = uarttab[uartn].device;

    status = uart->ISR;

    /* Verify input buffer */
    if ( uarttab[uartn].conf.useinputfifo ) {
        if ( !fifo_empty(uarttab[uartn].inputfifo) )
            status |= UART_RXNOTEMPTY;
    } else {
        if ( uarttab[uartn].inputbuffer )
            status |= UART_RXNOTEMPTY;
    }

    /* Verify output buffer */
    if( uarttab[uartn].conf.useoutputfifo ) {
        if ( fifo_empty(uarttab[uartn].outputfifo) )
            status |= UART_RXNOTEMPTY;
    } else {
        if ( uarttab[uartn].outputbuffer == 0 )
            status |= UART_TXEMPTY;
    }
    return status;

}

/**
 ** @brief UART Get Status
 **
 ** @note  Clear input buffer and wait until output buffer is empty
 **
 **/

 int
 UART_Flush(int uartn) {

     if( uartn >= uarttabsize ) return -1;

    // Flush input buffer
    if( uarttab[uartn].conf.useinputfifo ) {
        fifo_clear(uarttab[uartn].inputfifo);
    } else {
        uarttab[uartn].inputbuffer = 0;
    }

    // Flush out buffer
    if( uarttab[uartn].conf.useoutputfifo ) {
        while (!fifo_empty(uarttab[uartn].outputfifo) ) {}
    } else {
        while ( uarttab[uartn].outputbuffer ) {}
    }

    return 0;
 }
