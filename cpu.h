#ifndef CPU_H_
#define CPU_H_

struct CPU {
        int IP;
	int offset;
        char IR[1000];
        int quanta;
};

extern struct CPU *cpu;

//void initCPU(int quanta);

int runCPU(int quanta);

#endif
