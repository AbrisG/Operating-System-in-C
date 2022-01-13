// contains interpreter() function
// each command that interpreter() accepts has a corresponding function that implements the commands functionality


#include <stdio.h>
#include "shell.h"
#include <string.h>
#include <stdlib.h>
#include "shellmemory.h"
#include "ram.h"
#include "kernel.h"
#include <time.h>
#include "memorymanager.h"
#include "DISK_driver.h"
//helper functions

void printWords(char *words[]) {
	int i = 0;
	while (words[i] != NULL) {
		printf("%s ", words[i]);
		i++;
	}
	printf("\n");
}

int getNumberOfArgs(char *words[]) {
	int i = 1;
        while (words[i] != NULL) {
                i++;
        }
	return i - 1;
}

int correctNumberOfArgs(char *words[], int numberOfArgs) {
	int i = 1;
	while (words[i] != NULL) {
		i++;
	}
	return numberOfArgs == (i - 1);
}


void freeWords(char *words[]) {
	int i = 0;
	while (words[i] != NULL) {
		free((char *) words[i]);
		i++;
	}
}

//shell commands

int help(char *words[]) {
	int errorCode = 0;
	if (correctNumberOfArgs(words, 0)) {
		printf("Available Commands:\n"
				"help -- displays all commands\n"
				"quit -- exits the shell\n"
				"set VAR STRING -- assigns value STRING to variable VAR\n"
				"print VAR -- displays the STRING assigned to VAR\n"
				"run SCRIPT.txt -- executes the file SCRIPT.txt\n"
				"exec PROG1.txt PROG2.txt PROG3.txt -- loads into ram and concurrently executes up to 3 programs\n"
				"mount PARTITION_NAME NUM_BLOCKS BLOCK_SIZE -- mounts an existing partition or creates a new partition with PARTITON_NAME and NUM_BLOCKS*BLOCK_SIZE space\n"
				"write FILE_NAME [STRING] -- writes STRING to file with FILE_NAME\n"				    "read FILE_NAME VAR -- reads the contents of file FILE_NAME into variable VAR\n"
);
	}	
	else {
		errorCode = 2;
	}
	return errorCode;
}

int quit(char *words[]) {
	int errorCode = 4;
	printf("Bye!\n");
	return errorCode;
}

int set(char *words[]) {
	int errorCode = 0;
	if (correctNumberOfArgs(words, 2)) {
		add(words[1], words[2]);
	}
	else {
		errorCode = 2;
	}
	
	return errorCode;

}

int print(char *words[]) {
	int errorCode = 0;
	char *value;
	if (correctNumberOfArgs(words, 1)) {
		value = find(words[1]);
		if (value != NULL) {
			printf("%s\n", value);
		}	
		else {
			printf("Error: Variable '%s' has not initialized.\n", words[1]);
			errorCode = 3;
		}
	}
	else {	
		errorCode = 2;
	}

	return errorCode;
}


int run(char *words[]) {
	int errorCode = 0;
	char line[1000] = {""};
	FILE *f;
	if (correctNumberOfArgs(words, 1)) { 
		f = fopen(words[1], "rt");
		if (f == NULL) {
			printf("Error: File %s not found.\n", words[1]);
			errorCode = 3;	
			return errorCode;
		}
		while (fgets(line, 999, f) != NULL) {
			errorCode = parse(line);
			if (errorCode != 0) break;
		}
	}	
	else {	
		errorCode = 2;
		return errorCode;
	}
	fclose(f);
	
	return errorCode;
}

int exec(char *words[]) {
	int errorCode = 0;
	int numberOfArgs = getNumberOfArgs(words);
	FILE *f;

	if (numberOfArgs > 3 || numberOfArgs < 0) {
		errorCode = 2; 
		return errorCode;
	}

	for (int i = 1; i < numberOfArgs + 1; i++) {
		f = fopen(words[i], "r");
		if (f == NULL) {
			errorCode = 3;
			printf("Could not open file '%s'\n", words[i]);
			return errorCode;
		}
		errorCode = launcher(f, words[i]);
		if (errorCode != 0) {
			return errorCode;
		}

	}
	errorCode = scheduler();
	
	resetRAM();
	resetReadyList();
	return errorCode;
}

int mount(char *words[]) {
	int errorCode = 0;
	
	//checking the number of args	
	if (!(correctNumberOfArgs(words, 3))) {
		printf("Error: Wrong number of arguments for mount\n");
		errorCode = 2;
		return errorCode;
	}
	
	//create the partition and mount it
	errorCode = partition(words[1], atoi(words[2]), atoi(words[3]));
	if (errorCode != 0) {
		printf("partition is failing\n");
		return errorCode;
	}
	errorCode = mountFS(words[1]);
	
	return errorCode;
}

int read(char *words[]) {
	int errorCode = 0, fd = openfile(words[1]);
	char finalString[1024] = "",  *wordsForSet[3];
	
	//checking correct number of args
	if (!correctNumberOfArgs(words, 2)) {
		errorCode = 2;
		return errorCode;
	}
	
	//could not open file	
	if (fd == -1) {
		printf("Error: could not open file '%s'\n", words[1]);
		errorCode = 3;
		return errorCode;
	}
	
	//if the first block returns -1, print NULL	
	if (readBlock(fd) == -1) {
		strcat(finalString, read_buffer);
	}

	//otherwise, build the string up   
	else {
		do {
			strcat(finalString, read_buffer); 
		}
		while (readBlock(fd) != -1);
	

	}

	//supply the words to set command
	wordsForSet[0] = strdup("set");
	wordsForSet[1] = strdup(words[2]);
	wordsForSet[2] = strdup(finalString);
	errorCode = set(wordsForSet);
	
	//cleanup
	freeWords(wordsForSet);
	
	return errorCode;
}


int write(char *words[]) {
	int errorCode = 0;
	
	//check for correct number of args	
	if (!correctNumberOfArgs(words, 2)) {
		errorCode = 2;
		return errorCode;
	}

	//open the file and check for errors
	int fd = openfile(words[1]);
	if (fd == -1) {
		printf("Error: Could not open file '%s'\n", words[1]);
		errorCode = 3;
		return errorCode;
	}
	

	int i = 0, numBlocks = (strlen(words[2]) + curBlockSize - 1) / curBlockSize;
	char blockToWrite[curBlockSize + 1];	

	for (int k = 0; k < numBlocks; k++) { 
		for (int j = 0; j < curBlockSize; j++) {
			if (words[2][i] != '\0') {
				blockToWrite[j] = words[2][i];
				i++;
			}
			else {
				blockToWrite[j] = '0';
			}
		}
		blockToWrite[curBlockSize]= '\0';
		if (writeBlock(fd, blockToWrite) == -1) {
			printf("Error: could not write block\n");
			errorCode = 3;
			return errorCode;
		}
	}		
	return errorCode;
}



int interpreter(char *words[]) { 
	int errorCode = 0;
	
	if 	(strcmp(words[0], "help") == 0)		errorCode = help(words);
	else if (strcmp(words[0], "quit") == 0) 	errorCode = quit(words);
	else if (strcmp(words[0], "set") == 0) 		errorCode = set(words);
	else if (strcmp(words[0], "print") == 0) 	errorCode = print(words);
	else if (strcmp(words[0], "run") == 0) 		errorCode = run(words);
	else if (strcmp(words[0], "exec") == 0) 	errorCode = exec(words);
	else if (strcmp(words[0], "mount") == 0)	errorCode = mount(words);
	else if (strcmp(words[0], "write") == 0) 	errorCode = write(words);
	else if (strcmp(words[0], "read") == 0)        errorCode = read(words);
	else errorCode = 1;
	
	//cleanup	
	freeWords(words);

	return errorCode;

}


