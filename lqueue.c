/*
 * lqueue.c by Tao Wang (Peter)
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include "lqueue.h"
#include "queue.h"

pthread_mutex_t lock;

lqueue_t *
lqopen(void) {
    struct lockedQ *lq = calloc(1, sizeof(struct lockedQ));
    pthread_mutex_init(&lq->lock, NULL);
    lq->queue = qopen();
    return lq;
}

void
lqclose(lqueue_t *qp) {
    struct lockedQ *lq = (struct lockedQ *)qp;
    pthread_mutex_lock(&lq->lock);
    qclose(lq->queue);
    pthread_mutex_unlock(&lq->lock);
}

int
lqput(lqueue_t *qp, void *elementp) {
    struct lockedQ *lq = (struct lockedQ *)qp;
	int ret;
    pthread_mutex_lock(&lq->lock);
    ret = qput(lq->queue, elementp);
    pthread_mutex_unlock(&lq->lock);
    return ret;
}

void *
lqget(lqueue_t *qp) {
    struct lockedQ *lq = (struct lockedQ *)qp;
	void *ret;
	pthread_mutex_lock(&lq->lock);
    ret = qget(lq->queue);
    pthread_mutex_unlock(&lq->lock);
	return ret;
}

void
lqapply(lqueue_t *qp, void (*fn)(void *elementp)) {
    struct lockedQ *lq = (struct lockedQ *)qp;
	pthread_mutex_lock(&lq->lock);
    qapply(lq->queue, fn);
    pthread_mutex_unlock(&lq->lock);
}

void *
lqsearch(lqueue_t *qp, int (*searchfn)(void* elementp, const void* keyp), const void* skeyp) {
    struct lockedQ *lq = (struct lockedQ *)qp;
    void *ret;
	pthread_mutex_lock(&lq->lock);
    ret = qsearch(lq->queue, searchfn, skeyp);
    pthread_mutex_unlock(&lq->lock);
	return ret;
}

void *
lqremove(lqueue_t *qp, int (*searchfn)(void* elementp,const void* keyp), const void* skeyp) {
	struct lockedQ *lq = (struct lockedQ *)qp;
    void *ret;
	pthread_mutex_lock(&lq->lock);
    ret = qremove(lq->queue, searchfn, skeyp);
    pthread_mutex_unlock(&lq->lock);
	return ret;
}

/* concatenatenates elements of q2 into q1, q2 is dealocated upon completion */
void
lqconcat(lqueue_t *q1p, queue_t *q2p) {
    struct lockedQ *lq1 = (struct lockedQ *)q1p;
    struct lockedQ *lq2 = (struct lockedQ *)q2p;
	pthread_mutex_lock(&lq1->lock);
    pthread_mutex_lock(&lq1->lock);
    qconcat(lq1->queue, lq2->queue);
    pthread_mutex_unlock(&lq1->lock);
    pthread_mutex_unlock(&lq1->lock);
}
