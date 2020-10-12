/* burst_execution.c - simulate execution of applications that alternate execution phases 
   requiring the CPU (CPU bursts), and execution phases not requiring it (CPU inactivity phases) */

#include <xinu.h>

void burst_execution(uint32 number_bursts, uint32 burst_duration,uint32 sleep_duration)
{
    uint32 burstnum;
    for(burstnum=0;burstnum<number_bursts;burstnum++)
    {
        uint32 init_time=proctab[currpid].runtime;
        while(proctab[currpid].runtime-init_time < burst_duration)
        {
            /* do nothing */
        }
        sleepms(sleep_duration); /* wait for process to run for (burst_duration) millisec and then go to sleep */
    }
}