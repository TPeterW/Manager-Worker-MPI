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
