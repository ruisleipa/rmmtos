#include "int.h"
#include "panic.h"
#include "task.h"

void intr_quick();

void intr_0();
void intr_1();
void intr_2();
void intr_3();
void intr_4();
void intr_5();
void intr_6();
void intr_7();
void intr_8();
void intr_9();
void intr_10();
void intr_11();
void intr_12();
void intr_13();
void intr_14();
void intr_15();
void intr_16();
void intr_17();
void intr_18();
void intr_19();
void intr_20();
void intr_21();
void intr_22();
void intr_23();
void intr_24();

intr_func_t* func_table[25]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

struct IrqWait
{
	struct Task* task;
	struct IrqEvent* next;
};

struct IrqWait* irq_events[16];

void irq_set_wakeup(struct Task* task, int irq)
{
	struct IrqWait* new_wait = malloc(sizeof(struct IrqWait));
	struct IrqWait** wait_list = &irq_events[irq];

	if(irq >= 16)
		panic("irq index out of range");

	if(!new_wait)
		panic("irq wait alloc failed");

	/*printf("task %x waiting for irq %d\n", task, irq);
		*/
	new_wait->task = task;

	if(!(*wait_list))
		new_wait->next = 0;
	else
		new_wait->next = (*wait_list);

	(*wait_list) = new_wait;
}

void irq_resume_waiting_tasks(int irq)
{
	struct IrqWait* item = irq_events[irq];
	struct IrqWait* tmp;

	while(item)
	{
		task_wakeup(item->task);

		tmp = item;

		item = item->next;

		free(tmp);
	}

	irq_events[irq] = 0;
}

void intr_set_handler(unsigned int index,unsigned int cs,unsigned int ip)
{
	if(index > 0xff)
		panic("index out of range");

	pokew(0, index * 4, ip);
	pokew(0, index * 4 + 2, cs);
}

void exception_handler(unsigned int num)
{
	intr_func_t* func = func_table[num];

	if(func)
	{
		func(num);
	}
	else
	{
		printf("unhandled exception %x\n", num);
		panic("");
	}
}

void irq_handler(unsigned int num)
{
	intr_func_t* func = func_table[num + 8];

	if(func)
		func(num);
	else if(irq_events[num] == 0)
		printf("unhandled irq %x\n", num);

	irq_resume_waiting_tasks(num);

	if(num >= 8)
		outb(0xA0, 0x20);

	outb(0x20, 0x20);
}

void intr_init()
{
	int i;

	for(i = 0; i < 256; i++)
		intr_set_handler(i, 0x0050, intr_quick);

	intr_set_handler(0x00, 0x0050, intr_0);
	intr_set_handler(0x01, 0x0050, intr_1);
	intr_set_handler(0x02, 0x0050, intr_2);
	intr_set_handler(0x03, 0x0050, intr_3);
	intr_set_handler(0x04, 0x0050, intr_4);
	intr_set_handler(0x05, 0x0050, intr_5);
	intr_set_handler(0x06, 0x0050, intr_6);
	intr_set_handler(0x07, 0x0050, intr_7);

	intr_set_handler(0x08, 0x0050, intr_8);
	intr_set_handler(0x09, 0x0050, intr_9);
	intr_set_handler(0x0a, 0x0050, intr_10);
	intr_set_handler(0x0b, 0x0050, intr_11);
	intr_set_handler(0x0c, 0x0050, intr_12);
	intr_set_handler(0x0d, 0x0050, intr_13);
	intr_set_handler(0x0e, 0x0050, intr_14);
	intr_set_handler(0x0f, 0x0050, intr_15);

	intr_set_handler(0x70, 0x0050, intr_16);
	intr_set_handler(0x71, 0x0050, intr_17);
	intr_set_handler(0x72, 0x0050, intr_18);
	intr_set_handler(0x73, 0x0050, intr_19);
	intr_set_handler(0x74, 0x0050, intr_20);
	intr_set_handler(0x75, 0x0050, intr_21);
	intr_set_handler(0x76, 0x0050, intr_22);
	intr_set_handler(0x77, 0x0050, intr_23);

	intr_set_handler(0x80, 0x0050, intr_24);
}

int irq_capture(unsigned int irq, intr_func_t* func)
{
	if(irq >= 16)
		panic("irq out of range");

	if(func_table[irq + 8])
		return 0;

	func_table[irq + 8] = func;

	return 1;
}

int exception_capture(unsigned int exc, intr_func_t* func)
{
	if(exc >= 8)
		panic("exception out of range");

	if(func_table[exc])
		return 0;

	func_table[exc] = func;

	return 1;
}

void sys_capture(intr_func_t* func)
{
	func_table[24] = func;
}

void intr_dispatcher(unsigned int num)
{
	intr_func_t* func = func_table[num];

	asm("sti");

	task_check_kernel_stack();

	if(num == 24)
	{
		if(!func)
			return;

		func(num);
	}
	else if(num >= 8)
	{
		irq_handler(num - 8);
	}
	else
	{
		exception_handler(num);
	}


	task_check_kernel_stack();

}

