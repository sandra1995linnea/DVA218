/*
 * linkedlist.h
 *
 *  Created on: May 20, 2020
 *      Author: student
 */

#ifndef LINKEDLIST_H_
#define LINKEDLIST_H_
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/times.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/select.h>
#include <stdbool.h>

#define MAXMSG 512
#define DATA 8
#define ACK 3
#define WAIT_TIMEOUT 6
#define ESTABLISHED 7
#define INIT 0

bool loop;

/*header*/
typedef struct
{
	int flags;
	int id;
	int seq;
	int windowsize;
	uint16_t crc;
	char data[MAXMSG];
	//char *data;
} rtp; // realiable transport protocol

/* Linked list of sent packages with their timestamps*/
typedef struct PackageList
{
	rtp *header;
	time_t timestamp;
	struct PackageList *next;
}SentPackages;

extern SentPackages *head;
extern SentPackages *tail;
extern bool loop;


void addLast(rtp *pkgHeader);
void addHeader(rtp *pkgHeader);
void removeHead();
void printAllPackages();
uint16_t checksum(void *header, size_t headerSize);



#endif /* LINKEDLIST_H_ */
