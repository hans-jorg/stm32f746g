/**
 * @file    uart.c
 *
 * @author  hans
 */


#include "stm32f746xx.h"
#include "system_stm32f746.h"

/**
 *  @note   Configures UART1 to 9600, 8 bits, no parity, 2 stop bits
 *
 *  @note   UART1 uses APB1 Clock. Clock enable in APB2ENR
 *
 *  @note   It uses GPIO A (PA9) and B (PB7) with alternate function AF7
 *
 *  @note
 */




void UART_Init(void) {

    // Configure clock for UART1
    // Enable AHB1 and APB2 clocks

    RCC->APB2ENR |= RCC_APB2ENR_USART1EN;

    // Configure pins for UART1
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN|RCC_AHB1ENR_GPIOBEN;
    GPIOA->AFR[1]  = (GPIOA->AFR[1]&GPIO_AFRH_AFRH1_Msk)|(7<<GPIO_AFRH_AFRH1_Pos); //PA9
    GPIOB->AFR[0]  = (GPIOB->AFR[0]&GPIO_AFRL_AFRL7_Msk)|(7<<GPIO_AFRL_AFRL7_Pos); //PB7
    // Configure transmission parameters

}

