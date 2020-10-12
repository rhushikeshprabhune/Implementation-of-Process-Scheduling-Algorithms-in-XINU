/* resched.c - resched, resched_cntl */

#include <xinu.h>

//#define DEBUG_CTXSW

struct	defer	Defer;
uint32 queue_count(qid16 q);
/*------------------------------------------------------------------------
 *  resched  -  Reschedule processor to highest priority eligible process
 *------------------------------------------------------------------------
 */
pid32 schedule_MLFQ();

void print_queues();

void	resched(void)		/* Assumes interrupts are disabled	*/
{
	struct procent *ptold;	/* Ptr to table entry for old process	*/
	struct procent *ptnew;	/* Ptr to table entry for new process	*/

	/* If rescheduling is deferred, record attempt and return */
    //kprintf("enter resched\n");
	if (Defer.ndefers > 0) {
		Defer.attempt = TRUE;
		return;
	}

	/* Point to process table entry for the current (old) process */

	ptold = &proctab[currpid];
	pid32 oldpid=currpid;  /* project_2 CT4 */
    
	///////////////////move things in 3 queues//////////////////////////////////
    //kprintf("currpid: %d(%d)(%d)..qn: %d\t",oldpid,ptold->prstate,ptold->alloted_time_usage,ptold->queue_number);
   // print_queues();
	// if (ptold->prstate == PR_CURR) {  /* Process remains eligible */
	// 	if (ptold->prprio > firstkey(readylist)) {
	// 		return;
	// 	}

	// 	/* Old process will no longer remain current */

	// 	ptold->prstate = PR_READY;

	// 	insert(currpid, readylist, ptold->prprio);
	// }
	// uint32 isCurrent=0;
	// if(ptold->prstate==PR_CURR)
	// {
	// 	isCurrent=1;
	// }
	////////////MLFQ SCHEDULING//////////////////////////////
   	if(ptold->isUserProcess==0)
	{
		if(ptold->prstate!=PR_CURR)
		{
			uint32 sys_count=queue_count(readylist);
			uint32 user_count=queue_count(HQ)+queue_count(MQ)+queue_count(LQ);
			if((sys_count+user_count)>1)  //check
			{
				if(sys_count>1) //////////////
				{
					currpid=dequeue(readylist);
				}
				else if(user_count>0) ///////////////////////////////////////////////////
				{
					currpid=schedule_MLFQ();
				}
				// else if(sys_count==1) /////////////////////////////////////////////
				// {
				// 	currpid=dequeue(readylist);
				// }
			}
			else
			{
				currpid=dequeue(readylist);
			}
		}
		else if(ptold->prstate==PR_CURR)
		{
			uint32 user_count=queue_count(HQ)+queue_count(MQ)+queue_count(LQ);
			uint32 sys_count=queue_count(readylist);
			if(ptold->prprio==0)
			{
				if(sys_count>0)
				{
					currpid=dequeue(readylist);
					ptold->prstate=PR_READY;
			        insert(oldpid,readylist,ptold->prprio);
				}
				else if(user_count>0)
				{
					currpid=schedule_MLFQ();
					ptold->prstate=PR_READY;
			        insert(oldpid,readylist,ptold->prprio);
				}
				else
				{
					return;
				}
			}
			else
			{
				if(ptold->prprio>firstkey(readylist))
			    {
					return;
				}
				ptold->prstate=PR_READY;
				insert(currpid,readylist,ptold->prprio);
				currpid=dequeue(readylist);
			}
		}
	}
	else if(ptold->isUserProcess==1)
	{
		if(ptold->prstate!=PR_CURR)
		{
			uint32 user_count=queue_count(HQ)+queue_count(MQ)+queue_count(LQ);
			uint32 sys_count=queue_count(readylist);
			if(sys_count>1)
			{
				currpid=dequeue(readylist);
			}
			else if(user_count>0)
			{
				currpid=schedule_MLFQ();
			}
			else
			{
				currpid=dequeue(readylist);
			}			
		}
		else if(ptold->prstate==PR_CURR)
		{
			uint32 user_count=queue_count(HQ)+queue_count(MQ)+queue_count(LQ);
			uint32 sys_count=queue_count(readylist);
			ptold->prstate=PR_READY;
			//////////////////////
			if(proctab[oldpid].alloted_time_usage>TIME_ALLOTMENT)
			{
				//kprintf("allot time over for %d\n",oldpid);
				if(proctab[oldpid].queue_number==3)
				{
					proctab[oldpid].queue_number=2;
					proctab[oldpid].alloted_time_usage=0;
					insert_mlfq(oldpid,MQ);
				}
				else if(proctab[oldpid].queue_number==2)
				{
					proctab[oldpid].queue_number=1;
					proctab[oldpid].alloted_time_usage=0;
					insert_mlfq(oldpid,LQ);
				}
				else if(proctab[oldpid].queue_number==1)
				{
					proctab[oldpid].alloted_time_usage=0;
					insert_mlfq(oldpid,LQ);
				}
			}
			else
			{
				if(ptold->queue_number==3)
				{
					insert_mlfq(currpid,HQ);
				}
				else if(ptold->queue_number==2)
				{
					insert_mlfq(currpid,MQ);
				}
				else if(ptold->queue_number==1)
				{
					insert_mlfq(currpid,LQ);
				}
			}
			
			if(sys_count>1)
			{
				currpid=dequeue(readylist);
			}
			else
			{
				pid32 newpid=schedule_MLFQ();
				if(newpid!=currpid)   //check for redundant ctxsw
				{
				  currpid=newpid;
				}
				else
				{
					ptold->prstate=PR_CURR;
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
					return;
				}
			}
		}
	}
	///////////////////////////////////////////
	/* Force context switch to highest priority ready process */

    /* save old pid before currpid is altered */

	//currpid = dequeue(readylist);
	////////////////////////////////////////give quantum its required values///////////////////////
	ptnew = &proctab[currpid];
	ptnew->prstate = PR_CURR;
	if(proctab[currpid].isUserProcess==0)   
	{
	    preempt=QUANTUM;		/* Reset time slice for process	*/
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
	#ifdef DEBUG_CTXSW         
        //if(oldpid!=currpid)          /* project_2 CT4 */
		//{
			kprintf("ctxsw::%d-%d\n",oldpid,currpid);
		//}
    #endif
	ptnew->num_ctxsw++;   /* project_2 CT2 */
	ctxsw(&ptold->prstkptr, &ptnew->prstkptr);
    
	
	/* update num_ctxsw variable */
	/* Old process returns here when resumed */

	return;
}

/*------------------------------------------------------------------------
 *  resched_cntl  -  Control whether rescheduling is deferred or allowed
 *------------------------------------------------------------------------
 */
status	resched_cntl(		/* Assumes interrupts are disabled	*/
	  int32	defer		/* Either DEFER_START or DEFER_STOP	*/
	)
{
	switch (defer) {

	    case DEFER_START:	/* Handle a deferral request */

		if (Defer.ndefers++ == 0) {
			Defer.attempt = FALSE;
		}
		return OK;

	    case DEFER_STOP:	/* Handle end of deferral */
		if (Defer.ndefers <= 0) {
			return SYSERR;
		}
		if ( (--Defer.ndefers == 0) && Defer.attempt ) {
			resched();
		}
		return OK;

	    default:
		return SYSERR;
	}
}

//count queue
uint32 queue_count(qid16 q)
{
	uint32 checkEntries=0;
	uint32 iterator=firstid(q);
	while(queuetab[iterator].qnext!=EMPTY)
	{
        checkEntries++;
		iterator=queuetab[iterator].qnext;
	}
	return checkEntries;

}
/////////////////////////////MLFO Scheduling//////////////////////////////////
pid32 schedule_MLFQ()
{
	uint32 newpid=0; //check
	uint32 checkEntriesHQ=0;
	uint32 checkEntriesMQ=0;
	uint32 checkEntriesLQ=0;
	uint32 iteratorHQ=firstid(HQ);
	uint32 iteratorMQ=firstid(MQ);
	uint32 iteratorLQ=firstid(LQ);
	while(queuetab[iteratorHQ].qnext!=EMPTY)
	{
        checkEntriesHQ++;
		iteratorHQ=queuetab[iteratorHQ].qnext;
	}

	while(queuetab[iteratorMQ].qnext!=EMPTY)
	{
        checkEntriesMQ++;
		iteratorMQ=queuetab[iteratorMQ].qnext;
	}

	while(queuetab[iteratorLQ].qnext!=EMPTY)
	{
        checkEntriesLQ++;
		iteratorLQ=queuetab[iteratorLQ].qnext;
	}
    //check if HQ is nonempty.if nonempty dequeue head and schedule
	if(checkEntriesHQ>0)
	{
		newpid=dequeue(HQ); //check
	}
	else if(checkEntriesMQ>0)
	{
		newpid=dequeue(MQ);
	}
	else if(checkEntriesLQ>0)
	{
		newpid=dequeue(LQ);
	}
	return newpid;
}


void print_queues()
{
	//print queues
	uint32 itHQ=firstid(HQ);
	kprintf("HQ: ");
	while(queuetab[itHQ].qnext!=EMPTY)
	{
		kprintf("%d(%d) ",itHQ,proctab[itHQ].prstate);
		itHQ=queuetab[itHQ].qnext;
	}
	kprintf("\t");

	uint32 itMQ=firstid(MQ);
	kprintf("MQ: ");
	while(queuetab[itMQ].qnext!=EMPTY)
	{

		kprintf("%d(%d) ",itMQ,proctab[itMQ].prstate);
		itMQ=queuetab[itMQ].qnext;
	}
	kprintf("\t");

	uint32 itLQ=firstid(LQ);
	kprintf("LQ: ");
	while(queuetab[itLQ].qnext!=EMPTY)
	{
		kprintf("%d(%d) ",itLQ,proctab[itLQ].prstate);
		itLQ=queuetab[itLQ].qnext;
	}
	kprintf("\t");

	uint32 itrdy=firstid(readylist);
	kprintf("RL: ");
	while(queuetab[itrdy].qnext!=EMPTY)
	{
		kprintf("%d(%d) ",itrdy,proctab[itrdy].prstate);
		itrdy=queuetab[itrdy].qnext;
	}
	kprintf("\n");
}
