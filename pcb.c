#include "pcb.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/*add to pcb:
 * int PC_page;    //which page the program is currently at
        int PC_offset;  //offset from the beggining of a frame (max 3)
        int pages_max;  //max number of pages this program has
        int pageTable[10]; //index is page number, value is frame number
 */	

struct PCB* makePCB(char *filename) {
	struct PCB *pcb = malloc(sizeof(struct PCB));
	
	pcb->name = strdup(filename);	
	pcb->PC = 0;	
	pcb->PC_page = 0;
	pcb->PC_offset = 0;
	pcb->pages_max = 0;
	for (int i = 0; i < 10; i++) {
		pcb->pageTable[i] = -1;
	}

}
void printPageTable(int array[]) {
	printf("pageTable: [");
	for (int i = 0; i < 9; i++) {
		printf("%d, ", array[i]);
	}
	printf("%d]\n", array[9]);
}

void printPCB(struct PCB *pcb) {
	printf("---PCB Info---\n");
	printf("name: %s\tPC: %d\tPC_page: %d\tPC_offset: %d\tpages_max: %d\n",
		       	pcb->name, pcb->PC, pcb->PC_page, pcb->PC_offset, pcb->pages_max);
	printPageTable(pcb->pageTable);
	printf("\n");
}

