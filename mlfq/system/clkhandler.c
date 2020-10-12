/* clkhandler.c - clkhandler */

#include <xinu.h>

/*------------------------------------------------------------------------
 * clkhandler - high level clock interrupt handler
 *------------------------------------------------------------------------
 */
void	clkhandler()
{
	static	uint32	count1000 = 1000;	/* Count to 1000 ms	*/

	/* Decrement the ms counter, and see if a second has passed */
    ctr1000++;

	/* Implement boost mechanism in MLFQ */
	
	if(clk_boost > PRIORITY_BOOST_PERIOD)
	{
		clk_boost=0;
		//move all jobs in HQ and put alloted time to zero
		//kprintf("boost!");
		int i=0;
		for(i=0;i<NPROC;i++)
		{
			if(proctab[i].prstate!=PR_FREE)
			{
				proctab[i].alloted_time_usage=0;
				proctab[i].queue_number=3;
			}
		}

		uint32 iterator_MQ=firstid(MQ);
		uint32 numMQ=0;
		while(queuetab[iterator_MQ].qnext!=EMPTY)
		{
			numMQ++;
			proctab[iterator_MQ].alloted_time_usage=0;
			proctab[iterator_MQ].queue_number=3;
			iterator_MQ=queuetab[iterator_MQ].qnext;
		}

		uint32 iterator_LQ=firstid(LQ);
		uint32 numLQ=0;
		while(queuetab[iterator_LQ].qnext!=EMPTY)
		{
			numLQ++;
			proctab[iterator_LQ].alloted_time_usage=0;
			proctab[iterator_LQ].queue_number=3;
			iterator_LQ=queuetab[iterator_LQ].qnext;
		}
		for(i=0;i<numMQ;i++)
		{
			uint32 MQpid = dequeue(MQ);
			insert_mlfq(MQpid,HQ);
		}
		for(i=0;i<numLQ;i++)
		{
			uint32 LQpid = dequeue(LQ);
			insert_mlfq(LQpid,HQ);
		}
	}
	else
	{
	    clk_boost++;
	}
	proctab[currpid].alloted_time_usage++;
	
	/* increment runtime of current process */
	proctab[currpid].runtime++;  /* project_2 CT2 */
	if((--count1000) <= 0) {

		/* One second has passed, so increment seconds count */

		clktime++;

		/* Reset the local ms counter for the next second */

		count1000 = 1000;
	}

	/* Handle sleeping processes if any exist */

	if(!isempty(sleepq)) {

		/* Decrement the delay for the first process on the	*/
		/*   sleep queue, and awaken if the count reaches zero	*/

		if((--queuetab[firstid(sleepq)].qkey) <= 0) {
			wakeup();
		}
	}

	/* Decrement the preemption counter, and reschedule when the */
	/*   remaining time reaches zero			     */

	if((--preempt) <= 0) {
		if(proctab[currpid].isUserProcess==0)
		{
		    preempt = QUANTUM;
		}
		else if(proctab[currpid].isUserProcess==1)
		{
			if(proctab[currpid].queue_number==3)
			{
				preempt=QUANTUM;
			}
			else if(proctab[currpid].queue_number==2)
			{
				preempt=2*QUANTUM;
			}
			else if(proctab[currpid].queue_number==1)
			{
				preempt=4*QUANTUM;
			}
		}
		resched();
	}
}
