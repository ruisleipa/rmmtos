#ifndef KERNEL_TASK_H
#define KERNEL_TASK_H

#define KERNEL_STACK_WORDS 2048
#define KERNEL_STACK_GUARD_POS 0
#define KERNEL_STACK_GUARD 0xafee

#define BP 0
#define DI 2
#define SI 4
#define DX 6
#define CX 8
#define BX 10
#define AX 12
#define CS 18

#include "fs/node.h"

struct TaskHandle
{
	struct Handle* handle;
};

struct Task
{
	unsigned int user_sp;
	unsigned int stack_top;
	unsigned int segment;
	unsigned char state;

	unsigned int kernel_sp;

	unsigned int* stack;

	struct TaskHandle* handles;
	unsigned char handle_count;

        struct Task* task_waiting_for_exit;

        struct Task* next;
};

void task_init();
void task_check_kernel_stack();
unsigned int task_get_register(struct Task* task, unsigned int reg);
void task_set_register(struct Task* task, unsigned int reg, unsigned int value);
void task_set_memory(struct Task* task, unsigned int addr, char* src, unsigned int count);
void task_get_memory(struct Task* task, unsigned int addr, char* dst, unsigned int count);

unsigned int task_add_handle(struct Task* task, struct Handle* handle);
void task_remove_handle(struct Task* task, unsigned int id);
struct Handle* task_get_handle(struct Task* task, unsigned int id);

void task_sleep(struct Task* task);
void task_wakeup(struct Task* task);

void exec(char* param = "");

#endif

