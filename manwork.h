struct report {
    int pid;
    char result[511];
};

void mpigo(int argc, char** argv,
    void (*generator)(void *payload, lqueue_t *qjobs), void *payload,
    void (*process)(void *payload, void *result),
    void (*display)(lqueue_t *qresult, int np));

void generator(void *payload, lqueue_t *qjobs);

void process(void *payload, void *result);

void display(lqueue_t *qresult, int np);


// #define index(x, y) (x) * size + (y)
// struct processPackage {
//     uint8_t board[100];     // for now supports maximum 10 x 10
//     int size;
//     int depth;
// };
// void
// printB(uint8_t *board, int size)
// {
//     int i, j;
//     for (i = 0; i < size; i++) {
//         for (j = 0; j < size; j++) {
//             printf("%c", board[index(i, j)] ? 'Q' : '-');
//         }
//         printf("\n");
//     }
//     printf("\n");
// }