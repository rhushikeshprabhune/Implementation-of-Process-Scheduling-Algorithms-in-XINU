/* print_ready_list.c - print PID's of processes in ready list */

#include <xinu.h>

/* project_2 CT1 */
syscall print_ready_list()
{
    pid32 iterator=firstid(readylist);
	while(queuetab[iterator].qnext!=EMPTY)
	{
		kprintf("%d ",iterator);
		iterator=queuetab[iterator].qnext;
	}
	kprintf("\n");
	//kprintf("NumUserProc: %d\n",numUserProc);

    return OK;
}
