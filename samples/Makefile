CC=mpicc
# CC=clang
CFLAGS= -Wall
LFLAGS= -lm

CFILES=queue.c lqueue.c manwork.c nqueens.c
HFILES=queue.h lqueue.h
OFILES=queue.o lqueue.o manwork.o nqueens.o

all:	nqueens

%.o:    %.c $(HFILES)                                                                                   
		$(CC) -c $(CFLAGS) $< -o $@ -pthread

nqueen:	$(OFILES) $(HFILES)
	$(CC) $(CFLAGS) $(OFILES) -o nqueens $(LFLAGS)

run:
	# mpirun -hostfile ./myhosts --bind-to core:overload-allowed -np ${np} nqueens ${size}
	mpirun -np ${np} nqueens ${size}

clean:
	rm -f *~ nqueens nqueens.o

realclean:
	rm -f *~ nqueens *.o


