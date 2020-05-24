/*
 * common.c
 *
 *  Created on: May 21, 2020
 *      Author: student
 */

#include "common.h"

/* writeMessage
 * Writes the message to the file (socket)
 * denoted by socket.
 */

void writeMessage(int socket, char *message, size_t size, struct sockaddr_in serverAddress, socklen_t length) {
	int nOfBytes;
	nOfBytes = sendto(socket, message, size, 0, (struct sockaddr *)&serverAddress, length);
	if(nOfBytes < 0) {
		perror("writeMessage - Could not write data\n");
		exit(EXIT_FAILURE);
	}
}


// Will wait until a packet is received. Will return with a pointer to a header
// if a packet is received within the time limit.
// If the packet had an incorrect CRC, the flags field will be set to WRONGCRC.
// If a timeout happens NULL will be returned.
rtp * readMessages(int socket, struct sockaddr* clientName, socklen_t *size) {

	int nOfBytes;
	fd_set set;
	FD_ZERO(&set);
	FD_SET(socket, &set);

	rtp *header = calloc(1, sizeof(rtp));
	if(header == NULL)
	{
		printf("Calloc failed...");
		exit(EXIT_FAILURE);
	}

	 struct timeval timeout;
	 timeout.tv_sec = 5;
	 timeout.tv_usec = 0;

	 int returnValue = select(sizeof(int), &set, NULL, NULL, &timeout);

	 if(returnValue > 0){
		nOfBytes = recvfrom(socket, (char*)header, sizeof(rtp), 0, clientName, size);
		if(nOfBytes < 0) {
			perror("Could not read data from client\n");
			exit(EXIT_FAILURE);
		}
		else
		{
			if(nOfBytes > 0) {
				// check if checksum is correct:
				uint16_t crc = header->crc;
				header->crc = 0;
				uint16_t actual = checksum((void*)header, sizeof(*header));
				//checks if the data is corrupted, will not return if it is
				if(crc != actual) {
					printf("Received packet with incorrect CRC! Packet says %d, actual crc is %d\n", crc, actual);
					printf("Package data = %s, %d, %d\n", header->data, header->seq, header->crc);
					header->flags = WRONGCRC;

					return header;
				}
				header->crc = crc;

				// print debug data regarding the packet

				switch (header->flags)
				{
				case SYN:
					printf("Incoming SYN \n");
					break;
				case SYNACK:
					printf("Incoming SYNACK \n");
					break;
				case ACK:
					printf("Incoming ACK \n");
					break;
				case FIN:
					printf("Incoming FIN \n");
					break;
				case FINACK:
					printf("Incoming FINACK \n");
					break;
				default:
					printf("Incoming packet \n");
					break;
				}
			}
		}
	 }
	 else if(returnValue == 0) // timeout
	 {
		 return NULL;
	 }
	 else
	 {
		 perror("Could not read from socket\n");
	 }

	 printf("Package data = %s, sequencenr: %d, crc: %d", header->data, header->seq, header->crc);
	 return header;
}

/*creates headers use in connectionSetup*/
rtp * createSetupHeader(int type, int wsize, char* data)
{
	return createHeader(type, wsize, data, -1);
}

rtp * createHeader(int type, int wsize, char* data, int seq)
{
	rtp* setupHeader = calloc(1, sizeof(rtp));
	if(!setupHeader)
	{
		printf("setupHeader message wasn't created, I couldn't allocate memory!\n");
		exit(EXIT_FAILURE);
	}

	setupHeader->flags = type;
	setupHeader->id = 0;
	setupHeader->seq = seq;
	setupHeader->windowsize = wsize;
	strcpy(setupHeader->data, data);
	setupHeader->crc = 0;
	setupHeader->crc = checksum((void*)setupHeader, sizeof(rtp));

	return setupHeader;
}



