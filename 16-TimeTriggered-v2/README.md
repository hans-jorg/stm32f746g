Using an enhanced Time Triggered Architecture
=============================================


Introduction
------------

Pont in his book "Patterns for Time-Triggered Embedded Systems" book presents
a simple way to implement a real-time embedded system, without using multitasking.
In a recent book, called "The Engineering of Reliable Embedded Systems", it shows an enhanced version.

As before it requires only one stack and uses many run-to-completion (RTC) functions.
It is also very reliable, due to its simplicity.


Time Triggered System
---------------------

There is a structure for each task. TASK_MAXCNT define the maximal number of tasks.

    typedef struct {
        void    (*task)(void);  // pointer to function
        // time unit is ticks
        uint32_t    period;     // period, i.e. time between activations
        uint32_t    delay;      // time to next activation
        uint32_t    runcnt;     // if greater than 1, overrun
    } TaskInfo;

    static TaskInfo taskinfo[TASK_MAXCNT];

There are trivial routines to add and remove task from the above array. When adding a task, its period and
delay must be provided.

The processing is done in two parts. *Task_Update* is run during a periodic interrupt.

    void Task_Update(void) {

    	task_tickcounter++;
    	return;
	}
    

During the main cycle, the *Task_Dispatch* is run.

    Task_Dispatch(void) {
    int i;
    TaskInfo *p;
    uint32_t dispatch;

        __disable_irq();
        if( task_tickcounter > 0 ) {
            task_tickcounter--;
            dispatch = 1;
        }
        __enable_irq();

        while( dispatch ) {
            for(i=0;i<TASK_MAXCNT;i++) {
                p = &taskinfo[i];
                if( p->task ) {
                    if( p->delay == 0 ) {
                        p->task();      		// call task function
                        if( p->period == 0 ) { 	// one time tasks are dangerous
                            Task_Delete(i);
                        } else {
                            p->delay = p->period;
                        }
                    } else {
                        p->delay--;
                    }
                }
            }
            __disable_irq();
            if( task_tickcounter > 0 ) {
                task_tickcounter--;
                dispatch = 1;
            } else {
                dispatch = 0;
            }
            __enable_irq();
        }
        return 0;
    }


References
----------

1. [Patterns for Time-Triggered Embedded Systems](https://www.safetty.net/publications/pttes)
2. [The Engineering of Reliable Embedded Systems](https://www.safetty.net/publications/the-engineering-of-reliable-embedded-systems-second-edition)