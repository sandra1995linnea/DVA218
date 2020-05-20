/*
 * linkedlist.c
 *
 *  Created on: May 18, 2020
 *      Author: student
 */
#include "list.h"
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <math.h>
#include <sys/times.h>
#include <time.h>
#include <stdbool.h>

SentPackages *head = NULL;
SentPackages *tail = NULL;

/*Add header in the linked list of sent packages*/
void addHeader(rtp *pkgHeader) {
	if (head == NULL) {
		head = malloc(sizeof(SentPackages));
		head->header = pkgHeader;
		head->timestamp = time(0);
		head->next = NULL;
		tail = (head);
		printf("Header was added in the linkedList");
		return;
	} else {
		addLast(pkgHeader);
	}
}

/*Add last in the linkedList of sent packages*/
void addLast(rtp *pkgHeader) {
	SentPackages *current = head;

	while (current != NULL) {
		if (current->next == NULL) {
			current->next = malloc(sizeof(SentPackages));
			current->next->header = pkgHeader;
			current->next->timestamp = time(0);
			current->next->next = NULL;
			tail = current->next;
			printf("Header was added last in the linkedList");
			return;
		}

		current = current->next;
	}

}

/*Removes first package in the list, which is the last package sent*/
void removehead() {
	if (head == NULL) {
		printf("The list is empty, nothing to remove");
	}
	if (head->next == NULL) {
		free(head);
		head = NULL;
		tail = NULL;
		return;
	}

	SentPackages *tmp = head;
	head = head->next;
	free(tmp);
}

/*Prints the sequence number of all the packages in the list*/
void printAllPackages() {
	SentPackages *package = head;

	while (package != NULL) {
		printf("Seq: %d\n", package->header->seq);
		package = package->next;
	}
}

/*Internet Checksum*/
uint16_t checksum(void *header, size_t headerSize)
{
	char *data = (char*) header;

	uint32_t sum = 0xffff;

	// adds the 16bit blocks and sumatesit to sum variables
	for (size_t i = 0; i + 1 < datalength; i += 2) 
	{
		uint16_t word;
		memcpy(&word, data + 1, 2);
		sum += ntohs(word);
		//if sum variable is bigger thatn 16bits (max ) substackt 16 bits
		if (sum > 0xffff) {
			sum -= 0xffff;
		}
	}

	/*Handles partial block at the end of data, if data summation is not evenly divideable*/
	if (datalength & 1)
	{
		uint16_t word = 0;
		memcpy(&word, data + datalength - 1, 1);
		sum += ntohs(word);

		if (sum > 0xffff) 
		{
			sum -= 0xffff;
		}

	}

	// Return the checksum in network Byte order, ~means it performs a bitwise complement, ex. 1011 to 0100
	return htons(~sum);
}

/*int main(int argc, char *argv[])
 {
 rtp *packageHeader = malloc(sizeof(rtp));

 packageHeader->seq = 1;
 packageHeader->crc = 0;
	strcpy(packageHeader->data, "Hello\0");

	packageHeader->crc = checksum((void*)packageHeader, sizeof(*packageHeader));
	uint16_t x;
	x = packageHeader->crc;

 addHeader(packageHeader);
 printAllPackages();

 }*/

