/* resched.c - resched, resched_cntl */

#include <xinu.h>

#define DEBUG_CTXSW

struct	defer	Defer;
syscall print_ready_list();

pid32 lottery_scheduling();

uint32 getUserProcesses();
/*------------------------------------------------------------------------
 *  resched  -  Reschedule processor to highest priority eligible process
 *------------------------------------------------------------------------
 */
void	resched(void)		/* Assumes interrupts are disabled	*/
{
	//kprintf("\n");
	struct procent *ptold;	/* Ptr to table entry for old process	*/
	struct procent *ptnew;	/* Ptr to table entry for new process	*/

	/* If rescheduling is deferred, record attempt and return */

	if (Defer.ndefers > 0) {
		Defer.attempt = TRUE;
		return;
	}

	/* Point to process table entry for the current (old) process */

	ptold = &proctab[currpid];
    pid32 oldpid=currpid;

	// if (ptold->prstate == PR_CURR) {  /* Process remains eligible */
	// 	if (ptold->prprio > firstkey(readylist)) {
	// 		return;
	// 	}

	// 	/* Old process will no longer remain current */

	// 	ptold->prstate = PR_READY;
	// 	insert(currpid, readylist, ptold->prprio);
	// }
    //kprintf("oldproc: %d...oldproc state: %d\n",currpid, proctab[currpid].prstate);
	//print_ready_list();
	//get number of user processes//
	///////////////////////////Lottery////////////////////////////////////////////
	if(ptold->isUserProcess==0)
	{
		if(ptold->prstate!=PR_CURR)
		{
			if(firstkey(readylist)>=1)
			{
				if(firstkey(readylist)>1)
				{
					currpid=dequeue(readylist);
				}
				else
				{
					uint32 user_num=getUserProcesses();
					if(user_num>1)
					{
					    currpid=lottery_scheduling();
					    getitem(currpid);
					}
					else
					{
						currpid=dequeue(readylist);
					}
				}
			}
			else
			{
				currpid=dequeue(readylist);
			}
		}
		else if(ptold->prstate==PR_CURR)
		{
			if(ptold->prprio>firstkey(readylist))
			{
				return;
			}
			ptold->prstate=PR_READY;
			insert(currpid,readylist,ptold->prprio);
            if(firstkey(readylist)>1)
			{
				currpid=dequeue(readylist);
			}
			else
			{
				uint32 user_num=getUserProcesses();
				if(user_num>1)
				{
				    currpid=lottery_scheduling();
				    getitem(currpid);
				}
				else
				{
					currpid=dequeue(readylist);
				}
				
			}
		}
	}
	else if(ptold->isUserProcess==1)
	{
		if(ptold->prstate!=PR_CURR)
		{
			uint32 prionew=firstkey(readylist);
			if(prionew>1)
			{
				currpid=dequeue(readylist);
			}
			else if(prionew==1)
			{
				uint32 user_num=getUserProcesses();
				if(user_num>1)
				{
				    currpid=lottery_scheduling();
				    getitem(currpid);
				}
				else
				{
					currpid=dequeue(readylist);
				}
				
			}
			else if(prionew==0)
			{
				currpid=dequeue(readylist);
			}
			
		}
		else if(ptold->prstate==PR_CURR)
		{
			ptold->prstate=PR_READY;
			insert(currpid,readylist,ptold->prprio);
			if(firstkey(readylist)>1)
			{
				currpid=dequeue(readylist);
			}
			else
			{
				uint32 user_num=getUserProcesses();
				if(user_num>1)
				{
				    currpid=lottery_scheduling();
				    getitem(currpid);
				}
				else
				{
					currpid=dequeue(readylist);
				}
				
				if(currpid==oldpid)
				{
					ptold->prstate=PR_CURR;
					return;
				}
			}
		}
	}
	/////////////////////////////////////////////////////////////////////////////
    //kprintf("curpid: %d...currpid state: %d\n",currpid,proctab[currpid].prstate);
	//currpid = dequeue(readylist);
	ptnew = &proctab[currpid];
	ptnew->num_ctxsw++;
	ptnew->prstate = PR_CURR;
	preempt = QUANTUM;		/* Reset time slice for process	*/
    //kprintf("curpid1: %d...currpid state1: %d\n",currpid,proctab[currpid].prstate);
	//print_ready_list();
	#ifdef DEBUG_CTXSW         
		kprintf("ctxsw::%d-%d\n",oldpid,currpid);
    #endif

	ctxsw(&ptold->prstkptr, &ptnew->prstkptr);
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
//get number of user processes//
uint32 getUserProcesses()
{
	uint32 numUserProcess=0;
	pid32 iterator=firstid(readylist);
	while(queuetab[iterator].qnext!=EMPTY)
	{
		if(proctab[iterator].isUserProcess==1)
		{
			numUserProcess++;
		}
		//kprintf("%d ",iterator);
		iterator=queuetab[iterator].qnext;
	}
	return numUserProcess;
}

syscall print_ready_list()
{
    pid32 iterator=firstid(readylist);
	while(queuetab[iterator].qnext!=EMPTY)
	{
		kprintf("%d ",iterator);
		iterator=queuetab[iterator].qnext;
	}
	kprintf("\n");

    return OK;
}

////////////////Lottery Scheduling///////////////////////////////////
pid32 lottery_scheduling()
{
	pid32 newpid;
	uint32 ticket_array[20]; /* create array to store user processes */
	pid32 pid_array[20]; /* create array to store user pid */
	uint32 sum_tickets=0;
	uint32 i;
	for(i=0;i<20;i++)
	{
		ticket_array[i]=0;
		pid_array[i]=0;
	}
    //populate pid and ticket arrays
	pid32 iterator=firstid(readylist);
    uint32 j=0;
	while(queuetab[iterator].qnext!=EMPTY)
	{
		if(proctab[iterator].isUserProcess==1)
		{
			//kprintf("pid: %d..%d",iterator,proctab[iterator].num_tickets);
			ticket_array[j]=proctab[iterator].num_tickets;
			pid_array[j]=iterator;
			sum_tickets+=proctab[iterator].num_tickets;
			j++;
		}
		//kprintf("\n");
		iterator=queuetab[iterator].qnext;
	}
	//kprintf("sum: %d\n",sum_tickets);
	uint32 m,k,a,b;
	for (k = 0; k < j; ++k) 
        {
            for (m = k + 1; m < j; ++m) 
            {
                if (pid_array[k] < pid_array[m]) 
                {
                    a = pid_array[k];
                    pid_array[k] = pid_array[m];
                    pid_array[m] = a;
					///////////////////
					b = ticket_array[k];
					ticket_array[k] = ticket_array[m];
					ticket_array[m]=b;
                }
            }
        }
    /////////////increasing order/////////////////////////////
	for (k = 0; k < j; ++k) 
        {
            for (m = k + 1; m < j; ++m) 
            {
                if (ticket_array[k] > ticket_array[m]) 
                {
					b = ticket_array[k];
					ticket_array[k] = ticket_array[m];
					ticket_array[m]=b;
					/////////////////////////
                    a = pid_array[k];
                    pid_array[k] = pid_array[m];
                    pid_array[m] = a;
                }
            }
        }
	//mergeSort(pid_array,ticket_array,0,99);
	//mergeSort(ticket_array,pid_array,0,99);
	uint32 counter=0;
	uint32 winner=rand()%sum_tickets;
	uint32 current;
	for(current=0;current<j;current++)
	{
		counter=counter+ticket_array[current];
		if(counter>winner)
		{
			break;
		}
	}
	newpid=pid_array[current];
	return newpid;
}
