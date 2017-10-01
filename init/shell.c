#include "io.h"
#include "memory.h"
#include "string.h"
#include "sys.h"
#include "uint64.h"
#include "printf.h"

extern struct File* input;
extern struct File* output;

int runCommand(char* command) {
	int i = 0;

	char* arg = "";

	while(command[i] != ' ' && command[i] != 0) {
		i++;
	}

	if(command[i] == ' ') {
		command[i] = 0;
		arg = &command[i+1];
	}

	printf("Got command: \"%s\" \"%s\"\n", command, arg);

	if(strcmp("exit", command) == 0) {
		return 0;
	} else if(strcmp("pause", command) == 0) {
		return 1;
	} else {
		unsigned int task_id;

		closeFile(output);
		output = 0;

		task_id = sys_fork();

		if(task_id == 0) {
			sys_exec(command, arg);
			sys_exit(1);
		} else {
			sys_wait_for_task(task_id);

			output = openFile("/devices/screen", WRITE);
			printf("Returning to shell...\n");
		}
	}

	return 1;
}

int rmmtosMain()
{
	char c;
	char row[80];
	unsigned int cont = 1;

	printf("RMMTOS Shell v 0.01\n");

	while(cont) {
		int ready = 0;
		int size = 0;

		printf("> ");

		while(!ready) {
			unsigned int res = readFile(input, &c, 1);

			if(res > 0) {
				switch(c) {
					case '\n': ready = 1; printf("%c", c); break;
					case '\b': size--; row[size] = 0; printf("\b \b", c); break;
					default: row[size] = c;	row[size + 1] = 0; size++; printf("%c", c); break;
				}
			}

			sys_sleep(100);
		}

		cont = runCommand(row);
	}

	return 0;
}
