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


// Will wait until a packet is received with correct crc
rtp * readMessages(int socket) {

	socklen_t size;
	struct sockaddr_in clientName;
	int nOfBytes;

	rtp *header = calloc(1, sizeof(rtp));
	if(header == NULL)
	{
		printf("Calloc failed...");
		exit(EXIT_FAILURE);
	}

	while(1) {
		nOfBytes = recvfrom(socket, (char*)header, sizeof(rtp), 0, (struct sockaddr *)&clientName, &size);
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
				if(crc != checksum(header, sizeof(rtp))){
					printf("Received packet with incorrect CRC!\n");
					continue; // goes around the loop again
				}
				header->crc = crc;

				// print debug data regarding the packet
				if(header->flags == ACK){
					printf(">Incoming ACK \n");
				}
				else if(header->flags == SYN){
					printf("Incoming SYN \n");
				}
				else
					printf(">Incoming packet. \n");

				printf("Package data = %s, %d, %d\n", header->data, header->seq, header->crc);
				return header;
			}
		}
	}
}

/*creates headers use in connectionSetup*/
rtp * createSetupHeader(int type, int wsize)
{
	rtp* setupHeader = calloc(1, sizeof(rtp));
	setupHeader->flags = type;
	setupHeader->id = 0;
	setupHeader->seq = -1;
	setupHeader->windowsize = wsize;
	strcpy(setupHeader->data, "I want to start a Connection");
	setupHeader->crc = 0;
	setupHeader->crc = checksum((void*)setupHeader, sizeof(rtp));

	return setupHeader;
}
