/*
 * server.c
 *
 *  Created on: May 10, 2020
 *      Author: student
 */
#include "linkedlist.h"
#include "common.h"

#define PORT 5555
#define MAXMSG 512
#define WSIZE 2
#define w_receiving 65

int state;
int event;
struct sockaddr_in clientName;
socklen_t clientNameLength = sizeof(struct sockaddr_in);

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

bool l_state = true;
int event = INIT;
void tear_down (int filedescriptor);

int makeSocket(unsigned short int port) {
	int sock;
	struct sockaddr_in name;

	/* Create a socket. */
	sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP); //make an UDP socket
	if(sock < 0) {
		perror("Could not create a socket\n");
		exit(EXIT_FAILURE);
	}

	memset((char*)&name, 0, sizeof(name));
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

//Sending an ACK on the approved package
void sendACKevent(int socket, int seqnr)
{
	removeHead(); //remove SYN from packageList

	rtp* setupHeader = createSetupHeader(ACK, WSIZE, "Here's an ACK!");
	addHeader(setupHeader);
	printf("Sending package with crc = %d\n", setupHeader->crc);

	writeMessage(socket,(char*) setupHeader, sizeof(rtp), clientName, sizeof(clientName));

	printf("ACK sent to the server at timestamp: %ld\n", time(0));
}


void Slidingwindow(int filedescriptor)
{

	socklen_t size = sizeof(struct sockaddr_in);
	rtp *header;
	int seqnr = 0; //keeps track if the packets are coming in the right order or not

   //mottagaren står bara i ett tillstånd, håll koll på seqnr
	while(1)
	{
		switch(state)
		{
			case w_receiving:

				header = readMessages(filedescriptor, (struct sockaddr*) &clientName, &size);
				seqnr++;

				//packet data error check
				if(header->seq == seqnr && header->flags == DATA)//approved data
				{
					// print data:
					printf("Data: %s, crc: %d, seq: %d", header->data, header->crc, header->seq);

					sendACKevent(filedescriptor, seqnr);//sending an ack on the package
				}
				if(header->flags == FIN) // if we received a FIN, the client does not want to stay connected
				{
					printf("No more packets to be sent, client is done sending");
					return;
				}
				else
					printf("Received and threw away packet that was either out of order or not a data packet\n");

				// free allocated memory
				free(header);
				header = NULL;

				break;

			default:
				printf("ERROR DEFAULT CASE REACHED IN Slidingwindow");
				return;
		}
	}

}


/* Initializes event and state variables for connectionSetup loop
 * creates an SYN-ACK message and sends it to the client
 * */
void sendSynACKevent(int socket)
{
	rtp *setupHeader = createSetupHeader(SYNACK, 2, "Look a SYNACK!");

	printf("Sending package with crc = %d\n", setupHeader->crc);

	writeMessage(socket,(char*) setupHeader, sizeof(rtp), clientName, clientNameLength);

	printf("SYN-ACK sent to the client at timestamp: %ld\n", time(0));
}

/*Starting up a connection with the client, three way handshake*/
void connectionSetup(int fileDescriptor)
{
	//sendSynACKevent(fileDescriptor);
	rtp* packet;
	state = WAIT_SYN;

	while(1)
	{
		//reads the SYN and ACK message from client
		packet = readMessages(fileDescriptor, (struct sockaddr*) &clientName, &clientNameLength);

		switch (state)
		{
		  case WAIT_SYN:
		  {
			//Server has received a response from the client, a SYN message
			if (packet->flags == SYN)
			{
				state = WAIT_ACK;
				sendSynACKevent(fileDescriptor);
				printf("SYN received, sent SYNACK, waiting for ACK\n");
			}
			break;
		  }
		  case WAIT_ACK:
		  {
			//Server received an ACK from client
			if (packet->flags == ACK)
			{
				printf("Ack received, server is connected\n");
				return;
			}
			break;
		  }
		  default:
		  {
			  printf("Default reached!\n");
			  return;
			  break;
		  }
		}
	}
}

//teardown function. Handles the cases when the server wants to end a connection
void tear_down(int filedescriptor)
{
	rtp* setupHeader = createSetupHeader(FINACK, WSIZE, "Look a FINACK!");
	writeMessage(filedescriptor, (char*) &setupHeader, sizeof(rtp), clientName, sizeof(clientName));
	addHeader(setupHeader);

	printf("FINACK was sent to the server. time: %ld\n", time(0));

	//waiting for ACK
	state = wait_ACK;

	//loop that handles the different cases of teardown
	while(1)
	{
		//handle incoming packages
		rtp* receivedPacket = readMessages(filedescriptor, NULL, NULL);

		//we use a switch in order to handle the different possible scenarios of teardown
		switch(state)
		{
			case wait_ACK:
			{
				if(receivedPacket->flags == ACK)
				{
					printf("Server shut down\n");
					state = CLOSED;
					event = INIT;

					close(filedescriptor);
					return;
				}
				break;
			}

			default:
			{
				printf("ERROR REACHED DEFAULT CASE IN tear_down!\n");
				return;
			}
		}
	}
}

int main(int argc, char *argv[]) {
	int sock;
	fd_set activeFdSet, set; /* Used by select */

	/* Create a socket and set it up to accept connections */
	sock = makeSocket(PORT);

	/* Initialize the set of active sockets */
	//saving all the active sockets in a struct
	FD_ZERO(&set);
	FD_SET(sock, &set);

	printf("\n[waiting for connections...]\n");

	connectionSetup(sock);

	printf("\n[waiting for messages...]\n");

	Slidingwindow(sock);

	tear_down(sock);
}
