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
#define WAIT_SYNACK 5

extern rtp *setupHeader;

void writeMessage(int socket, char *message, size_t size, struct sockaddr_in serverAddress, socklen_t length);
void createSetupHeader(int type,int wsize);
rtp * readMessages(int socket);


#endif /* COMMON_H_ */
