#ifndef __INT_H
#define __INT_H

#include "task.h"

typedef void (intr_func_t)(unsigned int);

void irq_set_wakeup(struct Task* task, int irq);
int irq_capture(unsigned int irq, intr_func_t* func);
int exception_capture(unsigned int exception, intr_func_t* func);
void sys_capture(intr_func_t* func);

#endif

