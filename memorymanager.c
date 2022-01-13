#include <stdlib.h>
#include <stdio.h>
#include "ram.h"
#include "kernel.h"
#include "pcb.h"
#include <string.h>

#define FRAME_SIZE 4
#define RAM_SIZE 40

int countTotalPages(FILE *f) {
	int totalPages;
	int totalLines = 0;
	char c;
		
	c = fgetc(f);
	while(c != EOF) {
      		if(c == '\n')
         		totalLines++;
		c = fgetc(f);
   	}

	totalPages = ((totalLines + FRAME_SIZE - 1)/ FRAME_SIZE);
		

	rewind(f);

	return totalPages;
}


void printPage(struct PCB *pcb, int pageNumber) {
	printf("[");
	for (int i = 0; i < 3; i++) {
		if (ram[(pcb->pageTable[pageNumber])*4 + i] == NULL) {
			printf("NULL ");
		}
		else {	
			printf("'%s', ", ram[(pcb->pageTable[pageNumber])*4 + i]);
		}
	}	
	if (ram[(pcb->pageTable[pageNumber])*4 + 3] == NULL) {
		printf("NULL]\n");
        }
        else {
		printf("'%s']\n", ram[(pcb->pageTable[pageNumber])*4 + 3]);
        }
	
}


int loadPage(int pageNumber, FILE *f, int frameNumber) {
	int errorCode = 0;
	int fileLineIndex = pageNumber * FRAME_SIZE;			
	int ramIndex = frameNumber * FRAME_SIZE;
	char buffer[1024];
	
	//printf("loadPage Debug: \n");

	//iterates to the correct line
	for (int i = 0; i < 40; i++) {
		if (i == fileLineIndex) {
			break;
		}
		if (fgets(buffer, sizeof(buffer), f) == NULL) {
			errorCode = 3;
			return errorCode;
		} 
	}	

	for (int i = 0; i < FRAME_SIZE; i++) {
		if (fgets(buffer, sizeof(buffer), f) == NULL) {
			setToNULL(ramIndex + i);
                }
		else {
			addLineToRAM(ramIndex + i, buffer);
		}
	}
	rewind(f);

	return errorCode;
}


int findFrame() {
	for (int i = 0; i < (RAM_SIZE/FRAME_SIZE); i++) {
		if (ram[i * FRAME_SIZE] == NULL) 
			return i;
	}

	return -1;
}

int findVictim(struct PCB *p) {
       int random = rand() % (RAM_SIZE/FRAME_SIZE);
       for (int i = 0; i < 10; i++) {
	       if (p->pageTable[i] == random) {
		       random = rand() % (RAM_SIZE/FRAME_SIZE);
		       i = 0;
	       }
       }
       return random;
}


int updatePageTable(struct PCB *p, int pageNumber, int frameNumber, int victimFrame) { 
	int errorCode = 0;
	struct PCBnode *temp = head;

	//there is no available frame, use victim's frame
	if (frameNumber == -1) {
		//assign victims frame to pageNumber of current PCB
		p->pageTable[pageNumber] = victimFrame;	

		//iterate across ready list to find victimPCB and change its frame to -1 (invalid)
		while (temp != NULL) {
                	for (int page = 0; page < 10;  page++) {
                        	if (temp->value->pageTable[page] == victimFrame) {
                                	temp->value->pageTable[page] = -1;
					return errorCode;
                        	}
                	}

                	temp = temp->next;
        	}
		//could not find the victimFrame's PCB
		errorCode = 3;
	}
	else {
		p->pageTable[pageNumber] = frameNumber;
	}

	return errorCode;
}

int loadFromMemory(struct PCB *pcb) {
	FILE *f;
	int errorCode = 0, frameNumber, victimFrame;	
	char systemCall[1024];
	
	snprintf(systemCall, sizeof(systemCall), "BackingStore/%s", pcb->name);
	f = fopen(systemCall, "r");
	if (f == NULL) {
		printf("Could not open file '%s'\n", systemCall);
		errorCode = 3;
		return errorCode;
	}	

	frameNumber = findFrame();
	if (frameNumber == -1) {
		victimFrame = findVictim(pcb);
		errorCode = loadPage(pcb->PC_page, f, victimFrame);
		if (errorCode != 0) return errorCode;
		errorCode = updatePageTable(pcb, pcb->PC_page, frameNumber, victimFrame);
	}
	else {
		loadPage(pcb->PC_page, f, frameNumber);
		updatePageTable(pcb, pcb->PC_page, frameNumber, -1);
	}
	fclose(f);
	
	return errorCode;

}

int launcher(FILE *p, char *filename) {
	int errorCode = 0;
	char systemCall[1024];
	char c;
	FILE *pCopy;
	
	//copy the file into BackingStore	
	snprintf(systemCall, sizeof(systemCall), "BackingStore/%s", filename);
	
	pCopy = fopen(systemCall, "r+");

	if (pCopy == NULL) {
		pCopy = fopen(systemCall, "w+");
		if (pCopy == NULL) {
			errorCode = 3;
			printf("Could not open file '%s'\n", systemCall);
			return errorCode;
		}
		c = fgetc(p);
		while (c != EOF) {
			fputc(c, pCopy);
			c = fgetc(p);
		}
	}

	//close original file
	fclose(p);
	rewind(pCopy);


	struct PCB *pcb = makePCB(filename);
	int maxPages = countTotalPages(pCopy);
	int frameNumber, victimFrame;
	
	//initializes the pages_max to maxPages
	pcb->pages_max = maxPages;

	for (int page = 0; page < maxPages && page < 2; page++) {
		frameNumber = findFrame();
		if (frameNumber == -1) {
			victimFrame = findVictim(pcb);
			loadPage(page, pCopy, victimFrame);
			updatePageTable(pcb, page, frameNumber, victimFrame);
		}
		else {
			loadPage(page, pCopy, frameNumber);
			updatePageTable(pcb, page, frameNumber, -1);	
		}
	}
		

		
	//initializes PC to index of first frame in pageTable	
	pcb->PC = pcb->pageTable[0] * FRAME_SIZE;
	
	addToReadyList(pcb);
	fclose(pCopy);

	return errorCode;
}
