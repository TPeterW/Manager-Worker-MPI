#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdint.h>
#include <mpi.h>
#include <pthread.h>

#include "lqueue.h"
#include "manwork.h"

#define mpisend(MSG, PID) {\
    MPI_Send(MSG, sizeof(buf), MPI_CHAR, PID, 26, MPI_COMM_WORLD);\
}

#define mpirecv(buf, source) {\
    MPI_Recv(buf, sizeof(buf), MPI_CHAR, source, MPI_ANY_TAG, MPI_COMM_WORLD, &status);\
}

struct request {
    uint8_t done;
    char payload[511];
};

struct response {
    uint8_t terminated;
    uint8_t hasResult;
    char result[511];
};

struct generatorPackage {
    void (*generator)(void *payload, lqueue_t *qjobs);
    void *payload;
    lqueue_t *qjobs;
    int *finished;
};

void manager(int np, void (*generator)(void *payload, lqueue_t *qjobs),
                void *payload, lqueue_t *qresult);

void worker(void (*process)(void *payload, void *result));

void *
generatorWrapper(void *argument)
{
    struct generatorPackage *genPac = (struct generatorPackage *)argument;
    genPac->generator(genPac->payload, genPac->qjobs);
    *genPac->finished = 1;

    return NULL;
}

void
mpigo(int argc, char** argv,
    void (*generator)(void *payload, lqueue_t *qjobs), void *payload,
    void (*process)(void *payload, void *result),
    void (*display)(lqueue_t *qresult, int np))
{
    int np;
    int pid;
    lqueue_t *qresult;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &np);
    MPI_Comm_rank(MPI_COMM_WORLD, &pid);

    if (pid == 0) {
        qresult = lqopen();
        manager(np, generator, payload, qresult);
        display(qresult, np);
    }
    else
        worker(process);

    MPI_Finalize();
}

void
manager(int np, void (*generator)(void *payload, lqueue_t *qjobs),
                void *payload, lqueue_t *qresult)
{
    MPI_Status status;

    lqueue_t *qjobs = lqopen();
    pthread_t *gen = calloc(1, sizeof(pthread_t));

    // generate jobs
    int finished = 0;    
    struct generatorPackage *genPac = calloc(1, sizeof(struct generatorPackage)); 
    genPac->generator = generator;
    genPac->payload = payload;
    genPac->qjobs = qjobs;
    genPac->finished = &finished;

    pthread_create(gen, NULL, &generatorWrapper, (void *)genPac);

    char buf[512];
    struct request *req;
    struct report *rep;

    int numProcessDone = 0;
    void *job;
    int checkGen = 1;
    do {
        job = lqget(qjobs);
        if (checkGen && job == NULL) {
            if (finished) {
                pthread_join(*gen, NULL);
                checkGen = 0;
            }
            continue;
        }

        req = calloc(1, sizeof(struct request));

        memset(buf, 0, sizeof(buf));
        mpirecv(buf, MPI_ANY_SOURCE);

        if (((struct response *)buf)->terminated)
            numProcessDone++;       // count how many have finished

        if (((struct response *)buf)->hasResult) {
            rep = calloc(1, sizeof(struct report));
            rep->pid = status.MPI_SOURCE;
            memcpy(rep->result, ((struct response *)buf)->result, sizeof(rep->result));
            lqput(qresult, rep);
        }

        if (job != NULL) {              // not done
            req->done = 0;
            memcpy(req->payload, job, sizeof(req->payload));
            mpisend(req, status.MPI_SOURCE);
        }
        else {                          // all done
            req->done = 1;
            mpisend(req, status.MPI_SOURCE);
        }
    } while (numProcessDone < np - 1);
}

void
worker(void (*process)(void *payload, void *result)) {
    char buf[512];
    char temp[512];
    MPI_Status status;

    struct response *resp = calloc(1, sizeof(struct response));
    void *result;

    int hasResult = 0;
    int shouldExit = 0;
    
    ((struct response *)buf)->hasResult = 0;    // first ever
    ((struct response *)buf)->terminated = 0;
    mpisend(buf, 0);
    do {
        mpirecv(buf, 0);
        if (((struct request *)buf)->done) {
            if (!hasResult) shouldExit = 1;
            else {
                mpisend(resp, 0);
                hasResult = 0;
            }
        }
        else {
            if (hasResult) {
                mpisend(resp, 0);
                hasResult = 0;
            }
            else {
                ((struct response *)temp)->hasResult = 0;
                ((struct response *)temp)->terminated = 0;
                mpisend(temp, 0);
            }

            result = calloc(1, sizeof(resp->result));
            memset(result, 0, sizeof(resp->result));
            process(((struct request *)buf)->payload, result);

            resp = calloc(1, sizeof(struct response));
            resp->hasResult = hasResult = 1;
            memcpy(resp->result, result, sizeof(resp->result));
        }
    } while (!shouldExit);

    ((struct response *)buf)->hasResult = 0;
    ((struct response *)buf)->terminated = 1;
    mpisend(buf, 0);
}