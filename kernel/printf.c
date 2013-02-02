#include "printf.h"
#include "varargs.h"
#include "mem.h"
#include "mutex.h"

void prel_putch(char c);

static putch_t* putch_func = prel_putch;

void putch_set(putch_t* func)
{
	putch_func=func;
}

void putch(char c)
{
	putch_func(c);
}

static int index=0;

void prel_putch(char c)
{
	poke(0xb800,index++,c);
	poke(0xb800,index++,0x0f);
}

void puts(char* str)
{
	while((*str)!=0)
	{
		putch((*str));
		str++;
	}
}

static char* digits="0123456789abcdef";

void puthex(unsigned int val)
{

}

void putfill(int totalnums,char fillchar)
{
	if(totalnums>0)
	{
		putfill(--totalnums,fillchar);
		putch(fillchar);
	}
}

void putuint(unsigned int value,char base,char totalnums,char fillchar)
{
	if(totalnums>0)
		totalnums--;

	if((value/base)!=0)
		putuint(value/base,base,totalnums,fillchar);
	else
		putfill(totalnums,fillchar);

	putch(digits[value%base]);
}

static MUTEX(printf_mutex);

void printf(char* fmt, ...)
{
   	int fill=8;
   	char fillchar=' ';

	va_list list;

	va_start(list,fmt);

//	lock(printf_mutex);

	while(*fmt!=0)
	{
		if(*fmt=='%')
		{
			fmt++;

			switch(*fmt)
			{
				case 'd':
					putuint(va_arg(list,int),10,0,fillchar);
					break;
				case 's':
					puts(va_arg(list,char*));
					break;
				case 'b':
					putuint(va_arg(list,int),2,fill,'0');
					break;
				case 'c':
					putch(va_arg(list,char));
					break;
				case 'h':
					putuint(va_arg(list,int),16,4,' ');
					break;
				case 'x':
					putuint(va_arg(list,int),16,4,'0');
					break;
				case 'a':
					putuint(va_arg(list,int),16,2,'0');
					break;
				case '%':
					putch('%');
					break;
				case '\0':
					continue;
				default:
					break;
			}
		}
		else
		{
			putch(*fmt);
		}

		fmt++;
	}

	//unlock(printf_mutex);
}

