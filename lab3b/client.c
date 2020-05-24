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
#define w_sending 6
#define w_waiting 7


rtp *header;
int event;
struct sockaddr_in serverName;
fd_set set;

void tear_down (int filedescriptor);

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


/* Removes the Syn message from packageList,
 * creates an ACK message and sends it to the server
 * */
void sendACKevent(int socket)
{
	removeHead(); //remove SYN from packageList

	rtp* setupHeader = createSetupHeader(ACK, WSIZE, "Here's an ACK!");
	addHeader(setupHeader);
	printf("Sending ACK with crc = %d\n", setupHeader->crc);

	send_with_random_errors(setupHeader, socket, serverName);

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

	printf("Sending SYN with crc = %d\n", setupHeader->crc);

	send_with_random_errors(setupHeader, socket, serverName);

	printf("SYN sent to the server at timestamp: %ld\n", time(0));
}

/*Starting up a connection with the server, three way hanshake*/
void connectionSetup(int fileDescriptor)
{
	rtp* packet;
	event = INIT;
	int state = WAIT_SYNACK;

	sendSYNevent(fileDescriptor);

	while(1)
	{
		//reads the response SYN-ACK and when the connection is established from the server
		packet = readMessages(fileDescriptor, NULL, NULL);

		switch (state)
		{
		  case WAIT_SYNACK:

			//client has received a response from the server, a SYN-ACK
			if (packet == NULL) // timeout
			{
				printf("----------TimeOut----------");
				sendSYNevent(fileDescriptor); //resend
			}
			else if (packet->flags == SYNACK)
			{
				state = ESTABLISHED;
				printf("Got SYNACK, SENDING AN ACK\n");
				sendACKevent(fileDescriptor);

				printf("Connection established with the server!\n\n");

				free(packet);

				return;
			}
			free(packet);
			break;

	/*	  case WAIT_TIMEOUT:
		  {
			//Server got a timeout and sent a SYN_ACK again, ACK got lost.
			//client sends ACK again
			if (packet->flags == SYNACK || packet->flags == WRONGCRC)
			{
				printf("OH NO! ACK is lost, I will send it again!\n");
				sendACKevent(fileDescriptor);
			}
			break;*/

		  default:
			  printf("Default reached!");
			  return;
			  break;
		}
	}
}

/*Creates a data message and adds it to the packageList*/
rtp *createDataMessage(int seqNumber)
{
	rtp *header = calloc(1, sizeof(rtp));
	if(header == NULL)
	{
		printf("Data message wasn't created, I couldn't allocate memory!\n");
		exit(EXIT_FAILURE);
	}

	strcpy(header->data, "Hello\n");
	header->flags = DATA;
	header->id = 1; //TODO WHAT TO SET HERE?
	header->seq = seqNumber;
	header->windowsize = WSIZE;
	header->crc = 0;
	header->crc = checksum((void*)header, sizeof(*header));
	addHeader(header);
	return header;
}


//function for the sliding window
void Slidingwindow(int filedescriptor)
{
	int state = w_sending;
	const int PACKETS_TO_SEND = 10;
	int sentPackets = 0; //  number of the packet that has been sent
	int ackedPackets = 0; // number of packets that have been acked


	rtp *header;
	socklen_t size = sizeof(struct sockaddr_in);

	while(1) // loops as long as there are packets to send
	{
		switch(state)
		{
			case w_sending:
				// Send all packages in sliding window
				//as long as the diff between wsize and ackedpackets are less than the wsize, we will send
				while(sentPackets - ackedPackets < WSIZE && sentPackets < PACKETS_TO_SEND)
				{
					sentPackets++;
					printf("Sending packet sequence no %d\n", sentPackets);
					header = createDataMessage(sentPackets);
					send_with_random_errors(header, filedescriptor, serverName);
					free(header);
				}
				state = w_waiting;
				break;

			case w_waiting:
				header = readMessages(filedescriptor, (struct sockaddr*) &serverName, &size);

				if (header == NULL)
				{
					printf("----------TimeOut----------\n");
					// resending packets:
					sentPackets = ackedPackets;
					state = w_sending;
				}
				else if (header->flags == SYNACK)
				{
					printf("Resent ACK since a new SYNACK was received");
					// If our last ack from the connection setup phase disappeared, the server will send a new SYNAC
					// and we'll have to send and ACK back
					sendACKevent(filedescriptor);
				}
				else if(header->flags == ACK)
				{
					//if the ack we receive is further than we last received
					if(header->seq > ackedPackets)
					{
						ackedPackets = header->seq; // the sequence number of the ack says how many packets the receiver has received
						printf("Received ack on packet %d\n", ackedPackets);
					}

					if(sentPackets < PACKETS_TO_SEND) //there are still packets to be sent
					{
						state = w_sending;
					}
					else if(ackedPackets == PACKETS_TO_SEND) // we've received acks on all packets
					{
						printf("ACK on the last packet has arrived, ready to close \n");
						free(header);
						return;
					}
				}
				free(header);
				break;
			default:
				printf("ERROR REACHED DEFAULT CASE IN SlidingWindow!\n");
				exit(NULL);
		}
	}
}

//teardown function
//handles the cases when the client wants to shut the connection down
void tear_down(int filedescriptor)
{
	rtp* packet;

	//first we need to send a FIN towards the server side
	rtp * finPacket = createSetupHeader(FIN, WSIZE, "Shut up, here's a FIN!");
	send_with_random_errors(finPacket, filedescriptor, serverName);
	free(finPacket);
	//addHeader(finPacket);
	printf ("FIN was send to server! time: %ld!\n" , time(0));

	int state = wait_FINACK;

	//loop to handle events from approaching packages
	while (1)
	{
		//receive packages
		packet = readMessages(filedescriptor, NULL, NULL);

		switch(state) //change current state of connection setup according to case
		{
			case wait_FINACK:
			{
				if (packet == NULL) // timeout
				{
					printf("----------TimeOut----------");
					finPacket = createSetupHeader(FIN, WSIZE, "Shut up, here's a FIN!");
					send_with_random_errors(finPacket, filedescriptor, serverName);
					free(finPacket);
				}
				else if(packet->flags == FINACK)
				 {
					 printf("FINACK received, sending ACK\n");
					 //FIN and ACK were received
					 state = CLOSED;

					 //removeHead();
					// event = INIT;

					 sendACKevent(filedescriptor);

					 printf("Connection closed\n");
					 free(packet);
					 return;
				 }
				free(packet);
				 break;
			}
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

	Slidingwindow(sock);

	tear_down(sock);

	close(sock);
	exit(EXIT_SUCCESS);
}
