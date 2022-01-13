#include <stdio.h>
#include <string.h>

#define FRAME_SIZE 4

//entire source file is loaded into the ram[] array
//each line from the source file is loaded into its own cell in the ram[] array
//ram has a limit of a thousand line of code for 330 lines per program at max load
//NULL pointer to indicate no code at cell location in ram[]
//program in memory when all its lines have been copied into ram; error if not possible
int ramK = 0;	//current first empty cell
char *ram[40] = {NULL};


/*
int load(char *program) {
	int errorCode = 0;
	FILE *f; 
	f = fopen(program, "rt");
	if (f == NULL) { 
		errorCode = 3;
		printf("Error: Could not open"
		return errorCode;
	}
}
*/


void addToRAM(FILE *p, int *start, int *end) {
	char buffer[1000];
	*start = ramK;
		while(fgets(buffer, 999, p) != NULL) {
		if (ramK > 999) {
			printf("Error: Not enough RAM to add program");
			*start = -1;
			break;
		}
		if (buffer[0] != 13) {
			ram[ramK] = strdup(buffer);
			ramK++;
		}
		else continue;	
	}
	*end = ramK - 1;

}

void addLineToRAM(int position, char *line) {
	ram[position] = strdup(line);
}

void setToNULL(int position) {
	ram[position] = NULL;
}

void removeFromRAM(int pageTable[]) {
	for (int i = 0; i < 10; i++) {
		if (pageTable[i] == -1) continue;
		for (int j = 0; j < 4; j++) {
			ram[pageTable[i]*4 + j] = NULL;
		}
	}
}


void printRAM() {
	int i = 0;
	int j = -1;
	printf("RAM Contents: \n");
	while (i < 40) {
		if (ram[i] != NULL) {
			j = i;
		}
		i++;
	}

	for (int k = 0; k < j + 1; k++) {	
		if (ram[k] != NULL) {
			printf("%d %s", k, ram[k]);
		}	
	 	else {
			printf("%d NULL\n", k);	
		}	
	}
	printf("\n");
}

void resetRAM() {
	ramK = 0;
}
