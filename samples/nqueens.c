#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdint.h>
#include <mpi.h>

#include "lqueue.h"
#include "manwork.h"

struct genPayload {
    int size;
    int depth;
};

struct processPackage {
    uint8_t board[400];     // for now supports maximum 20 x 20
    int size;
    int depth;
};

struct solution {
    int count;
    int placements;
};

#define index(x, y) (x) * size + (y)

void initBoard(uint8_t *board, int i, int j, int size, int depth, lqueue_t *qjobs);
void solveBoard(uint8_t *board, int i, int j, int size, void *result);
int legalMove(uint8_t *board, int i, int j, int size);
void printBoard(uint8_t *board, int size);

int
main(int argc, char **argv)
{
    int size = atoi(argv[1]);

    struct genPayload *pl = calloc(1, sizeof(struct genPayload));
    pl->size = size;
    pl->depth = 3;

    mpigo(argc, argv, &generator, pl, &process, &display);

    return 0;
}

void
generator(void *payload, lqueue_t *qjobs)
{
    int size = ((struct genPayload *)payload)->size;
    int depth = ((struct genPayload *)payload)->depth;
    uint8_t *board = calloc(size * size, sizeof(uint8_t));

    initBoard(board, 0, 0, size, depth, qjobs);
}

void
process(void *payload, void *result)
{
    struct processPackage *pac = (struct processPackage *)payload;
    uint8_t *board = pac->board;
    int size = pac->size;
    int depth = pac->depth;

    solveBoard(board, depth, 0, size, result);
}

void
display(lqueue_t *qresult, int np)
{
    int workload[np];
    int count = 0;

    struct report *rep;
    do {
        rep = lqget(qresult);
        if (rep != NULL) {
            workload[rep->pid] += ((struct solution *)rep)->count;
            count += ((struct solution *)rep)->placements;
        }
    } while (rep != NULL);

    int i;
    for (i = 1; i < np; i++) {
        printf("WK%d: %d\n", i, workload[i]);
    }
    printf("Total solutions: %d\n", count);
}

void
printBoard(uint8_t *board, int size)
{
    int i, j;
    for (i = 0; i < size; i++) {
        for (j = 0; j < size; j++) {
            printf("%c", board[index(i, j)] ? 'Q' : '-');
        }
        printf("\n");
    }
    printf("\n");
}

void
solveBoard(uint8_t *board, int i, int j, int size, void *result)
{
    if (i < 0 || j < 0 || i >= size || j >= size)
        return;
    
    if (legalMove(board, i, j, size)) {
        // *(int *)result++;                   // placements
        if (i == size - 1) {
            board[index(i, j)] = 1;
            // printBoard(board, size);
            *(int *)result++;               // solutions
            board[index(i, j)] = 0;
            return;
        }

        board[index(i, j)] = 1;
        solveBoard(board, i + 1, 0, size, sl);
        board[index(i, j)] = 0;
    }
    
    solveBoard(board, i, j + 1, size, sl);
}

void
initBoard(uint8_t *board, int i, int j, int size, int depth, lqueue_t *qjobs)
{
    if (i < 0 || j < 0 || i >= size || j >= size || i >= depth)
        return;
    
    if (legalMove(board, i, j, size)) {
        if (i == depth - 1) {
            board[index(i, j)] = 1;
            // printBoard(board, size);

            // add to queue
            struct processPackage *pac = calloc(1, sizeof(struct processPackage));
            pac->size = size;
            pac->depth = depth;                     // hardcode
            memcpy(pac->board, board, size * size * sizeof(uint8_t));
            lqput(qjobs, pac);

            board[index(i, j)] = 0;
        }

        board[index(i, j)] = 1;
        initBoard(board, i + 1, 0, size, depth, qjobs);
        board[index(i, j)] = 0;
    }
    
    initBoard(board, i, j + 1, size, depth, qjobs);
}

int
legalMove(uint8_t *board, int i, int j, int size)
{
    int k;
    int colSum = 0;
    int leftDiagSum = 0;
    int rightDiagSum = 0;

    for (k = 0; k < size; k++)
        colSum += board[index(k, j)];

    // left diagonal
    k = 1;
    while (i - k >= 0 && j - k >= 0) {
        leftDiagSum += board[index(i - k, j - k)];
        k++;
    }
    k = 1;
    while (i + k < size && j + k < size) {
        leftDiagSum += board[index(i + k, j + k)];
        k++;
    }

    // right diagonal
    k = 1;
    while (i + k < size && j - k >= 0) {
        rightDiagSum += board[index(i + k, j - k)];
        k++;
    }
    k = 1;
    while (i - k >= 0 && j + k < size) {
        rightDiagSum += board[index(i - k, j + k)];
        k++;
    }

    return leftDiagSum == 0 && rightDiagSum == 0 && colSum == 0;
}
