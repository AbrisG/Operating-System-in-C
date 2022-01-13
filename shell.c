#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "interpreter.h"

int parse(char cmd[]) {
	char tmp[200] = "";
	char *words[100] = {NULL};
	int i, j;
	int w = 0;
	
/*	printf("beggining of parse:\n");
	printf("words: ");
	printWords(words);
	printf("tmp: %s\n", tmp); */

	if (cmd[0] == '\n') return 0;	
	//else cmd[strcspn(cmd,"\n")] = 0;     //removes trailing newline      

	for (i = 0; cmd[i]== ' ' && i < 1000; i++); //skip all whitespaces
	
/*	int k = 0;	
	while (cmd[k] != '\0') {
		printf("%c", cmd[k]); 
		k++;
	}
	printf("k: %d\n", k); */
	

	while (cmd[i] != '\0' && i < 1000) {
		if (cmd[i] == '[') {
			i++;
			for (j = 0; cmd[i] != ']'; i++, j++) {
				if (cmd[i] == '\0') {
					printf("Error: Missing ']'\n");
					return 3;
				}
				tmp[j] = cmd[i];
			}
			i++;
		}
		else {
			for (j = 0; cmd[i] != '\0' && cmd[i] != ' ' && i < 1000; i++, j++) {
				if (cmd[i] == '\n') {
					break;
				}
			       tmp[j] = cmd[i];
			}
		}
		tmp[j] = '\0';


		//printf("[%s]\n", tmp);
		
		words[w] = strdup(tmp);

		i++; w++;		
	}
/*	printf("i at nullterm: %d\n", i);	
	printf("end of parse:\n");
	printf("words: ");
	printWords(words);
	printf("tmp: %s\n", tmp);*/

	return interpreter(words);	

}


int shellUI() {
	//char *command = (char *)malloc(1000*sizeof(char));
	char command[1000]; 
	char *firstPrompt = "Welcome to the Abris shell!\nVerson 1.0 Created January 2020\n";
	char *prompt = "$ ";
	int errorCode = 0;
	size_t cap = 999;	
	FILE *readLocation = stdin;
	int rdFileDescriptor;

	printf("%s", firstPrompt);
		
	while(1) {
                if (isatty(STDIN_FILENO)) {
                        printf("%s", prompt);              //general prompt cycle  
                }
		fflush(stdout);
		if (fgets(command, cap, stdin) == NULL) {
			freopen("/dev/tty", "r", stdin);
			continue;	
		}
                errorCode = parse(command);

                if (errorCode == 1) {                     //no such command available
			command[strcspn(command, "\n")] = 0; //this might be dangerous
                        printf("Error: '%s' is not a valid command\n"
                               "Type 'help' to view all available commands\n", command);
		}
                else if (errorCode == 2)                //lacking arguments, 
                        printf("Error: Wrong number of arguments\n"
                                       "Type 'help' to view command syntax\n");
                else if (errorCode == 3)                //custom error statements
                        continue;
                else if (errorCode == 4)                //quit
                        exit(0);
        }
	

}	
