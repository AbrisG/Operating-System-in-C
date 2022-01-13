#ifndef PCB_H_
#define PCB_H_

struct PCB {
	char *name;
	int PC;
       	int PC_page;	//which page the program is currently at
	int PC_offset;	//offset from the beggining of a frame (max 3)
	int pages_max; 	//max number of pages this program has
	int pageTable[10]; //index is page number, value is frame number
};

struct PCB* makePCB(char *filename);
void printPCB(struct PCB *pcb);

#endif
