unsigned char inb(unsigned int port)
{
#asm
	push bp
	mov  bp, sp

	push dx
	mov  dx, 4[bp]
	mov  ax, #0
	in   al, dx

	pop  dx

	pop  bp
#endasm
}

void outb(unsigned int port,unsigned char val)
{
#asm
	push bp
	mov  bp, sp

	push ax
	push dx
	mov  dx, 4[bp]
	mov  al, 6[bp]
	out  dx, al
	pop  dx
	pop  ax

	pop  bp
#endasm
}

