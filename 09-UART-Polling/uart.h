#ifndef UART_H
#define UART_H
/**
 * @file     uart.h
 * @brief    Hardware Abstraction Layer (HAL) for UARTs
 * @version  V1.0
 * @date     23/01/2016
 *
 * @note     Direct access to registers
 * @note     No library except CMSIS is used
 * @note     No support for synchronous communication
 * @note     No interrupts (for now)
 *
 ******************************************************************************/

#ifndef UART_BIT
#define UART_BIT(N) (1U<<(N))
#define UART_BITFIELD(V,P)   ((V)<<(P))
#endif

/**
 ** @brief  Parameters to configure UART
 **
 ** @note   OR'ed into a 32 bit integer
 **
 ** @note   0 value means the default value
 **/


/**
 *  @brief Parity   (bits 1-0)
 */
///@{
#define UART_PARITY_M       (0x3)
#define UART_PARITY_P       (0)
#define UART_NOPARITY       (0x0)
#define UART_EVENPARITY     (0x1)
#define UART_ODDPARITY      (0x2)
///@}

/**
 *  @brief Size   (bits 3-2)
 */
///@{
#define UART_SIZE_M         (0xC)
#define UART_SIZE_P         (2)
#define UART_8BITS          (0x0)
#define UART_9BITS          (0x8)
#define UART_7BITS          (0xC)
///@}

/**
 *  @brief Stop bits   (bits 6-4)
 */
///@{
#define UART_STOP_M         (0x70)
#define UART_STOP_P         (4)
#define UART_STOP_1         (0x10)
#define UART_STOP_0_5       (0x20)
#define UART_STOP_2         (0x00)
#define UART_STOP_1_5       (0x40)
///@}

/**
 *  @brief Over clocking   (bit 7)
 */
///@{
#define UART_OVER_M         (0x80)
#define UART_OVER_P         (7)
#define UART_OVER8          (0x80)
#define UART_OVER16         (0x00)

/**
 *  @brief  Clock source (bit 9-8)
 */
///@{
#define UART_CLOCK_M        (0x300)
#define UART_CLOCK_P        (8)
#define UART_CLOCK_APB      (0x200)
#define UART_CLOCK_SYSCLK   (0x100)
#define UART_CLOCK_HSI      (0x000)
#define UART_CLOCK_LSE      (0x300)
///@}

/**
 *  @brief  Baud rate (bit 31-12)
 *
 *  @note   Values up to 2^19-1 = 1048575
 */
///@{
#define UART_BAUD_M         (0xFFFFF000)
#define UART_BAUD_P         (12)
#define UART_BAUD_150       (UART_BITFIELD(150,12))
#define UART_BAUD_300       (UART_BITFIELD(300,12))
#define UART_BAUD_600       (UART_BITFIELD(600,12))
#define UART_BAUD_1200      (UART_BITFIELD(1200,12))
#define UART_BAUD_2400      (UART_BITFIELD(2400,12))
#define UART_BAUD_4800      (UART_BITFIELD(4800,12))
#define UART_BAUD_9600      (UART_BITFIELD(9600,12))
#define UART_BAUD_19200     (UART_BITFIELD(19200,12))
#define UART_BAUD_38400     (UART_BITFIELD(38400,12))
#define UART_BAUD_57600     (UART_BITFIELD(57600,12))
#define UART_BAUD_115200    (UART_BITFIELD(115200,12))
//@}

/**
 * @brief Id for USART/USARTs
 *
 *   Device     |   Id
 *   -----------|-----------------
 *   USART1     |   UART_1 or USART1
 *   USART2     |   UART_2 or USART2
 *   USART3     |   UART_3 or USART3
 *   UART4      |   UART_4
 *   UART5      |   UART_5
 *   USART6     |   UART_6 or USART6
 *   UART7      |   UART_7
 *   UART8      |   UART_8
 */
//@{
#define UART_1      (0)
#define USART_1     (0)
#define UART_2      (1)
#define USART_2     (1)
#define UART_3      (2)
#define USART_3     (2)
#define UART_4      (3)
#define UART_5      (4)
#define UART_6      (5)
#define USART_6     (5)
#define UART_7      (6)
#define UART_8      (7)

//@}

/// Symbols returned by GetStatus
//@{
#define UART_TXCOMPLETE UART_BIT(6)
#define UART_RXNOTEMPTY UART_BIT(5)
#define UART_TXEMPTY    UART_BIT(7)
#define UART_RXBUSY     UART_BIT(16)
#define UART_RXFERROR   UART_BIT(1)
#define UART_RXPERROR   UART_BIT(0)
///@}

int UART_Init(int uartn, uint32_t config);
int UART_WriteChar(int uartn, uint32_t c);
int UART_WriteString(int uartn, char s[]);

int UART_ReadChar(int uartn);
int UART_ReadCharNoWait(int uartn);
int UART_ReadString(int uartn, char *s, int n);
int UART_GetStatus(int uartn);

#endif // UART_H
