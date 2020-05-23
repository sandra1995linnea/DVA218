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
#define WRONGCRC 16
#define TIMEOUT 17
#define LISTEN 18
#define RECEIVE 19


#define send_FIN 9
#define receive_FINACK 10
#define wait_FINACK 11
#define send_ACK 12
#define send_FINACK 13
#define wait_ACK 14
#define receive_ACK 15

extern rtp *setupHeader;
extern int clientState;
extern int serverState;

void writeMessage(int socket, char *message, size_t size, struct sockaddr_in serverAddress, socklen_t length);
rtp * createSetupHeader(int type, int wsize, char* data);
rtp * readMessages(int socket, struct sockaddr* clientName, socklen_t *size);


#endif /* COMMON_H_ */
