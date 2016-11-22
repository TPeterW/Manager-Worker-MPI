#pragma once
/* 
 * lqueue.h -- public interface to the queue module
 */

#include <pthread.h>
#include "queue.h"

struct lockedQ {
    pthread_mutex_t lock;
    queue_t *queue;
};

/* the queue representation is hidden from users of the module */
typedef void lqueue_t;		

/* create an empty queue */
lqueue_t* lqopen(void);        

/* deallocate a queue, frees everything in it */
void lqclose(lqueue_t *qp);   

/* put element at end of queue */
int lqput(lqueue_t *qp, void *elementp); 

/* get first element from queue */
void* lqget(lqueue_t *qp);

/* apply a void function to every element of a queue */
void lqapply(lqueue_t *qp, void (*fn)(void* elementp));

/* search a queue using a supplied boolean function, returns an element */
void* lqsearch(lqueue_t *qp, 
	      int (*searchfn)(void* elementp,const void* keyp),
	      const void* skeyp);

/* search a queue using a supplied boolean function, removes and
 * returns the element 
 */
void* lqremove(lqueue_t *qp,
	      int (*searchfn)(void* elementp,const void* keyp),
	      const void* skeyp);

/* concatenatenates elements of q2 into q1, q2 is dealocated upon completion */
void lqconcat(lqueue_t *q1p, lqueue_t *q2p);

