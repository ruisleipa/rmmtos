#include "sys.h"

unsigned int sys_open_file(char* path, unsigned int mode)
{
#asm
	push bp
	mov  bp, sp

	push bx
	push cx

	mov ax, #1
	mov bx, 4[bp]
	mov cx, 6[bp]

	int $80

	pop cx
	pop bx

	pop bp
#endasm
}

void sys_close_file(unsigned int handle)
{
#asm
	push bp
	mov  bp, sp

	push bx

	mov ax, #2
	mov bx, 4[bp]

	int $80

	pop bx

	pop bp
#endasm
}

void sys_exit(unsigned int code)
{
#asm
	push bp
	mov  bp, sp

	push bx

	mov ax, #0
	mov bx, 4[bp]

	int $80

	pop bx

	pop bp
#endasm
}

unsigned int sys_read_file(unsigned int handle, char* buffer, unsigned int size)
{
#asm
	push bp
	mov  bp, sp

	push bx
	push cx
	push dx

	mov ax, #3
	mov bx, 4[bp]
	mov cx, 6[bp]
	mov dx, 8[bp]

	int $80

	pop dx
	pop cx
	pop bx

	pop bp
#endasm
}

unsigned int sys_write_file(unsigned int handle, char* buffer, unsigned int size)
{
#asm
	push bp
	mov  bp, sp

	push bx
	push cx
	push dx

	mov ax, #4
	mov bx, 4[bp]
	mov cx, 6[bp]
	mov dx, 8[bp]

	int $80

	pop dx
	pop cx
	pop bx

	pop bp
#endasm
}

unsigned int sys_mount()
{
#asm
	push bp
	mov  bp, sp

	mov ax, #7

	int $80

	pop bp
#endasm
}
