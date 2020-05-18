/*
 * client.c
 *
 *  Created on: May 10, 2020
 *      Author: student
 */
#include "linkedlist.h"


#define PORT 5555
#define hostNameLength 50
#define messageLength  256
#define MAXMSG 512


struct sockaddr_in serverName;
socklen_t size;

/* initSocketAddress
 * Initialises a sockaddr_in struct given a host name and a port.
 */
void initSocketAddress(struct sockaddr_in *name, char *hostName, unsigned short int port) {
	struct hostent *hostInfo; /* Contains info about the host */
	/* Socket address format set to AF_INET for Internet use. */
	name->sin_family = AF_INET;
	/* Set port number. The function htons converts from host byte order to network byte order.*/
	name->sin_port = htons(port);
	/* Get info about host. */
	hostInfo = gethostbyname(hostName);
	if(hostInfo == NULL) {
		fprintf(stderr, "initSocketAddress - Unknown host %s\n",hostName);
		exit(EXIT_FAILURE);
	}
	/* Fill in the host name into the sockaddr_in struct. */
	name->sin_addr = *(struct in_addr *)hostInfo->h_addr;
}

/* writeMessage
 * Writes the string message to the file (socket)
 * denoted by fileDescriptor.
 */
void writeMessage(int fileDescriptor, char *message, size) {
	int nOfBytes;

	nOfBytes = sendto(fileDescriptor, message, strlen(message) + 1, 0, (struct sockaddr *)&serverName, size);
	if(nOfBytes < 0) {
		perror("writeMessage - Could not write data\n");
		exit(EXIT_FAILURE);
	}
}
// recieving the respond message from the server and printing it out on the screen
void receiveMessage(int sock, size)
{
	char buffer[MAXMSG];

	int nOfBytes = recvfrom(sock, buffer, MAXMSG, 0, (struct sockaddr*) &serverName, &size);
	if(nOfBytes < 0) {
		perror("Could not read data from client\n");
		exit(EXIT_FAILURE);
	}
	else if (nOfBytes > 0)
	{
		printf(">Incoming message: %s\n", buffer);
	}
}

/*all the clients are listening after a message
 * that will be sent to every client
 */
void* ListenToMessages(void *pointer)
{
	int *socket = (int*) pointer;

	while(1)
	{
		receiveMessage(*socket, size);
	}
}

int main(int argc, char *argv[]) {
	int sock;
	struct sockaddr_in serverName;
	char hostName[hostNameLength];
	char messageString[messageLength];
	pthread_t thread1;
	int func1;
	socklen_t size;

	/* Check arguments */
	if(argv[1] == NULL) {
		perror("Usage: client [host name]\n");
		exit(EXIT_FAILURE);
	}
	else {
		strncpy(hostName, argv[1], hostNameLength);
		hostName[hostNameLength - 1] = '\0';
	}

	/* Create the socket */
	sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if(sock < 0) {
		perror("Could not create a socket\n");
		exit(EXIT_FAILURE);
	}

	/* Initialize the socket address */
	initSocketAddress(&serverName, hostName, PORT);

	/* Connect to the server */
	if(connect(sock, (struct sockaddr *)&serverName, sizeof(serverName)) < 0) {
		perror("Could not connect to server\n");
		exit(EXIT_FAILURE);
	}

	else
	{	//creating a thread that will handle the messages to every client
		func1 = pthread_create(&thread1, NULL, ListenToMessages, &sock);
		if (func1 != 0)
		{
			perror("Pthread is not working...");
			exit(EXIT_SUCCESS);
		}

	}


	printf("\nType something and press [RETURN] to send it to the server.\n");
	printf("Type 'quit' to nuke this program.\n");
	fflush(stdin);

	while(1) {
		printf("\n>");
		fgets(messageString, messageLength, stdin);
		messageString[messageLength - 1] = '\0';

		if(strncmp(messageString,"quit\n",messageLength) != 0) {
			writeMessage(sock, messageString, size);
		}
		else {
			close(sock);
			exit(EXIT_SUCCESS);
		}
	}
}
