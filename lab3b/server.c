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
#define WAIT_ACK 9
#define w_receiving 6

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

// sending a message to the client, that we got the message
void respondToClient(int fileDescriptor, char *message)
{
	int nrOfBytes;
	//nrOfBytes = sendto(fileDescriptor, message, strlen(message) + 1, 0, (struct sockaddr *)&clientName, sizeof(clientName));
/*	if(nrOfBytes < 0) {
		perror("writeMessage - Could not write data\n");
		exit(EXIT_FAILURE);
	}*/
}

void *Slidingwindow(void *data)
{
	int filedescriptor = (int)(*(int*)data);

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
		{   //mottagaren står bara i ett tillstånd, håll koll på seqnr
			while(1)
			{
				switch(state)
				{
					case: w_receiving

					//packet data error check

					if()//approved data
					{
						//send ACK
					}
					else
					{
						//destroypackets
					}

					break;
				}
			}
		}
}



/* Initializes event and state variables for connectionSetup loop
 * creates an SYN-ACK message and sends it to the client
 * */
void sendSynACKevent(int socket)
{
	event = INIT;
	state = WAIT_SYN;

	createSetupHeader(SYNACK, 2);
	rtp *header = setupHeader;

	printf("Sending package with crc = %d\n", header->crc);

	//writeMessage(socket,(char*) header, sizeof(rtp), serverName, sizeof(serverName));

	printf("SYN-ACK sent to the client at timestamp: %ld\n", time(0));

}

/*Starting up a connection with the client, three way hanshake*/
void connectionSetup(int fileDescriptor)
{
	sendSynACKevent(fileDescriptor);

		while(1)
		{
			//reads the SYN and ACK message from client
			//event = readMessage(fileDescriptor, size);

			switch (state)
			{
			  case WAIT_SYN:
			  {
				//Server has received a response from the client, a SYN message
				if (event == SYN)
				{
					state = WAIT_ACK;
					sendSynACKevent(fileDescriptor);
				}
				break;
			  }
			  case WAIT_ACK:
			  {
				//Server received an ACK from client
				if (event == ACK)
				{
					printf("Server is connected\n");
					return;
				}
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

//teardown function. Hnadles the cases when the server wants to end a connection
void tear_down(int filedescriptor, socklen_t size)
{
	int event = INIT;
	rtp header;

	header.windowsize = 1;
	header.id = 0;
	header.seq = -1;
	header.flags = send_FINACK;

	header.crc = 0;
	header.crc = checksum ((void*) &header, sizeof(header));

	writeMessage(filedescriptor, header, size);
	create_header (&header);
	printf("FIN and ACK were sent to the server. time: %ld\n", time(0));

	//waiting for ACK
	state = wait_ACK;

	//loop that handles the different cases of teardown
	while(loop == true)
	{
		//handle incoming packages
		event = read_message(filedescriptor, size);

		//we use a switch in order to handle the different possible scenarios of teardown
		switch(state)
		{
			case wait_ACK:
			{
				if(event == receive_ACK)
				{
					printf("Server shut down\n");
					state = ESTABLISHED;
					event = INIT;

					l_state = false;
					close(filedescriptor);
					return;
				}
				break;
			}

			default:
			{
				printf("Teardown is complete \n");
				return;
				break;
			}
		}
	}
}

int main(int argc, char *argv[]) {
	int sock;
	fd_set activeFdSet, set; /* Used by select */
	//socklen_t size;

	/* Create a socket and set it up to accept connections */
	sock = makeSocket(PORT);

	//size = sizeof(struct sockaddr_in);

	/* Initialize the set of active sockets */
	//saving all the active sockets in a struct
	FD_ZERO(&set);
	FD_SET(sock, &set);

	printf("\n[waiting for connections...]\n");

	while(1) {

		/* Data arriving on an already connected socket */
		if(readMessages(sock) < 0)
		{
			close(sock);
			FD_CLR(sock, &activeFdSet);
		}
	}
}
