#include "clock.h"

#include "int.h"
#include "panic.h"

unsigned int seconds=0;
static unsigned int ticks=0;

struct ClockAlarm
{
	unsigned int delta;
	struct Task* task;
	struct ClockAlarm* next;
};

struct ClockAlarm* alarm_queue = 0;

void clock_set_wakeup(struct Task* task, unsigned int milliseconds)
{
	struct ClockAlarm* alarm = malloc(sizeof(struct ClockAlarm));
	struct ClockAlarm* ptr;

	if(!alarm)
		panic("out of mem on setting clock wakeup");

	alarm->delta = milliseconds;
	alarm->task = task;
	alarm->next = 0;

	/*printf("set_alarm: %x\n", task);
	*/
	if(!alarm_queue)
	{
		alarm_queue = alarm;
		return;
	}

	ptr = alarm_queue;

	do
	{
		alarm->delta -= ptr->delta;
	}
	while(ptr->next && alarm->delta > ptr->next->delta);

	if(!ptr->next)
	{
		ptr->next = alarm;
		return;
	}

	ptr->next->delta -= alarm->delta;
	alarm->next = ptr->next;
	ptr->next = alarm;

	return;
}

void clock_func()
{
	unsigned int tickl = TICK;

	ticks++;

	if((ticks % HZ)==0)
	{
		ticks -= HZ;
		seconds++;
	}

	while(tickl > 0)
	{
		if(!alarm_queue)
			return;

		if(alarm_queue->delta <= tickl)
		{
	/*printf("alarm: %x\n", alarm_queue->task);		*/
			task_wakeup(alarm_queue->task);
			tickl -= alarm_queue->delta;

			{
				struct ClockAlarm* tmp = alarm_queue;
				alarm_queue = tmp->next;
				free(tmp);
			}
		}
		else
		{
			alarm_queue->delta -= tickl;
			tickl = 0;
		}
	}
}

void clock_init()
{
	unsigned int divisor=1193180/HZ;
	outb(0x43,0x36);
	outb(0x40,divisor&0xFF);
	outb(0x40,divisor>>8);

	irq_capture(0, clock_func);
}

