#include "mutex.h"

void lock(int* mutex)
{
	int aquired = 0;

	do
	{
		asm("pushf");
		asm("cli");

		if(*mutex == 0)
		{
			*mutex = 1;
			aquired = 1;
		}

		asm("popf");

		if(!aquired)
			task_schedule();
	}
	while(!aquired);
}

void unlock(int* mutex)
{
	*mutex = 0;
}

