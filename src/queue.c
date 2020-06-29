#include <stdio.h>
#include <stdlib.h>
#include "queue.h"

int empty(struct queue_t *q)
{
	return (q->size == 0);
}

void enqueue(struct queue_t *q, struct pcb_t *proc)
{
	/* TODO: put a new process to queue [q] */
	if (q->size == MAX_QUEUE_SIZE) return;
	q->proc[q->size] = proc;
	q->size++;
}

struct pcb_t *dequeue(struct queue_t *q)
{
	/* TODO: return a pcb whose prioprity is the highest
	 * in the queue [q] and remember to remove it from q
	 * */
	if (q->size == 0) return NULL;
	int sizeOfQueue = q->size;
	struct pcb_t *proc_priority = q->proc[0];
	int ind_priority = 0;
	for (int counter = 1; counter < sizeOfQueue; counter ++)
	{
		if (q->proc[counter]->priority > proc_priority->priority)
		{
			proc_priority = q->proc[counter];
			ind_priority = counter;
		}
	}
	for (; ind_priority < sizeOfQueue - 1; ind_priority++)
	{
		q->proc[ind_priority] = q->proc[ind_priority + 1];
		
	}
	q->proc[ind_priority] = NULL;
	q->size--;
	return proc_priority;
}
