#ifndef __VARARGS_H
#define __VARARGS_H

typedef char* va_list;

#define va_start(ap, p)	(ap = (char *) (&(p)+1))
#define va_arg(ap,t) ((t *)(ap += sizeof(t)))[-1]
#define va_end(ap) ap = NULL

#endif
