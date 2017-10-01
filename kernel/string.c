#include "string.h"

int strcmp(char* str1,char* str2)
{
	int dist = 0;

	while(*str1 || *str2)
	{

		if(*str1 != *str2)
		{
			dist = *str1 - *str2;

			break;
		}

		str1++;
		str2++;
	}

	if(dist > 0)
		return 1;
	else if(dist < 0)
		return -1;

	return 0;
}

int memcmp(char* str1,char* str2, int count)
{
	int dist = 0;

	while(count--)
	{
		if(*str1 != *str2)
		{
			dist = *str1 - *str2;

			break;
		}

		str1++;
		str2++;
	}

	if(dist > 0)
		return 1;
	else if(dist < 0)
		return -1;

	return 0;
}

char* strcpy(char* dest,char* src)
{
	while(*src!=0)
	{
		*dest=*src;

		dest++;
		src++;
	}

	*dest = 0;

	return dest;
}

char* farmemcpy(char* dest,char* src,int count,unsigned int destseg,unsigned int srcseg)
{
#asm
	push bp
	mov bp,sp

	push di
	push si
	push bx
	push ax
	push cx
	pushf

	cli

	mov cx,12[bp]
	mov ax,ds
	mov ds,cx
	mov cx,10[bp]
	mov bx,es
	mov es,cx
	mov cx,8[bp]
	mov di,4[bp]
	mov si,6[bp]

	rep
	movsb

	mov ds, ax
	mov es, bx

	popf
	pop cx
	pop ax
	pop bx
	pop si
	pop di

	pop bp
#endasm
}

char* memcpy(char* dest,char* src,int count)
{
	farmemcpy(dest,src,count,0x0050,0x0050);
}

char* farmemset(char* dest,unsigned int val,int count,unsigned int seg)
{
#asm
	push bp
	mov bp,sp

	push di
	push bx
	push cx
	push ax
	pushf

	cli

	mov cx,10[bp]
	mov bx,es
	mov es,cx
	mov cx,8[bp]
	mov ax,6[bp]
	mov di,4[bp]

	rep
	stosb

	mov es, bx

	popf
	pop ax
	pop cx
	pop bx
	pop di

	pop bp
#endasm
}

char* memset(char* dest,unsigned int val,int count,unsigned int seg)
{
	farmemset(dest,val,count,0x0050);
}

char* farmemsetw(char* dest,unsigned int val,int count,unsigned int seg)
{
#asm
	push bp
	mov bp,sp

	push di
	push cx
	push ax
	push bx
	pushf

	cli

	mov cx,10[bp]
	mov bx,es
	mov es,cx
	mov cx,8[bp]
	mov ax,6[bp]
	mov di,4[bp]

	rep
	stosw

	mov es, bx

	popf
	pop bx
	pop ax
	pop cx
	pop di

	pop bp
#endasm
}

unsigned int strlen(char* str)
{
	int len=0;

	while(*str!=0)
	{
		len++;
		str++;
	}

	return len;
}

int atoi(char* str)
{
	int value=0;
	int state=0;
	int neg;

	while(*str)
	{
		if(!isspace(*str) || state==0)
		{
			neg=1;

			if(*str=='-')
			{
				neg=-1;
			}

			state=1;
		}

		if(state==1)
		{
			if(isdigit(*str))
			{
				value*=10;
				value+=((*str)-'0');
			}
			else
			{
				break;
			}
		}

		str++;
	}

	value*=neg;

	return value;
}

char toupper(char c)
{
	if(islower(c)) {
		return c - 32;
	}

	return c;
}

char tolower(char c)
{
	if(isupper(c)) {
		return c + 32;
	}

	return c;
}
