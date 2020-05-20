CC = gcc
CFLAGS = -Wall
PROGRAMS = client server

ALL: ${PROGRAMS}

client: client.c
	${CC} ${CFLAGS} -pthread -o client client.c linkedlist.h linkedlist.c

server: server.c
	${CC} ${CFLAGS} -o server server.c

clean:
	rm -f ${PROGRAMS}
