#include "pcb.h"

#ifndef KERNEL_H
#define KERNEL_H

int myinit(char *filename);

void addToReadyList(struct PCB *pcb);

void printReadyList();
void resetReadyList();

struct PCBnode {
	struct PCB *value;
	struct PCBnode *next;
};

struct PCBnode *head;
struct PCBnode *tail;

int scheduler();

#endif
