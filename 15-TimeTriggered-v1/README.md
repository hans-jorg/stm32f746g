Using a Time Triggered Architecture
===================================


Introduction
------------

Pont in his book "Patterns for Time-Triggered Embedded Systems" book presents
a simple way to implement a real-time embedded system, without using multitasking.

It requires only one stack and uses many run-to-completion (RTC) functions.
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

    int
    Task_Update(void) {
    int i;
    TaskInfo *p;
    
        for(i=0;i<TASK_MAXCNT;i++) {
            p = &taskinfo[i];
            if( p->task ) {
                if( p->delay == 0 ) {
                    p->runcnt++;
                    if( p->period )
                        p->delay = p->period;
                } else {
                    p->delay--;
                }
            }
        }
    
        return 0;
    }
    

During the main cycle, the *Task_Dispatch* is run.

    int Task_Dispatch(void) {
    int i;
    TaskInfo *p;
    
        for(i=0;i<TASK_MAXCNT;i++) {
            p = &taskinfo[i];
            if( p->task ) {
                if( p->runcnt ) {
                    p->task();
                    p->runcnt--;
                    if( p->runcnt == 0 && p->period == 0 ) {
                        Task_Delete(i);
                    }
                }
            }
    
        }
    
        return 0;
    }

> NOTE: There is a  risk when the Task_Dispatch decrements p->runcnt!!


References
----------

1. [Patterns for Time-Triggered Embedded Systems](https://www.safetty.net/publications/pttes)