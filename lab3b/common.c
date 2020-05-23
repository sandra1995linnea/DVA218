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
 int clientState = 0;
 int serverState = 0;


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

	int bytesRecv = select(sizeof(int), &set, NULL, NULL, &timeout);

	if(bytesRecv > 0) {
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

				//if checksum is wrong delete header, and inform receiver NULL means no packet
				if(crc != checksum(header, sizeof(rtp))){
					printf("Received packet with incorrect CRC!\n");
					header->flags = WRONGCRC;
					return header;
				}
				header->crc = crc;

				// print debug data regarding the packet

				switch (header->flags) {
				case SYN:
					printf("Incoming SYN \n");
					break;
				case SYNACK:
					printf("Incoming SYNACK \n");
					break;
				case ACK:
					printf(">Incoming ACK \n");
					break;
				case send_FIN:
					printf("Incoming FIN \n");
					break;
				case send_FINACK:
					printf("Incoming FINACK \n");
					break;
				case send_ACK:
					printf("Incoming ACK \n");
					break;
				default:
					printf("Incoming packet\n");
					break;
				}
			}
		}
	}
	else if(bytesRecv == 0)
	{
		clientState = ESTABLISHED;

		return NULL;
	}
	else
	{
		perror("Could not listen from socket\n");
		exit(EXIT_FAILURE);
	}

	printf("Package data = %s, %d, %d\n", header->data, header->seq, header->crc);
	return header;
}

/*creates headers use in connectionSetup*/
rtp * createSetupHeader(int type, int wsize, char* data)
{
	rtp* setupHeader = calloc(1, sizeof(rtp));
	setupHeader->flags = type;
	setupHeader->id = 0;
	setupHeader->seq = -1;
	setupHeader->windowsize = wsize;
	strcpy(setupHeader->data, data);
	setupHeader->crc = 0;
	setupHeader->crc = checksum((void*)setupHeader, sizeof(rtp));

	return setupHeader;
}
