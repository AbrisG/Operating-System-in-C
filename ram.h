#include <stdio.h>
#ifndef RAM_H
#define RAM_H

void addToRAM(FILE *p, int *start, int *end);
void removeFromRAM(int pageTable[]);
void addLineToRAM(int position, char *line);
void printRAM();
void resetRAM();
void setToNULL(int position);

char *ram[40];

#endif
