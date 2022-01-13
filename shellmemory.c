// has private data structures and public functions that implement shell memory
#include <string.h>
#include "shellmemory.h"
#include <stdlib.h>

struct MEM memoryArray[1000];

void add(char *var, char* value) {
	int i = 0;
	while (memoryArray[i].var != NULL) {
		if (strcmp(memoryArray[i].var, var) == 0) {
			memoryArray[i].value = value;
			break;	
		}
		i++;
	}
	memoryArray[i].var = (char *)malloc(sizeof(strlen(var) + 1));
	memoryArray[i].value = (char *)malloc(sizeof(strlen(value) + 1));
	strcpy(memoryArray[i].var, var);
	strcpy(memoryArray[i].value, value);
		
}


char* find(char *var) {
	int i = 0;
	while (memoryArray[i].var != NULL) {
		if(strcmp(memoryArray[i].var, var) == 0) {
			return memoryArray[i].value;
		}
		i++;
	}
	return NULL;
}
