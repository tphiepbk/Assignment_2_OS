#include <stdio.h>
#include <stdlib.h>
#include "queue.h"

int empty(struct queue_t * q) {
	return (q->size == 0);
}

void enqueue(struct queue_t * q, struct pcb_t * proc) {
	/* TODO: put a new process to queue [q] */	
	int count=0;
	count=q->size;

	if(count == MAX_QUEUE_SIZE){
		return;
	}

	q->proc[q->size++]=proc;
	//q->size++;
}

struct pcb_t * dequeue(struct queue_t * q) {
	/* TODO: return a pcb whose prioprity is the highest
	 * in the queue [q] and remember to remove it from q
	 * */
	if (q->size==0){
		return NULL;
	}

	int max_prior_index=0;

	for (int i=1;i< q->size;i++){
		if(q->proc[i]->priority > q->proc[max_prior_index]->priority){
			max_prior_index=i;
		}
	}

	struct pcb_t * temp = q->proc[max_prior_index];

	for (int i=max_prior_index+1;i<q->size;i++){
		q->proc[i-1]=q->proc[i];
	}

	q->size--;
	return temp;
}

