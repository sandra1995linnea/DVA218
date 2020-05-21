CC = gcc
CFLAGS = -Wall
PROGRAMS = client server

ALL: ${PROGRAMS}

client: client.c
	${CC} ${CFLAGS} -pthread -o client client.c linkedlist.h linkedlist.c common.h common.c

server: server.c
	${CC} ${CFLAGS} -o server server.c linkedlist.h linkedlist.c common.h common.c

clean:
	rm -f ${PROGRAMS}
