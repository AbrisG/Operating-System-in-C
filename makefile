mykernel: kernel.o shell.o interpreter.o shellmemory.o ram.o pcb.o cpu.o memorymanager.o DISK_driver.o
	gcc -o mykernel kernel.o shell.o interpreter.o shellmemory.o ram.o pcb.o cpu.o memorymanager.o DISK_driver.o

kernel.o: kernel.c shell.h ram.h pcb.h cpu.h kernel.h memorymanager.h DISK_driver.h
	gcc -c kernel.c 

shell.o: shell.c interpreter.h
	gcc -c shell.c

interpreter.o: interpreter.c shell.h kernel.h shellmemory.h ram.h DISK_driver.h
	gcc -c interpreter.c

shellmemory.o: shellmemory.c shellmemory.h
	gcc -c shellmemory.c

ram.o: ram.c
	gcc -c ram.c

pcb.o: pcb.c pcb.h
	gcc -c pcb.c

cpu.o: cpu.c cpu.h ram.h shell.h
	gcc -c cpu.c

DISK_driver.o: DISK_driver.c
	gcc -c DISK_driver.c

memorymanager.o: memorymanager.c ram.h pcb.h kernel.h
	gcc -c memorymanager.c

clean: 
	rm -f mykernel kernel.o shell.o interpreter.o shellmemory.o ram.o pcb.o cpu.o memorymanager.o DISK_driver.o
