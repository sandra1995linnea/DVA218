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
rtp * readMessages(int socket, struct sockaddr* clientName, socklen_t *size) {

	int nOfBytes;

	rtp *header = calloc(1, sizeof(rtp));
	if(header == NULL)
	{
		printf("Calloc failed...");
		exit(EXIT_FAILURE);
	}

	while(1) {
		nOfBytes = recvfrom(socket, (char*)header, sizeof(rtp), 0, clientName, size);
		if(nOfBytes < 0) {
			perror("Could not read data from client\n");
			exit(EXIT_FAILURE);
		}
		else
		{
			if(nOfBytes > 0) {
				// check if checksum is correct:
				int crc = header->crc;
				header->crc = 0;
				int actual = checksum((void*)header, sizeof(*header));
				if(crc != actual) {
					printf("Received packet with incorrect CRC! Packet says %d, actual crc is %d\n", crc, actual);
					printf("Package data = %s, %d, %d\n", header->data, header->seq, header->crc);

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
rtp * createSetupHeader(int type, int wsize, char* data)
{
	rtp* setupHeader = calloc(1, sizeof(rtp));
	if(!setupHeader)
	{
		printf("setupHeader message wasn't created, I couldn't allocate memory!\n");
		exit(EXIT_FAILURE);
	}

	setupHeader->flags = type;
	setupHeader->id = 0;
	setupHeader->seq = -1;
	setupHeader->windowsize = wsize;
	strcpy(setupHeader->data, data);
	setupHeader->crc = 0;
	setupHeader->crc = checksum((void*)setupHeader, sizeof(rtp));

	return setupHeader;
}
