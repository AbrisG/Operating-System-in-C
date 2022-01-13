#include "cpu.h"
#include "ram.h"
#include "shell.h"
#include <stdlib.h>
#include <string.h>

struct CPU initCPU = {
	.quanta = 2	
};

struct CPU *cpu = &initCPU;

void printChars(char chars[]) {
	int i = 0;
	while (chars[i] != 0) {
		printf("%c ", chars[i]);
		i++;
	}
	printf("\n");
}

int runCPU(int quanta) {
	int errorCode = 0;

	for (int i = 0; i < quanta; i++) {
		memset(cpu->IR, '\0', sizeof(cpu->IR));
		if (ram[cpu->IP + cpu->offset] == NULL) {
			errorCode = -2;	//indicates end of file
			return errorCode;
		}
		strcpy(cpu->IR, ram[cpu->IP + cpu->offset]); //copies instruction into IR
		errorCode = parse(cpu->IR);	
		if (errorCode != 0) {
			return errorCode;
		}
		cpu->offset++;
		 if (cpu->offset >= 4) {
                        errorCode = -1; //indicates page fault
                        return errorCode;
                }
	}

	return errorCode;
}
