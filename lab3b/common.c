/*
 * common.c
 *
 *  Created on: May 21, 2020
 *      Author: student
 */

#include "common.h"

rtp * readMessages(int socket) {

	socklen_t size;
	struct sockaddr_in clientName;

	rtp *header = calloc(1, sizeof(rtp));
	if(header == NULL)
	{
		printf("Calloc failed...");
		exit(EXIT_FAILURE);
	}

	int nOfBytes = recvfrom(socket, (char*)header, sizeof(rtp), 0, (struct sockaddr *)&clientName, &size);
	if(nOfBytes < 0) {
		perror("Could not read data from client\n");
		exit(EXIT_FAILURE);
	}
	else
	{
		if(nOfBytes == 0)
		{
			free(header);
			return NULL;
		}
		else {
			// check if checksum is correct:
			uint16_t crc = header->crc;
			header->crc = 0;
			if(crc != checksum(header, sizeof(rtp))){
				printf("Received packet with incorrect CRC!\n");
				free(header);
				return NULL;
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
		}
	}
	return header;
}

