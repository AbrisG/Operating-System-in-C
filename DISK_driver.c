#include <string.h>
#include <stdlib.h>
#include <stdio.h>


//global variables

struct PARTITION {
        int total_blocks;
        int block_size;
} part;


struct FAT {
	char *filename;
	int file_length;
	int blockPtrs[10];
	int current_location;
} fat[20];

int fat_aft[5], partitionPointer = 0, curBlockSize;
char curFileName[100], *partitions[100], *block_buffer, *read_buffer;
FILE *active_file_table[5] = {NULL};

void seekToBlock(FILE *f, int blockIndex);
int findFreeBlock(FILE *f);

void initIO() {
	//initializing partition (not sure about the 0 values)
	part.total_blocks = 0;
	part.block_size = 0;
	

	//intializing the fat table
	for (int i = 0; i < 20; i++) {
		fat_aft[i] = -1;
		if (fat[i].filename != NULL) {
			free(fat[i].filename);
			fat[i].filename = NULL;	
		}
		fat[i].file_length = 0;
		for (int j = 0; j < 10; j++) {
			fat[i].blockPtrs[j] = -1; 
		}
		fat[i].current_location = -1;
	}	
	
	for (int i = 0; i < 5; i++) {
		fat_aft[i] = -1;
		active_file_table[i] = NULL;
	}

	curFileName[0] = '?';
}

int partition(char *name, int blocksize, int totalblocks) {
	int errorCode = 0;
	FILE *f;	
	
	//resetting the fat table using initIO()
	initIO();
	
	//check if partition already exists
	for (int i = 0; i < partitionPointer; i++) {
		if (strcmp(name, partitions[i]) == 0) {	
			return errorCode;
		}
	}

	//creating PARTITION directory if it doesn't exist already
	system("mkdir PARTITION 2>/dev/null");	

	//open the partiton
	char newPartition[100];
	snprintf(newPartition, sizeof(newPartition), "PARTITION/%s", name);	

	f = fopen(newPartition, "w+");	
	if (f == NULL) {
		printf("Error: Could not open file '%s' during partition\n", curFileName);
		errorCode = 3;
		return errorCode;
	}
	
	//update the partition info		
	part.total_blocks = totalblocks;
	part.block_size = blocksize;

	//writing the data to file
	fprintf(f, "%i %i\n", part.total_blocks, part.block_size);	
	for (int i = 0; i < 20; i++) {
		if (fat[i].filename == NULL)
	      		fprintf(f, "%s %i %i\n", "NULL", fat[i].file_length, fat[i].current_location);
		else 
			fprintf(f, "%s %i %i\n", fat[i].filename, fat[i].file_length, fat[i].current_location);

		for (int j = 0; j < 9; j++) {
			fprintf(f, "%i ", fat[i].blockPtrs[j]);
		}
		fprintf(f, "%d\n", fat[i].blockPtrs[10]);

	}
		
	//create the blocks
	for (int i = 0; i < totalblocks; i++) {
		for (int j = 0; j < blocksize; j++) {
			fprintf(f, "%i", 0);
		}
		fprintf(f, "\n");
	}
		
	fclose(f);

	//save the partititon that was just created
	partitions[partitionPointer] = strdup(name);
	partitionPointer++;
		
	return errorCode;
		
}

int mountFS(char *name) {
	int errorCode = 0;
	FILE *f;

	//reset the fat
	initIO();
	
	//oepning the partition	
	snprintf(curFileName, sizeof(curFileName), "PARTITION/%s", name);	
	f = fopen(curFileName, "r");	
	if (f == NULL) {
		printf("Error: could not open partition '%s' during mount\n", curFileName);
		errorCode = 3;
		return errorCode;
	}
	

	//filling partition info	
	char *buffer = NULL;
	char nameBuffer[100];
	char *blockPtrBuffer = "";
	size_t n = 0;

	getline(&buffer, &n, f);
	sscanf(buffer, "%i %i\n", &part.total_blocks, &part.block_size);
	
	for (int i = 0; i < 20; i++) {
		getline(&buffer, &n, f);
		sscanf(buffer, "%s %i %i\n", nameBuffer, &fat[i].file_length, &fat[i].current_location);
		if (strcmp(nameBuffer, "NULL") == 0) {
			fat[i].filename = NULL;
		}
		else {
			fat[i].filename = strdup(nameBuffer);
			fat[i].current_location = 0;
		}
		getline(&buffer, &n, f);
		blockPtrBuffer = strtok(buffer, " ");		
		fat[i].blockPtrs[0] = atoi(blockPtrBuffer);
		for (int j = 1; j < 10; j++) {
			blockPtrBuffer = strtok(NULL, " ");
			fat[i].blockPtrs[j] = atoi(blockPtrBuffer); 
		}
	}

	//malloc the block_buffer and cleanup	
	block_buffer = (char *) malloc(part.block_size + 2);
	fclose(f);
	free(buffer);	
	curBlockSize = part.block_size;

	return errorCode;
}

int openfile(char *name) {
	int errorCode = -1;
	
	if (curFileName[0] == '?') {
		printf("Error: Cannot open file because no FS has been mounted\n");
		return errorCode;
	}

	//find the file in the fat
	int fatI = -1;
	for (int i = 0; i < 20; i++) {
		if (fat[i].filename == NULL) continue;
		if (strcmp(fat[i].filename, name) == 0) {
			fatI = i;
			break;		
		}		
	}


	//find a free slot in the active file table
	int aftI = -1;
	for (int i = 0; i < 5; i++) {
		if (active_file_table[i] == NULL) {
			aftI = i;
			break;
		}		
	}
	if (aftI == -1) {
		printf("Error: Active file table is full\n");
		return errorCode;
	}


	//found the file
	if (fatI != -1) {
		FILE *f = fopen(curFileName, "r+");
		if (f == NULL) {
			printf("Error: could not open partition '%s' during oppenfile\n", curFileName);
			return errorCode;
		}
		
		//seek to beggining of data section	
		seekToBlock(f, 0);
		active_file_table[aftI] = f;
		fat_aft[aftI] = fatI;
		return fatI;

			
	}
	//could not find the file
	else {
		for (int i = 0; i < 20; i++) {
			if (fat[i].filename  == NULL) {
				fat[i].filename = strdup(name);
				fat[i].file_length = 0;
				fat[i].current_location = 0;
				fat_aft[aftI] = i;
				return i;
			}
		}	
		//fat table is full
		return -1;	
	}

}

int  readBlock(int file) {
	int errorCode = 0, fatI = file, blockIndex, numBytes;
	FILE *readFile;


	//if file is out of range, then return errorCode = -1
	if (file < 0 || file >= 20) {
		printf("Error: File descriptor out of range\n");
		errorCode = -1;
	        return errorCode;	
	}

	//if the file at the given fatI does not correspond to any active file
	//then return errorCoe = -1
	int aftI = -1;	
	for (int i = 0; i < 5; i++) {
		if (fat_aft[i] == fatI) {
			aftI = i;
			break;
		}
	}

	if (aftI == -1) {
		printf("Error: No active file assocaited with file descriptor\n");
		errorCode = -1;
		return errorCode;
	}

	
	//grab the file pointer from the active file table using the aftI variable
	readFile = active_file_table[aftI];
	
	//find block index by using current location and blockPtrs array
	blockIndex = fat[fatI].blockPtrs[fat[fatI].current_location];

	//end of file, then return -1
	if (blockIndex == -1) {
		read_buffer = strdup("NULL");
		errorCode = -1;
		return errorCode;
	}

	//seek to correct block
	seekToBlock(readFile, blockIndex);
	fseek(readFile, 1, SEEK_CUR);
		

	//if the file is not at the end, save into block_buffer, and increment current location
	if (fgets(block_buffer, part.block_size + 1, readFile) == NULL) {
		printf("At end of file but not in a good wat\n");
		errorCode = -1;
		return errorCode;
	}

	else {
		block_buffer[strcspn(block_buffer, "\n")] = 0;
		fat[fatI].current_location += 1;
	}
	
	//formatting: removing the trailing 0s	
	for (int i = part.block_size - 1; i >= 0 && block_buffer[i] == '0'; i--) {
		block_buffer[i] = 0;
	}
	
	read_buffer = strdup(block_buffer);
	
	return errorCode;
	

}

int writeBlock(int file, char *data) {
	int errorCode = -1; 
	FILE *readFile, *writeFile;
	
	if (strlen(data) > part.block_size) {
		printf("Error: data length exceeds block size\n");
		return errorCode;
	}

        //if file is out of range, then return errorCode = -1
        if (file < 0 || file >= 20) {
		printf("Error: File index out of range\n");
                return errorCode;
        }

	
	//if the file at the given fatI does not correspond to any active file, then return errorCoe = -1
	int fatI = file, aftI = -1;
	for (int i = 0; i < 5; i++) {
		if (fat_aft[i] == fatI) {
			aftI = i;
			break;
		}
	}
	if (aftI == -1) {
		printf("Error: Could not find any open file associated with file %d\n", file);
		return errorCode;
	}

	//grab the file pointer from the active file table using the aftI variable
        readFile = active_file_table[aftI];

	if (readFile == NULL) {
		readFile = fopen(curFileName, "r+");
	}
	
	writeFile = fopen("PARTITION/tempfile", "w");

	if (writeFile == NULL) {
		printf("Error: Could not open writeFile in writeBlock\n");
		return errorCode;	
	}
	
	//appending to the end of the file by default
	fat[fatI].current_location = fat[fatI].file_length;

	if (fat[fatI].current_location >= 10) {
		printf("Error: Could not write block because end of file has been reached");
		return errorCode;
	}

	//find block index by using current_location and blockPtrs array			
	int blockIndex = fat[fatI].blockPtrs[fat[fatI].current_location];

	//if there is no corresponding block at that index of the blockPtrs array
	//then find free block
	if (blockIndex == -1) {
		blockIndex = findFreeBlock(readFile);

		//if there is no free block, then memory is full, and return errorCode = -1
		if (blockIndex == -1) {
			printf("Error: Could not find free block in partition\n");	
			return errorCode;
		}
		//update the fat
		fat[fatI].blockPtrs[fat[fatI].current_location] = blockIndex;
	}
	fat[fatI].file_length += 1;
	fat[fatI].current_location += 1;

	//formatting the changed contents	
	char firstLineFat[100];
	char blockToWrite[part.block_size + 2];

	snprintf(firstLineFat, sizeof(firstLineFat), "%s %i %i\n", fat[fatI].filename, fat[fatI].file_length, fat[fatI].current_location);
	
	snprintf(blockToWrite, sizeof(blockToWrite), "%s\n", data);	

	//copy everything from the original file, but insert modified lines at the right places
	char buffer[1024];
	int i = 0;
	int fatStart = 1 + fatI*2, fatEnd = fatStart + 1;

	rewind(readFile);
	while (fgets(buffer, sizeof(buffer), readFile) != NULL) {
		if (i == fatStart) {
			fprintf(writeFile, "%s", firstLineFat);		
		}
		else if (i == fatEnd) {
			for (int j = 0; j < 9; j++) {
				fprintf(writeFile, "%i ", fat[fatI].blockPtrs[j]);
			}
			fprintf(writeFile, "%i\n", fat[fatI].blockPtrs[9]);
		}

		else if (i == (1 + 2*20 + blockIndex)) {
			fprintf(writeFile, "%s", blockToWrite);    
		}			
		else {
			fprintf(writeFile, "%s", buffer);
		}
		i++;
	}	


	//update relevant variables
	fclose(writeFile);

	remove(curFileName);
	rename("PARTITION/tempfile", curFileName);
	
	active_file_table[aftI] = fopen(curFileName, "r+");

	return 0; 
}

	void seekToBlock(FILE *f, int blockIndex) {
		int firstBlock = -(part.total_blocks * (part.block_size + 1) + 1);
		int offset = blockIndex * (part.block_size + 1);
		fseek(f, firstBlock + offset, SEEK_END);
	}	


	int findFreeBlock(FILE *f) {
		char buffer[part.block_size + 2]; 
		char emptyBlock[part.block_size + 2];
		int freeBlockIndex = -1;
		
		//construct the emptyBlock
		for (int i = 0; i < part.block_size; i++) {
			emptyBlock[i] = '0';
		}
		emptyBlock[part.block_size] = '\n';
		emptyBlock[part.block_size + 1] = '\0';

		
		//seek to beggining of the block
		//and compare each block to emptyBlock to find an empty block
		rewind(f);
		seekToBlock(f, 0);
		while (fgets(buffer, sizeof(buffer), f) != NULL) {
			if (strcmp(buffer, emptyBlock) == 0) {
				return freeBlockIndex;
			}
			freeBlockIndex++;
		}
		
		//could not find the free block
		return -1;

	}
