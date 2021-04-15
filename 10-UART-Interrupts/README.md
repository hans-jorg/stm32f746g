UART using interrupts
=====================

Introduction
------------

There is a serial connection between the STM32F746G MCU and the Debug MCU. The Debug MCU
creates a CDC (communication device class) virtual device that can be accessed from the PC
using a terminal program.


                                 |------------------------------------------------------------|
                                 |                                                            |
                                 |   ----------------    SWD      |----------------|          |
                 USB             |   |              |-------------|                |          |
    PC |-------------------------|---|  Debug MCU   |             |  STM32F746G    |          |
                                 |   |              |-------------|                |          |
                                 |   ----------------  serial     |----------------|          |
                                 |                                                            |
                                 |------------------------------------------------------------|

Board signal |   SMT32F746G Pin   |  Jumper  |  Debug signal | Debug MCU
-------------|--------------------|----------|---------------|--------------
VCP_TX       |  PA9 / USART1_TX   |   SB13   |   STLINK_RX   |  PA3
VCP_RX       |  PB7 / USART1_RX   |   SB12   |   STLINK_TX   |  PA2

This connect run at 9600 bps with 8 bits, no parity and 1 stop bit.

The best way to interface to a serial interface is using interrupts. The interrupt routine for receiving data is straightforward, read the character from the UART and put it in a FIFO (First-In First-Out) buffer. The reading routines get the data from the FIFO. The transmission work in a similar way. If there is no data in the FIFO, just send it. If there is data in the FIFO, put the data to be transmitted there. The interrupt routine for transmitting data seeks for it in the FIFO, and if there is data there, send it.

There are eight UARTs ((Universal Asynchronous Receiver Transmitter)) 
 in the STM32F756G MCU. Some of the can handle synchronous communications too and
so they are called USARTs (Universal Synchronous/Asynchronous Receiver Transmitter)

UART HAL
--------

The HAL is composed of the following functions:

* int UART_Init(int uartn, uint32_t config)
* int UART_WriteChar(int uartn, uint32_t c)
* int UART_WriteString(int uartn, char s[])
* int UART_ReadChar(int uartn)
* int UART_ReadString(int uartn, char *s, int n)
* int UART_GetStatus(int uartn)

There are many symbols defined to be used as parameters.

UART    | USART
--------|------------------
UART_1  | USART_1
UART_2  | USART_2
UART_3  | USART_3
UART_4  |
UART_5  |
UART_6  | USART_6
UART_7  |
UART_8  |

The symbols below are used to compound the *config* word.

*Parity*  
UART_NOPARITY   (default)  
UART_EVENPARITY  
UART_ODDPARITY 

*Size*  
UART_8BITS   (default)    
UART_9BITS  
UART_7BITS  

*Stop bits*  
UART_STOP_1  
UART_STOP_0_5  
UART_STOP_2    (default)   
UART_STOP_1_5 

*Over clocking*  
UART_OVER8  
UART_OVER16    (default)  

*Clock source*  
UART_CLOCK_APB  
UART_CLOCK_SYSCLK  
UART_CLOCK_HSI      (default)  
UART_CLOCK_LSE  

*baudrate*  
UART_BAUD_150  
UART_BAUD_300  
UART_BAUD_600  
UART_BAUD_1200  
UART_BAUD_2400  
UART_BAUD_4800  
UART_BAUD_9600  
UART_BAUD_19200  
UART_BAUD_38400  
UART_BAUD_57600  
UART_BAUD_115200  


References
----------

1. [STM32F75xxx and STM32F74xxx advanced Arm ® -based 32-bit MCUs Reference Manual - RM0385](https://www.st.com/resource/en/reference_manual/dm00124865-stm32f75xxx-and-stm32f74xxx-advanced-arm-based-32-bit-mcus-stmicroelectronics.pdf)
