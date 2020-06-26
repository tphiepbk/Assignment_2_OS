#include <stdio.h>
#include <stdlib.h>
#include "queue.h"

int empty(struct queue_t * q) {
	return (q->size == 0);
}

void enqueue(struct queue_t * q, struct pcb_t * proc) {
	if(empty(q)){
		q -> proc[0] = (struct pcb_t *)malloc(sizeof(struct pcb_t));
		q -> proc[0] = proc ;
		q -> size +=1 ;
	}
	else if(q -> size < 10){
		q -> proc[q->size] = (struct pcb_t *)malloc(sizeof(struct pcb_t));
		q -> proc[q->size] = proc ;
		q -> size +=1 ;
		
	}
		
	/* TODO: put a new process to queue [q] */	
}

struct pcb_t * dequeue(struct queue_t * q) {
	/* TODO: return a pcb whose prioprity is the highest
	 * in the queue [q] and remember to remove it from q
	 * */
	struct pcb_t * proc_t ;
	int kt = 0; 
	if(empty(q)){
		return NULL;
	}
	else {
		
		proc_t = q -> proc[0];
		for(int i = 1 ; i < q -> size ; i++){
			if(q -> proc[i] -> priority > proc_t -> priority){
				proc_t = q -> proc[i];
				kt = i;
			}
		}
		q -> size -= 1 ;
		q -> proc[kt] = q -> proc[q->size];
		return proc_t;
	}
}

