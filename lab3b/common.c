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



//reads messages with ID
rtp * readMessageID(int socket, struct sockaddr* clientName, socklen_t *size)
{
	rtp *header;
	while(1)
	{
		header = readMessages(socket, clientName, size);

		// If header is null, there was a timeout so return
		// otherwise check if the ID is correct
		if(header == NULL || header->id == clientID)
		{
			return header;
		}
		else
		{
			printf("Received packet with incorrect ID\n");
			free(header);
		}
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
				printf("\n");
				switch (header->flags)
				{
				case SYN:
					printf("Incoming SYN at timestamp: %ld\n", time(0));
				case SYNACK:
					printf("Incoming SYNACK at timestamp: %ld\n", time(0));
					break;
				case ACK:
					printf("Incoming ACK timestamp: %ld\n", time(0));
					break;
				case FIN:
					printf("Incoming FIN timestamp: %ld\n", time(0));
					break;
				case FINACK:
					printf("Incoming FINACK timestamp: %ld\n", time(0));
					break;
				default:
					printf("Incoming packet timestamp: %ld\n", time(0));
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

	 printf("Package data = %s, sequencenr: %d, crc: %d ID = %d\n", header->data, header->seq, header->crc, header->id);
	 return header;
}

/*creates headers use in connectionSetup*/
rtp * createSetupHeader(int type, int wsize, char* data, int id)
{
	return createHeader(type, wsize, data, -1, id);
}

rtp * createHeader(int type, int wsize, char* data, int seq, int id)
{
	rtp* setupHeader = calloc(1, sizeof(rtp));
	if(!setupHeader)
	{
		printf("setupHeader message wasn't created, I couldn't allocate memory!\n");
		exit(EXIT_FAILURE);
	}

	setupHeader->flags = type;
	setupHeader->id = id;
	setupHeader->seq = seq;
	setupHeader->windowsize = wsize;
	strcpy(setupHeader->data, data);
	setupHeader->crc = 0;
	setupHeader->crc = checksum((void*)setupHeader, sizeof(rtp));

	return setupHeader;
}


void send_with_random_errors(rtp *header, int sock, struct sockaddr_in serverAddress) {
	int rand_num = rand() % 10;

	switch(rand_num)
	{
	case 0:
		//healthy package again
		writeMessage(sock, (char*)header, sizeof(rtp), serverAddress, sizeof(serverAddress));
		break;
	case 1:
		//corrupt packet
		printf("------Sending corrupt package------\n");
		rtp  *corrupt_header = malloc(sizeof(rtp));
		strcpy(corrupt_header->data, "i am bad\0");

		corrupt_header->windowsize = WSIZE;
		corrupt_header->id = 1;
		corrupt_header->flags = DATA;
		corrupt_header->seq = header->seq;
		corrupt_header->crc = 1555;

		writeMessage(sock, (char*) corrupt_header, sizeof(rtp), serverAddress, sizeof(serverAddress));
		free(corrupt_header);
		break;

	case 2:
		//creates a wrong order package with sequence number 2
		printf("-------Sending package with wrong seqnr-------\n");
		rtp *corrupt_seqnr_header = malloc(sizeof(rtp));
		strcpy(corrupt_seqnr_header->data, "i am bad\0");

		corrupt_seqnr_header->windowsize = WSIZE;
		corrupt_seqnr_header->id = 1;
		corrupt_seqnr_header->flags = DATA;
		corrupt_seqnr_header->seq = 2;
		corrupt_seqnr_header->crc = checksum((void*) corrupt_seqnr_header, sizeof(*corrupt_seqnr_header));

		writeMessage(sock, (char*)corrupt_seqnr_header, sizeof(rtp), serverAddress, sizeof(serverAddress));
		free(corrupt_seqnr_header);
		break;

	case 3:
		//healthy package again
				writeMessage(sock, (char*)header, sizeof(rtp), serverAddress, sizeof(serverAddress));
				break;

	case 4:
		//healthy package again
				writeMessage(sock, (char*)header, sizeof(rtp), serverAddress, sizeof(serverAddress));
				break;
	case 5:
		//healthy package again
				writeMessage(sock, (char*)header, sizeof(rtp), serverAddress, sizeof(serverAddress));
				break;

	default:
		//healthy package again
		writeMessage(sock, (char*)header, sizeof(rtp), serverAddress, sizeof(serverAddress));
		break;
	}
}

