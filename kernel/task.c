#include "task.h"
#include "int.h"
#include "string.h"
#include "panic.h"
#include "mutex.h"

#define RUNNING 0
#define SLEEPING 1

/*
this state is used for tasks that have their associated events fired when have
not slept yet. This prevents the task subsequently entering a sleep state that
has no event to wake the task up again.
*/
#define WAKEN_BEFORE_SLEEP 2
#define EXITED 3

static struct Task* task_list=0;
struct Task* current_task=0;

extern void* init;
extern void* init_end;

void switch_task(struct Task* old_task, struct Task* new_task)
{
#asm
	push bp
	mov bp, sp

	push ax
	push si
	push di

	mov di, 4[bp]
	mov si, 6[bp]

	mov ax, 8[si]

	cli
	xchg bx, bx

	// XXX: is this really correct
	mov _current_task, si
	xchg ax, sp

	sti

	mov 8[di], ax

	pop di
	pop si
	pop ax

	pop bp
#endasm
}

static MUTEX(scheduler_lock);

static unsigned char segment_bitmap = 0;

unsigned int allocate_segment() {
	int i;
	unsigned int segment = 0x1050;

	for (i = 0; i < 8; ++i) {
		if((segment_bitmap & (0x80 >> i)) == 0) {
			segment_bitmap |= (0x80 >> i);
			return segment;
		}

		segment += 0x1000;
	}

	return 0;
}

void free_segment(unsigned int segment) {
	int i = (segment - 0x1050) / 0x1000;
	segment_bitmap &= ~(0x80 >> i);
}

void task_schedule()
{
	struct Task* new_task = current_task;
	struct Task* old_task = current_task;
	int seen_non_ended = 0;
	int looped = 0;

	do
	{
		new_task = new_task->next;

		if(new_task == 0) {
			new_task = task_list;
			looped++;
		}

		if(new_task->state != EXITED) {
			seen_non_ended = 1;
		}

		if(new_task->state == RUNNING) {
			if(new_task != old_task)
			{
				debug_printf("switching tasks: %x -> %x\n", old_task, new_task);
				debug_printf("%x %x %x %x %x -> ", old_task->user_sp, old_task->stack_top, old_task->segment, old_task->state, old_task->kernel_sp);
				debug_printf("%x %x %x %x %x\n", new_task->user_sp, new_task->stack_top, new_task->segment, new_task->state, new_task->kernel_sp);
				switch_task(old_task, new_task);
			}
			return;
		}

		if(seen_non_ended && looped > 2) {
			asm("hlt");
		}
	}
	while(seen_non_ended || looped < 2);

	panic("No tasks to run.");
}

struct Task* task_add(unsigned int segment)
{
	struct Task* task = malloc(sizeof(struct Task));
	unsigned int* stack = malloc(sizeof(*stack) * KERNEL_STACK_WORDS);

	if(!task || !stack || !segment)
		panic("Task alloc failed!");

	debug_printf("task->segment: %x\n", segment);

	task->stack = stack;
	task->user_sp = 0xffea;
	task->segment = segment;

	pokew(segment, 0xfffe, 0x0200); /* flags */
	pokew(segment, 0xfffc, segment); /* cs */
	pokew(segment, 0xfffa, 0x0000); /* ip */
	pokew(segment, 0xfff8, 0xbeef); /* unneeded address */
	pokew(segment, 0xfff6, 0x0000); /* ax */
	pokew(segment, 0xfff4, 0x0000); /* bx */
	pokew(segment, 0xfff2, 0x0000); /* cx */
	pokew(segment, 0xfff0, 0x0000); /* dx */
	pokew(segment, 0xffee, 0x0000); /* si */
	pokew(segment, 0xffec, 0x0000); /* di */
	pokew(segment, 0xffea, 0x0000); /* bp */

	task->stack_top = &task->stack[KERNEL_STACK_WORDS - 1];

	task->kernel_sp = task->stack_top;

	task->state = RUNNING;
	task->next = 0;

	memsetw(task->stack, 0, KERNEL_STACK_WORDS);
	task->stack[KERNEL_STACK_GUARD_POS] = KERNEL_STACK_GUARD;

	task->handles = 0;
	task->handle_count = 0;

	task->task_waiting_for_exit = 0;

	if(task_list == 0)
	{
		task_list = task;
		current_task = task;
	}
	else
	{
		task->next = task_list;
		task_list = task;
	}

	return task;
}

struct Task* task_new()
{
	unsigned int task_seg = allocate_segment();
	struct Task* task = task_add(task_seg);

	debug_printf("task_new: %x\n", task_seg);

	return task;
}

void task_init()
{
	unsigned int task_seg = allocate_segment();
	task_add(task_seg);

	debug_printf("task_new: %x\n", task_seg);
}

void task_check_kernel_stack()
{
	if(current_task && current_task->stack[KERNEL_STACK_GUARD_POS] != KERNEL_STACK_GUARD)
		panic("Kernel stack guard trashed!");
}

unsigned int task_get_register(struct Task* task, unsigned int reg)
{
	unsigned int value;

	peekw(task->segment, task->user_sp + reg, &value);

	return value;
}

void task_set_register(struct Task* task, unsigned int reg, unsigned int value)
{
	pokew(task->segment, task->user_sp + reg, value);
}

void task_set_memory(struct Task* task, unsigned int addr, char* src, unsigned int count)
{
	farmemcpy(addr, src, count, task->segment, 0x0050);
}

void task_get_memory(struct Task* task, unsigned int addr, char* dst, unsigned int count)
{
	farmemcpy(dst, addr, count, 0x0050, task->segment);
}

unsigned int task_add_handle(struct Task* task, struct Handle* handle)
{
	int i;
	int new_handle_count;
	struct TaskHandle* new_table;

	for(i = 0; i < task->handle_count; i++)
	{
		if(task->handles[i].handle == 0)
		{
			task->handles[i].handle = handle;
			return i + 1;
		}
	}

	new_handle_count = task->handle_count + 5;

	new_table = malloc(sizeof(struct TaskHandle) * new_handle_count);

	if(!new_table)
		return 0;

	memset(new_table, 0, sizeof(struct TaskHandle) * new_handle_count);

	if(task->handles)
	{
		memcpy(new_table, task->handles, sizeof(struct TaskHandle) * task->handle_count);
		free(task->handles);
	}

	task->handles = new_table;

	i = task->handle_count;

	task->handles[i].handle = handle;

	task->handle_count = new_handle_count;

	return i + 1;
}

void task_remove_handle(struct Task* task, unsigned int id)
{
	id--;

	if(id < task->handle_count)
		task->handles[id].handle = 0;
}

struct Handle* task_get_handle(struct Task* task, unsigned int id)
{
	id--;

	if(id < task->handle_count)
		return task->handles[id].handle;

	return 0;
}

void task_exit(struct Task* task)
{
	int i;

	free_segment(task->segment);

	for(i = 0; i < task->handle_count; ++i) {
		struct TaskHandle* task_handle = &task->handles[i];

		if(task_handle->handle) {
			if(task_handle->handle->flags & DIRECTORY) {
				directory_close(task_handle->handle);
				task_handle->handle = 0;
			} else if(task_handle->handle->flags & FILE) {
				file_close(task_handle->handle);
				task_handle->handle = 0;
			}
		}
	}

	free(task->handles);
	task->handle_count = 0;

	task->state = EXITED;

	free(task->stack);
	task->stack = 0;

	if(task->task_waiting_for_exit) {
		task_wakeup(task->task_waiting_for_exit);
	}

	task_schedule();
}

extern void* kernel_exit;
extern void* kernel_exit_spc;

struct Task* segment_to_task(unsigned int segment) {
	struct Task* task = task_list;

	while(task) {
		if(task->segment == segment)
			return task;

		task = task->next;
	}

	return 0;
}

unsigned int fork()
{
	struct Task* task = malloc(sizeof(struct Task));
	unsigned int* stack = malloc(sizeof(*stack) * KERNEL_STACK_WORDS);
	unsigned int segment = allocate_segment();

	if(!task || !stack || !segment)
		panic("Task alloc failed!");

	memcpy(task, current_task, sizeof(struct Task));

	task->segment = segment;
	task->stack = stack;
	//task->state = SLEEPING;

	debug_printf("fork: new %x\n", task->segment);

	farmemcpy(0x0000, 0x0000, 0x8000, task->segment, current_task->segment);
	farmemcpy(0x8000, 0x8000, 0x8000, task->segment, current_task->segment);

	memsetw(task->stack, 0, KERNEL_STACK_WORDS);

	/* create a kernel stack for returning to userspace */
	task->stack[KERNEL_STACK_WORDS - 1] = &kernel_exit; /* return address */
	task->stack[KERNEL_STACK_WORDS - 2] = 0x0000; /* bp */
	task->stack[KERNEL_STACK_WORDS - 3] = 0x0000; /* ax */
	task->stack[KERNEL_STACK_WORDS - 4] = 0x0000; /* si */
	task->stack[KERNEL_STACK_WORDS - 5] = 0x0000; /* di */

	task->kernel_sp = &task->stack[KERNEL_STACK_WORDS - 5];
	task->stack_top = &task->stack[KERNEL_STACK_WORDS];

	task->stack[KERNEL_STACK_GUARD_POS] = KERNEL_STACK_GUARD;

	task->handle_count = 0;
	task->handles = 0;

	task->task_waiting_for_exit = 0;

	pokew(task->segment, task->user_sp + 18, task->segment); //cs
	pokew(task->segment, task->user_sp + 12, 0); // ax

	task->next = task_list;
	task_list = task;

	return task->segment;
}

void exec(char* param)
{
	int i = strlen(param);

	current_task->user_sp = 0xffff;

	while(i >= 0) {
		poke(current_task->segment, current_task->user_sp, param[i]);
		current_task->user_sp--;

		i--;
	}

	current_task->user_sp--;


	pokew(current_task->segment, current_task->user_sp, 1); /* argc */

	current_task->user_sp -= 2;

	pokew(current_task->segment, current_task->user_sp, 0x0200); /* flags */
	pokew(current_task->segment, current_task->user_sp - 2, current_task->segment); /* cs */
	pokew(current_task->segment, current_task->user_sp - 4, 0x0000); /* ip */
	pokew(current_task->segment, current_task->user_sp - 6, 0xbeef); /* unneeded address */
	pokew(current_task->segment, current_task->user_sp - 8, 0x0000); /* ax */
	pokew(current_task->segment, current_task->user_sp - 10, 0x0000); /* bx */
	pokew(current_task->segment, current_task->user_sp - 12, 0x0000); /* cx */
	pokew(current_task->segment, current_task->user_sp - 14, 0x0000); /* dx */
	pokew(current_task->segment, current_task->user_sp - 16, 0x0000); /* si */
	pokew(current_task->segment, current_task->user_sp - 18, 0x0000); /* di */
	pokew(current_task->segment, current_task->user_sp - 20, 0x0000); /* bp */

	current_task->user_sp = current_task->user_sp - 20;

	debug_printf("poked a new stack for %x\n", current_task->segment);

	return;
}

void task_sleep(struct Task* task)
{
	if(task->state == SLEEPING)
		panic("SLEEPING SLEEPING");
	if(task->state == WAKEN_BEFORE_SLEEP)
		task->state = RUNNING;
	else if(task->state == RUNNING)
		task->state = SLEEPING;

	task_schedule();
}

void task_wakeup(struct Task* task)
{/*
printf("task wakeup: %x\n", task);
*/
	if(task->state == WAKEN_BEFORE_SLEEP)
		panic("second wakeup");
	else if(task->state == RUNNING)
		task->state = WAKEN_BEFORE_SLEEP;
	else if(task->state == SLEEPING)
		task->state = RUNNING;
	else if(task->state == EXITED)
		panic("wakeup on exited");
}
