/*
 * client.c
 *
 *  Created on: May 10, 2020
 *      Author: student
 */
#include "linkedlist.h"
#include "common.h"
#include <pthread.h>


#define PORT 5555
#define hostNameLength 50
#define messageLength  256
#define MAXMSG 512
#define WSIZE 2
#define w_sending 6
#define w_waiting 7


int  seqNumber;
rtp *header;
int state;
int event;
struct sockaddr_in serverName;
fd_set set;

void tear_down (int filedescriptor, socklen_t size);

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



/*all the clients are listening after a message
 * that will be sent to every client
 */
void* ListenToMessages(void *pointer)
{
	int *socket = (int*) pointer;

	while(1)
	{
		readMessages(*socket, NULL, NULL);
	}
}


/* Removes the Syn message from packageList,
 * creates an ACK message and sends it to the server
 * */
void sendACKevent(int socket)
{
	event = INIT;
	state = WAIT_TIMEOUT;
	removeHead(); //remove SYN from packageList

	rtp* setupHeader = createSetupHeader(ACK, WSIZE, "Here's an ACK!");
	addHeader(setupHeader);
	printf("Sending package with crc = %d\n", setupHeader->crc);

	writeMessage(socket,(char*) setupHeader, sizeof(rtp), serverName, sizeof(serverName));

	printf("ACK sent to the server at timestamp: %ld\n", time(0));
}



/* Sets the event and state variables for the connectionSetup loop
 * creates a SYN message and sends it to the server
 * Adds the SYN message to the packageList
 * */
void sendSYNevent(int socket)
{
	rtp* setupHeader = createSetupHeader(SYN, WSIZE, "Hi I want to talk, here's a SYN");
	addHeader(setupHeader);

	printf("Sending package with crc = %d\n", setupHeader->crc);

	writeMessage(socket,(char*) setupHeader, sizeof(rtp), serverName, sizeof(serverName));

	printf("SYN sent to the server at timestamp: %ld\n", time(0));
}

/*Starting up a connection with the server, three way hanshake*/
void connectionSetup(int fileDescriptor)
{
	rtp* packet;
	event = INIT;
	state = WAIT_SYNACK;

	sendSYNevent(fileDescriptor);

	while(1)
	{
		//reads the response SYN-ACK and when the connection is established from the server
		packet = readMessages(fileDescriptor, NULL, NULL);

		printf("Got a packet\n");

		switch (state)
		{
		  case WAIT_SYNACK:
		  {
			//client has received a response from the server, a SYN-ACK
			if (packet->flags == SYNACK)
			{
				printf("Got SYNACK, SENDING AN ACK\n");
				sendACKevent(fileDescriptor);
			}
			break;
		  }
		  case WAIT_TIMEOUT:
		  {
			//Server got a timeout and sent a SYN_ACK again, ACK got lost.
			//client sends ACK again
			if (event == SYNACK)
			{
				printf("OH NO! ACK is lost, I will send it again!\n");
				sendACKevent(fileDescriptor);
			}
			break;
		  }
		  case ESTABLISHED:
		  {
			 printf("Connection established with the server!");
			 removeHead();
			 return;
			 break;
		  }
		  default:
		  {
			  printf("Default reached!");
			  return;
			  break;
		  }

		}

	}
}

/*Creates a data message and adds it to the packageList*/
rtp *createDataMessage()
{
	rtp *header = malloc(sizeof(rtp));
	if(!header)
	{
		printf("Data message wasn't created, I couldn't allocate memory!\n");
		exit(EXIT_FAILURE);
	}

	strcpy(header->data, "Hello\n");
	header->flags = DATA;
	header->id = 1;
	header->seq = seqNumber;//add
	header->windowsize = WSIZE;
	header->crc = 0;
	header->crc = checksum((void*)&header, sizeof(header));
	addHeader(header);
	return header;
}


//function for the sliding window


void *Slidingwindow(void *data)
{
	int filedescriptor = (int)(*(int*)data);

	struct timeval timer;
	timer.tv_sec = 5;
	timer.tv_usec = 0;

	// gives the set zero bits for all filedescriptors
	FD_ZERO(&set);
	// sets all bits of sock in set
	FD_SET(filedescriptor, &set);

	socklen_t size = sizeof(struct sockaddr_in);
	rtp *header;
	int n0fBytes;

	header = (rtp*)calloc(1, sizeof(rtp));

	if (header == NULL){
		printf("calloc failed....\n"); // if calloc returns null, it failed
		exit(EXIT_FAILURE);
	}
	else
	{
		while(1)
		{
			switch(state)
			{
				case w_sending:
				writeMessage(filedescriptor, (char*) header, sizeof(rtp), serverName, sizeof(serverName));
				if(header->windowsize == WSIZE) //window is full
				{
					state = w_waiting;
				}

			/*	else if() //no more packets to send
				{
					state = w_waiting;
				}

				else if () //timeout, resend from N
				{
					state = w_sending;
				}*/

				else if(event == ACK)
				{
					header->windowsize++;
					state = w_sending;
				}

				break;

				case w_waiting:
				if(event == ACK) //ACK arrives
				{
/*					if() //there are still packets to be sent
					{
						header->windowsize++;
						state = w_sending;
					}

					else if() //timeout, resend from N
					{
						//destroy the packets and go back to sending
					}

					else if() //waiting for more acks
					{
						header->windowsize++;
						state = w_waiting;
					}
*/
				}

				break;
			}
		}
	}

	return NULL; // TODO!
}

//teardown function
//handles the cases when the client wants to shut the connection down
void tear_down (int filedescriptor, socklen_t size)
{
	rtp* packet;

	//first we need to send a FIN towards the server side
	rtp * finPacket = createSetupHeader(FIN, WSIZE, "Shut up, here's a FIN!");
	writeMessage(filedescriptor, (char*) finPacket, sizeof(rtp), serverName, sizeof(serverName));
	addHeader(finPacket);
	printf ("FIN was send to server! time: %ld!\n" , time(0));

	state = wait_FINACK;

	//loop to handle events from approaching packages
	while (1)
	{
		//receive packages
		packet = readMessages(filedescriptor, NULL, NULL);

		switch(state) //change current state of connection setup according to case
		{
			case wait_FINACK:
			{
				 if(packet->flags == FINACK)
				 {
					 printf("FINACK received, sending ACK\n");
					 //FIN and ACK were received
					 state = CLOSED;

					 removeHead();
					// event = INIT;

					 sendACKevent(filedescriptor);

					 printf("Connection closed\n");
					 return;
				 }
				 break;
			}

		/*	//when ACK gets lost and we
			case WAIT_TIMEOUT:
			{
				if (packet->flags == receive_FINACK)
				{
					//in case ACK is lost and we receive SYN and ACK before the time is out.
					printf("Wait for timeout. received FIN and ACK\n");
					removeHead();

					header->crc=0;
					header->crc = checksum ((void*) &header, sizeof(header));
					header->flags = send_ACK;

					writeMessage(filedescriptor, (char*) header, sizeof(rtp), serverName, sizeof(serverName));
					printf("ACK was send again to server. time: %ld\n", time(0));
					createSetupHeader(ACK, WSIZE);
				}
				break;
			}

			 case ESTABLISHED:
			{
				printf("Client shut down\n");
				removeHead();
				return;
				break;
			}*/
			default:
			{
				printf("ERROR REACHED DEFAULT CASE IN tear_down\n");
				break;
			}
		}
	}
}


int main(int argc, char *argv[]) {
	int sock;
	char hostName[hostNameLength];
	char messageString[messageLength];
	pthread_t thread1;
	int func1;

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

	connectionSetup(sock);

	//creating a thread that will handle the messages to every client
	func1 = pthread_create(&thread1, NULL, ListenToMessages, &sock);
	if (func1 != 0)
	{
		perror("Pthread is not working...");
		exit(EXIT_SUCCESS);
	}

	printf("\nType something and press [RETURN] to send it to the server.\n");
	printf("Type 'quit' to nuke this program.\n");
	fflush(stdin);

	while(1) {
		printf("\n>");
		fgets(messageString, messageLength, stdin);
		messageString[messageLength - 1] = '\0';

		// TODO call tear_down somewhere....

		if(strncmp(messageString,"quit\n",messageLength) != 0) {
		//	writeMessage(sock, messageString, strlen(messageString));
		}
		else {
			close(sock);
			exit(EXIT_SUCCESS);
		}
	}
}
