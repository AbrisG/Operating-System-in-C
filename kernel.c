#include "shell.h"
#include "DISK_driver.h"
#include "ram.h"
#include "pcb.h"
#include "kernel.h"
#include "cpu.h"
#include "memorymanager.h"
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <errno.h>

struct PCBnode *head;
struct PCBnode *tail;


void addToReadyList(struct PCB *pcb) {
	struct PCBnode *new, *temp;
	new = malloc(sizeof(struct PCBnode));
	new->value = pcb;
	new->next = NULL;

	if (head == NULL && tail == NULL) {
		head = new;
		tail = new;
	}
	else {
		temp = tail;
		tail = new;
		temp->next = new;

	}
}

void placeAtEnd() {
	if (head == tail) {
		return;
	}
	else {
		tail->next = head;
		tail = tail->next;
		head = head->next;
		tail->next  = NULL;
	}
}

void removeFromReadyList() {
	struct PCBnode *temp = head;
	head = head->next;
	free(temp->value);
	free(temp);	

}

void printReadyList() {
	struct PCBnode *temp = head;
	printf("Ready List Contents: \n");
	while (temp != NULL) {
		printf("%d\n", temp->value->PC);
		temp = temp->next;
	}	
	printf("\n");

}

void resetReadyList() {
	head = NULL;
	tail = NULL;
}



int scheduler() {
        int errorCode = 0;
	struct PCB *p;
        while(head != NULL) {
		p = head->value;

                //copy PC of pcb into IP of cpu
                cpu->IP = head->value->PC;
		cpu->offset = p->PC_offset;

		errorCode = runCPU(cpu->quanta);
		
		//error
                if (errorCode > 0) return errorCode;
		
		//page fault
		else if (errorCode == -1) {
			//if the next page still belongs to current process, find index of next
			//frame and place it into PC 
			if (p->PC_page + 1 <  p->pages_max){
				p->PC_page++;
				p->PC_offset = 0;
				if (p->pageTable[p->PC_page] != -1) {
				}
				else {
					loadFromMemory(p);		
				}
				p->PC = p->pageTable[p->PC_page] * 4;
				placeAtEnd();
				
			}

			//otherwise, program is finished	
			else {
				removeFromRAM(p->pageTable);
				removeFromReadyList();

			}
		}

		//if CPU ran out of lines of code during quanta
		else if (errorCode == -2) {
			removeFromRAM(p->pageTable);
			removeFromReadyList();
		}

		//quanta finished, palce at end (do i check if it has gone over the) 
		else {
			p->PC = cpu->IP;
			p->PC_offset = cpu->offset;
			placeAtEnd();
		}

        }
        return errorCode;

}




int kernel() {
	int errorCode = 0;
	printf("Kernel 1.0 loaded!\n");
	errorCode = shellUI();
	return errorCode;
}

int boot() {
	int errorCode = 0;
	DIR *dir = opendir("BackingStore");
	if (dir) {
		closedir(dir);
		system("rm -rf BackingStore");
		system("mkdir BackingStore");
	 
	}
	else {	
		system("mkdir BackingStore");
	}

	//test
	initIO();

	return errorCode;
}

int main() {
	int error = 0;
	boot();
	error = kernel();
	return error;
}
