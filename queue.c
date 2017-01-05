/*
 * queue.c by Tao Wang (Peter)
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "queue.h"

struct node {
	struct node *next;
	void *content;
};

struct head {
	struct node *root;
};

void nclose(struct node *qp);

int nput(struct node *qp, void *elementp);

void napply(struct node *qp, void (*fn)(void *elementp));

/* create an empty queue */
queue_t *
qopen(void) {
	struct head *ret = calloc(1, sizeof(struct node *));
	ret->root = calloc(2, sizeof(void *));
	return (queue_t *)ret;
}

/* deallocate a queue, frees everything in it, a wrapper */
void
qclose(queue_t *qp) {
	nclose(((struct head *)qp)->root);
	free(qp);
}

/* encapsulated close function */
void
nclose(struct node *qp) {
	if (qp == NULL)
		return;
	if (qp->next != NULL)
		nclose(qp->next);
	free(qp);
}

/* put element at end of queue */
int
qput(queue_t *qp, void *elementp) {
	return nput(((struct head *)qp)->root, elementp);
}

/* encapsulated put function */
int
nput(struct node *qp, void *elementp) {
	struct node *temp = qp;
	
	if (elementp != NULL && temp->content == elementp) {
		return -1;
	}
	if (temp->next == NULL) {
		temp->next = calloc(2, sizeof(void *));
		temp->content = elementp;
		
		return 0;	/* success */
	}
	else {
		return nput(temp->next, elementp);
	}
}

/* get first element from queue, and remove it */
void *
qget(queue_t *qp) {
	struct node *temp;
	void *ret;
	
	if (((struct head *)qp)->root == NULL)
		return NULL;
	
	temp = ((struct head *)qp)->root;
	ret = temp->content;
	
	((struct head *)qp)->root = temp->next;
	if (((struct head *)qp)->root == NULL)
		((struct head *)qp)->root = calloc(2, sizeof(void *));
	free(temp);
	
	return ret;
}

/* apply a void function to every element of a queue */
void
qapply(queue_t *qp, void (*fn)(void *elementp)) {
	napply(((struct head *)qp)->root, fn);
}

/* encapsulated apply function */
void
napply(struct node *qp, void (*fn)(void *elementp)) {
	if (qp->next != NULL)
		napply(qp->next, fn);
	if (qp->content != NULL)	
		fn(qp->content);
}

/* search a queue using a supplied boolean function, returns an element */
void *
qsearch(queue_t *qp, int (*searchfn)(void* elementp, const void* keyp), const void* skeyp) {
	struct node *temp = ((struct head *)qp)->root;
	if (temp == NULL || temp->next == NULL)
		return NULL;

	while (!searchfn(temp->content, skeyp)) {
		temp = temp->next;
		if (temp == NULL || temp->next == NULL)
			return NULL;
	}
	return temp->content;
}

/* search a queue using a supplied boolean function, removes and
 * returns the element */
void *
qremove(queue_t *qp, int (*searchfn)(void* elementp,const void* keyp), const void* skeyp) {
	struct node *temp = ((struct head *)qp)->root;
	void *pre = NULL;
	void *ret;
	
	if (temp == NULL || temp->content == NULL)
		return NULL;

	while (!searchfn(temp->content, skeyp)) {
		if (temp->next == NULL)
			return NULL;
		else {
			pre = temp;
			temp = temp->next;
			if (temp->content == NULL)
				return NULL;
		}
	}
	
	ret = temp->content;
	if (pre == NULL)
		((struct head *)qp)->root = temp->next;
	else
		((struct node *)pre)->next = temp->next;
	return ret;
}

/* concatenatenates elements of q2 into q1, q2 is dealocated upon completion */
void
qconcat(queue_t *q1p, queue_t *q2p) {
	struct node *temp = ((struct head *)q1p)->root;
	
	if (((struct head *)q2p)->root->content == NULL) {
		return;
	}
	if (((struct head *)q1p)->root->content == NULL) {
		((struct head *)q1p)->root = ((struct head *)q2p)->root;
		return;
	}

	while (temp->next->next != NULL)
		temp = temp->next;
	temp->next = ((struct head *)q2p)->root;
}
