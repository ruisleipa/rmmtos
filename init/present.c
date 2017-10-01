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
		}
	}

	return 1;
}

char content[512];

#define ROWS 100

int rmmtosMain(char* arg)
{
	struct File* present_file = openFile(arg);
	char* rows[ROWS];
	unsigned int cont = 1;
	unsigned int res;
	int i, j;
	char c;

	if(!present_file) {
		sys_exit(1);
	}

	res = readFile(present_file, content, 512);

	memsetw(rows, 0, ROWS);

	rows[0] = content;

	j = 0;
	i = 1;

	while(j < (res & 0xfff)) {
		if(content[j] == '\n') {
			content[j] = 0;
			rows[i] = &content[j + 1];
			i++;
		}

		j++;
	}

	// present from first row
	i = 0;

	runCommand(rows[i]);

	while(cont) {
		int ready = 0;
		int size = 0;

		while(!ready) {
			unsigned int res = readFile(input, &c, 1);

			if(res > 0) {
				switch(c) {
					case 'n': if(rows[i+1]) {i++; runCommand(rows[i]); } break;
					case 'p': if(i > 0) {i--; runCommand(rows[i]); } break;
					case 'q': cont = 0; break;
					default: break;
				}
			}

			sys_sleep(100);
		}
	}

	return 0;
}
