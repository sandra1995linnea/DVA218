/*
 * server.c
 *
 *  Created on: May 10, 2020
 *      Author: student
 */
#include "server.h"


#define PORT 5555
#define MAXMSG 512

/* makeSocket
 * Creates and names a socket in the Internet
 * name-space. The socket created exists
 * on the machine from which the function is
 * called. Instead of finding and using the
 * machine's Internet address, the function
 * specifies INADDR_ANY as the host address;
 * the system replaces that with the machine's
 * actual address.
 */
int makeSocket(unsigned short int port) {
	int sock;
	struct sockaddr_in name;

	/* Create a socket. */
	sock = socket(PF_INET, SOCK_STREAM, 0);
	if(sock < 0) {
		perror("Could not create a socket\n");
		exit(EXIT_FAILURE);
	}
	/* Give the socket a name. */
	/* Socket address format set to AF_INET for Internet use. */
	name.sin_family = AF_INET;
	/* Set port number. The function htons converts from host byte order to network byte order.*/
	name.sin_port = htons(port);
	/* Set the Internet address of the host the function is called from. */
	/* The function htonl converts INADDR_ANY from host byte order to network byte order. */
	/* (htonl does the same thing as htons but the former converts a long integer whereas
	 * htons converts a short.)
	 */
	name.sin_addr.s_addr = htonl(INADDR_ANY);
	/* Assign an address to the socket by calling bind. */
	if(bind(sock, (struct sockaddr *)&name, sizeof(name)) < 0) {
		perror("Could not bind a name to the socket\n");
		exit(EXIT_FAILURE);
	}
	return(sock);
}

// sending a message to the client, that we got the message
void respondToClient(int fileDescriptor, char *message)
{
	int nrOfBytes;
	nrOfBytes = write(fileDescriptor, message, strlen(message) + 1);
		if(nrOfBytes < 0) {
			perror("writeMessage - Could not write data\n");
			exit(EXIT_FAILURE);
		}
}


/* readMessageFromClient
 * Reads and prints data read from the file (socket
 * denoted by the file descriptor 'fileDescriptor'.
 */
int readMessageFromClient(int fileDescriptor) {
	char buffer[MAXMSG];
	int nOfBytes;

	nOfBytes = read(fileDescriptor, buffer, MAXMSG);
	if(nOfBytes < 0) {
		perror("Could not read data from client\n");
		exit(EXIT_FAILURE);
	}
	else
		if(nOfBytes == 0)
			/* End of file */
			return(-1);
		else {
			/* Data read */
			printf(">Incoming message: %s\n", buffer);

			respondToClient(fileDescriptor, "I hear you duede");
		}
	return(0);
}


int main(int argc, char *argv[]) {
	int sock;
	int clientSocket;
	int i;
	fd_set activeFdSet, readFdSet; /* Used by select */
	struct sockaddr_in clientName;
	socklen_t size;

	/* Create a socket and set it up to accept connections */
	sock = makeSocket(PORT);

	/* Listen for connection requests from clients */
	//seting the socket to be a listening socket
	if(listen(sock,1) < 0) {
		perror("Could not listen for connections\n");
		exit(EXIT_FAILURE);
	}

	/* Initialize the set of active sockets */
	//saving all the active sockets in a struct
	FD_ZERO(&activeFdSet);
	FD_SET(sock, &activeFdSet);

	printf("\n[waiting for connections...]\n");

	while(1) {
		/* Block until input arrives on one or more active sockets
       FD_SETSIZE is a constant with value = 1024 */
		readFdSet = activeFdSet;
		if(select(FD_SETSIZE, &readFdSet, NULL, NULL, NULL) < 0) {
			perror("Select failed\n");
			exit(EXIT_FAILURE);
		}


		/* Service all the sockets with input pending */
		for(i = 0; i < FD_SETSIZE; ++i)
		{
			if(FD_ISSET(i, &readFdSet))
			{
				if(i == sock) {
					/* Connection request on original socket */
					size = sizeof(struct sockaddr_in);

					/* Accept the connection request from a client. */
					clientSocket = accept(sock, (struct sockaddr *)&clientName, (socklen_t *)&size);
					if(clientSocket < 0) {
						perror("Could not accept connection\n");
						exit(EXIT_FAILURE);
					}

					printf("Sock = %d. \nServer: Connect from client %s, port %d, on socket %d\n", sock,
							inet_ntoa(clientName.sin_addr),
							ntohs(clientName.sin_port), clientSocket);

					FD_SET(clientSocket, &activeFdSet);

					printf("Socket to server is now open :)\n\n");

				}
				else {
					/* Data arriving on an already connected socket */
					if(readMessageFromClient(i) < 0)
					{
						close(i);
						FD_CLR(i, &activeFdSet);
					}
				}

			  }
		}
	}
}
