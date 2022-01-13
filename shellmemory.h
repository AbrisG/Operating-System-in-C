#ifndef SHELLMEMORY_H_
#define SHELLMEMORY_H_

struct MEM {
        char *var;
        char *value;
};

extern struct MEM memoryArray[1000];
void add(char *var, char *value);
char* find(char *var);

#endif
