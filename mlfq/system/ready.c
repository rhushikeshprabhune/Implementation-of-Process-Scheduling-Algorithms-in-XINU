/* ready.c - ready */

#include <xinu.h>

qid16	readylist;			/* Index of ready list		*/
qid16 HQ;
qid16 MQ;
qid16 LQ;

/*------------------------------------------------------------------------
 *  ready  -  Make a process eligible for CPU service
 *------------------------------------------------------------------------
 */
status	ready(
	  pid32		pid		/* ID of process to make ready	*/
	)
{
	register struct procent *prptr;

	if (isbadpid(pid)) {
		return SYSERR;
	}

	/* Set process state to indicate ready and add to ready list */
    prptr = &proctab[pid];
	prptr->prstate = PR_READY;
    if(proctab[pid].isUserProcess==0)
	{
	    insert(pid, readylist, prptr->prprio);
	    resched();
	}
	else if(proctab[pid].isUserProcess==1)
	{
		if(prptr->queue_number==3)
		{
			//kprintf("proc is for HQ\n");
			insert_mlfq(pid, HQ);
	        resched();
		}
		else if(prptr->queue_number==2)
		{
			//kprintf("proc is for MQ\n");
			insert_mlfq(pid, MQ);
	        resched();
		}
		else if(prptr->queue_number==1)
		{
			//kprintf("proc is for LQ\n");
			insert_mlfq(pid, LQ);
	        resched();
		}
	}

	return OK;
}
