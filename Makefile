#include lib/Makefile

CC = gcc
CFLAGS = -l pthread

OBJS = tftp.o
PROGS = client server

all: ${OBJS} ${PROGS}

server:
	${CC} server.c ${OBJS} ${CFLAGS} -o server

client:
	${CC} client.c ${CFLAGS} -o client

tftp.o:
	${CC} -c tftp.c

clean:
	rm -f ${PROGS} ${OBJS}
