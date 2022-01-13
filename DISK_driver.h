#ifndef _DISK_DRIVER_H
#define _DISK_DRIVER_H

void initIO();
int partition(char *name, int blocksize, int totalblocks);
int mountFS(char *name);
int openfile(char *name);
int readBlock(int file);
int writeBlock(int file, char *data);

extern char *read_buffer;
extern int curBlockSize;



#endif
