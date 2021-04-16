/**
 * @file     main.c
 * @brief    Blink LEDs using Systick and CMSIS (Heavy use of macros)
 * @version  V1.0
 * @date     06/10/2020
 *
 * @note     The blinking frequency depends on core frequency
 * @note     Direct access to registers
 * @note     No library used
 *
 *
 ******************************************************************************/

#include <stdio.h>
#include "stm32f746xx.h"
#include "system_stm32f746.h"
#include "led.h"
#include "uart.h"
#include "ucos_ii.h"

#define DELAYLED                500
#define DELAYUART               1000


/**
 * Stacks for tasks
 */
//{
static OS_STK TaskStartStack[APP_CFG_STARTUP_TASK_STK_SIZE];
static OS_STK TaskLEDStack[TASKLED_STK_SIZE];
static OS_STK TaskUARTStack[TASKUART_STK_SIZE];


/**
 * @brief  Task for blinking LED
 *
 * @note   LED_Init must be called before
 */

void TaskLED(void *param) {

    while(1) {
        LED_Toggle();
        OSTimeDly(DELAYLED);
    }
}

/**
 * @brief  Task for sending a char
 *
 * @note   LED_Init must be called before
 */

void TaskUART(void *param) {

    while(1) {
        UART_WriteChar(UART_1,'*');
        OSTimeDly(DELAYUART);
    }
}

/**
 * @brief   Board Support Package (BSP)
 */
typedef unsigned int CPU_INT32U;

void  OS_CPU_TickInit (CPU_INT32U  tick_rate)
{
    CPU_INT32U  cnts;
    CPU_INT32U  cpu_freq;

    cpu_freq = SystemCoreClock;
    cnts     = (cpu_freq / tick_rate);                          /* Calculate the number of SysTick counts               */

    OS_CPU_SysTickInit(cnts);                                   /* Call the Generic OS Systick initialization           */
}

/**
 * @brief  Task for starting other tasks
 *
 * @note   It is recommended to create tasks when uc/os is already running.
 *         This enable the calibration of Stats module.
 */


void TaskStart(void *param) {

    // Initialize the Tick interrupt (CMSIS way)
    SysTick_Config(SystemCoreClock/OS_TICKS_PER_SEC);

    // Initialize the Tick interrupt (uCOS way)
    OS_CPU_TickInit(OS_TICKS_PER_SEC);


#if (OS_TASK_STAT_EN > 0)
    OSStatInit();                                               // Determine CPU capacity
#endif

    // Create a task to blink LED 0
    OSTaskCreate(   TaskLED,                                    // Pointer to task
                    (void *) 0,                                 // Parameter
                    (void *) &TaskLEDStack[TASKLED_STK_SIZE-1], // Initial value of SP
                    TASKLED_PRIO);                              // Task Priority/ID

    // Create a task to blink LED 1
/*    OSTaskCreate(   TaskUART,                                   // Pointer to task*/
/*                    (void *) 0,                                 // Parameter*/
/*                    (void *) &TaskUARTStack[TASKUART_STK_SIZE-1],// Initial value of SP*/
/*                    TASKUART_PRIO);                             // Task Priority/ID*/



    OSTaskDel(OS_PRIO_SELF);                                    // Kill itself. Task should never return
}



/**
 * @brief   main
 *
 * @note    Initializes GPIO and blinks LED
 *
 *
 */

int main(void) {
static const uint32_t uartconfig =  UART_NOPARITY | UART_8BITS | UART_STOP_2 |
                                    UART_BAUD_9600;


    /* Configure clock to 200 MHz */
    SystemSetCoreClock(CLOCKSRC_PLL,1);

    // Configure LEDs
    LED_Init();
    LED_Set();

    // Configure UART
    UART_Init(UART_1,uartconfig);

    // Initialize uc/os II
    OSInit();

    // Create a task to start the other tasks
    OSTaskCreate(   TaskStart,                                          // Pointer to function
            (void *) 0,                                                 // Parameter for task
            (void *) &TaskStartStack[APP_CFG_STARTUP_TASK_STK_SIZE-1],  // Initial value of SP
            APP_CFG_STARTUP_TASK_PRIO);                                 // Task Priority/ID

    // Effectively starting uC/OS
    __enable_irq();

    // Enter uc/os and never returns
    OSStart();
}
