/*
 * linkedlist.c
 *
 *  Created on: May 18, 2020
 *      Author: student
 */
#include <linkedlist.h>
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
void addHeader(rtp *pkgHeader)
{
	if(head == NULL)
	{
		head = malloc(sizeof(SentPackages));
		head->header = pkgHeader;
		head->timestamp = time(0);
		head->next = NULL;
		tail = (head);
		printf("Header was added in the linkedList");
		return;
	}
	else
	{
		addLast(pkgHeader);
	}
}

/*Add last in the linkedList of sent packages*/
void addLast(rtp *pkgHeader)
{
	SentPackages *current = head;
	
	while(current != NULL)
	{
		if(current->next == NULL)	
		{
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
void removehead()
{
	if(head == NULL)
	{
		printf("The list is empty, nothing to remove");
	}
	if(head->next == NULL)
	{
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
void printAllPackages()
{
	SentPackages *package = head;
	
	while(package != NULL)
	{
		printf("Seq: %d\n", package->header->seq);
		package = package->next;
	}
}


public static void main(String[] args)
{
	rtp *packageHeader = malloc(sizeof(rtp));
	
	packageHeader->seq = 1;
	
	addHeader(packageHeader);
	printAllPackages();

}

















