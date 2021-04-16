Using uC/OS-II
==============

Introduction
------------

uC/OS is a real time kernel introduced by Jean Labrosse in 1992  [1]. uC/OS-II [2] is an ]
upgraded version of the one presented in the magazine and later in a book,

uC/OS-II
--------

The tasks for blinking LED is straightforward. 

    void TaskLED(void *param) {

        while(1) {
            LED_Toggle();
            OSTimeDly(500);
        }
    }'

The tasks must be created when uC/OS is already running. To do it, a single task
must be created before starting it, and it starts the tasks.


    int main(void) {

        /* Configure clock to 200 MHz */
        SystemSetCoreClock(CLOCKSRC_PLL,1);

        // Configure LEDs
        LED_Init();
        LED_Set();

        // Initialize uc/os II
        OSInit();

        // Create a task to start the other tasks
        OSTaskCreate(   TaskStart,
                		(void *) 0,
                		(void *) &TaskStartStack[APP_CFG_STARTUP_TASK_STK_SIZE-1],
                        APP_CFG_STARTUP_TASK_PRIO);

        // Effectively starting uC/OS
        __enable_irq();

        // Enter uc/os and never returns
        OSStart();
    }

The TaskStart is very simple. It creates the tasks and then kills itself.

     void TaskStart(void *param) {

        // Initialize the Tick interrupt (CMSIS way)
        SysTick_Config(SystemCoreClock/OS_TICKS_PER_SEC);

        // Initialize the Tick interrupt (uCOS way)
        OS_CPU_TickInit(OS_TICKS_PER_SEC);

        // Create a task to blink LED 0
        OSTaskCreate(   TaskLED,
                        (void *) 0,
                        (void *) &TaskLEDStack[TASKLED_STK_SIZE-1],
                        TASKLED_PRIO);

		// .....
        
        OSTaskDel(OS_PRIO_SELF);
    }

The priorities are defined the app_cfg.h file.

	...
    #define  APP_CFG_STARTUP_TASK_PRIO          3u
    #define  TASKLED_PRIO                      (10)
    #define  TASKUART_PRIO                     (11)
	...


References
----------

1. A Portable Real-Time Kernel in C. Embedded Systems Programming. May/June 1992.
2. [uC/OS-II](https://www.weston-embedded.com/micrium-books/micrium-books-downloads/category/295-ucos-ii)