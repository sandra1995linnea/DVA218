/*
 * common.h
 *
 *  Created on: May 21, 2020
 *      Author: student
 */

#ifndef COMMON_H_
#define COMMON_H_

#include "linkedlist.h"

#define SYN 1
#define SYNACK 2
#define ACK 3
#define WAIT_SYN 4
#define WAIT_ACK 6
#define WAIT_SYNACK 5
#define FINACK 10
#define FIN 12

#define send_FIN 9
#define receive_FINACK 10
#define wait_FINACK 11
#define send_ACK 12
#define send_FINACK 13
#define wait_ACK 14
#define receive_ACK 15
#define WRONGCRC 16

#define WSIZE 2

extern rtp *setupHeader;

void writeMessage(int socket, char *message, size_t size, struct sockaddr_in serverAddress, socklen_t length);
rtp * createSetupHeader(int type, int wsize, char* data, int id);
rtp * readMessages(int socket, struct sockaddr* clientName, socklen_t *size);
rtp * createHeader(int type, int wsize, char* data, int seq, int id);
void send_with_random_errors(rtp *header, int sock, struct sockaddr_in serverAddress);

#endif /* COMMON_H_ */
